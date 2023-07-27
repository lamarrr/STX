#pragma once

#include <algorithm>
#include <chrono>
#include <iostream>
#include <thread>
#include <utility>

#include "stx/async.h"
#include "stx/config.h"
#include "stx/scheduler/thread_slot.h"
#include "stx/task/id.h"
#include "stx/task/priority.h"
#include "stx/vec.h"

STX_BEGIN_NAMESPACE

using TimePoint = std::chrono::steady_clock::time_point;
using namespace std::chrono_literals;
using std::chrono::nanoseconds;

/// Scheduler Documentation
/// - Tasks are added to the timeline once they become ready for execution
/// - We first split tasks into timelines based on when they last ran
/// - We then select the most starved tasks in a sliding window containing at
/// least a certain amount of tasks
/// - We then sort the sliding window using their priorities
/// - We repeat these steps
/// - if any of the selected tasks is in the thread slot then we need not
/// suspend or cancel them
/// - the task itself manages its own suspension, preemption, and cancelation,
/// the timeline doesn't handle that, the task should use a request proxy to
/// track requests.
///
/// tasks in the timeline are ready-to-execute, preempted, or suspended tasks
struct ScheduleTimeline
{
  static constexpr nanoseconds STARVATION_PERIOD = 16ms * 4;

  struct Task
  {
    // task to execute
    RcFn<void()> fn = fn::rc::make_static([]() {});

    // used for book-keeping and notification of state
    PromiseAny promise;

    // assigned id of the task
    TaskId id{};

    // priority to use in evaluating CPU-time worthiness
    TaskPriority priority{NORMAL_PRIORITY};

    // the timepoint the task became ready for execution. in the case of a
    // suspended task, it correlates with the timepoint the task became ready
    // for resumption
    TimePoint last_preempt_timepoint{};

    // last known status of the task
    FutureStatus last_status_poll{FutureStatus::Preempted};
  };

  explicit ScheduleTimeline(Allocator allocator) :
      starvation_timeline{allocator}, thread_slots_capture{allocator}
  {}

  Result<Void, AllocError> add_task(RcFn<void()> fn, PromiseAny promise, TaskId id, TaskPriority priority, TimePoint present_timepoint)
  {
    // task is ready to execute but preempteed upon adding
    promise.notify_preempted();
    TRY_OK(ok, starvation_timeline.push(Task{std::move(fn), std::move(promise), id, priority, present_timepoint, FutureStatus::Preempted}));

    (void) ok;

    return Ok(Void{});
  }

  void poll_tasks(TimePoint present_timepoint)
  {
    // update all our record of the tasks' statuses
    //
    // NOTE: the task could still be running whilst cancelation was requested.
    // it just means we get to remove it from taking part in future scheduling.
    //
    // if the task is already running, it either has to attend to the
    // cancel request, attend to the suspend request, or complete.
    // if it makes modifications to the terminal state, after we have made
    // changes to it, its changes are ignored. and if it has reached a terminal
    // state before we attend to the request, our changes are ignored.
    //
    //
    for (Task &task : starvation_timeline.span())
    {
      // the status could have been modified in another thread, so we need
      // to fetch the status
      FutureStatus new_status = task.promise.fetch_status();

      // if preempt timepoint not already updated, update it
      if ((task.last_status_poll != FutureStatus::Preempted && new_status == FutureStatus::Preempted))
      {
        task.last_preempt_timepoint = present_timepoint;
      }

      task.last_status_poll = new_status;
    }
  }

  void execute_resume_requests()
  {
    for (Task &task : starvation_timeline)
    {
      if (task.last_status_poll == FutureStatus::Suspended && task.promise.fetch_suspend_request() == SuspendState::Executing)
      {
        // make it ready for resumption/execution
        task.promise.notify_preempted();
      }
    }
  }

  void remove_done_tasks()
  {
    Span done_tasks = starvation_timeline.span()
                          .partition([](Task const &task) { return task.last_status_poll != FutureStatus::Completed && task.last_status_poll != FutureStatus::Canceled; })
                          .second;

    starvation_timeline.erase(done_tasks);
  }

  // returns number of selected tasks
  size_t select_tasks_for_slots(size_t num_slots)
  {
    Span starving = starvation_timeline.span()
                        // ASSUMPTION(unproven): The tasks are mostly sorted so we are very
                        // unlikely to pay much cost in sorting???
                        //
                        // suspended tasks are not considered for execution
                        //
                        .partition([](Task const &task) { return task.last_status_poll == FutureStatus::Preempted || task.last_status_poll == FutureStatus::Executing; })
                        .first
                        // sort hungry tasks by preemption/starvation duration (most starved
                        // first). Hence the starved tasks would ideally be more likely
                        // chosen for execution
                        .sort([](Task const &a, Task const &b) { return a.last_preempt_timepoint < b.last_preempt_timepoint; });

    auto *selection = starving.begin();

    TimePoint const most_starved_task_timepoint = starving[0].last_preempt_timepoint;

    nanoseconds selection_period_span = STARVATION_PERIOD;

    while (selection < starving.end())
    {
      if ((selection->last_preempt_timepoint - most_starved_task_timepoint) <= selection_period_span)
      {
        // add to timeline selection
        selection++;
        continue;
      }
      else if ((selection->last_preempt_timepoint - most_starved_task_timepoint) > selection_period_span && (static_cast<size_t>(selection - starving.begin()) < num_slots))
      {
        // if there's not enough tasks within the current starvation period span
        // to fill up all the slots then extend the starvation period span
        // (multiple enough to cover this selection's timepoint)
        nanoseconds diff = selection->last_preempt_timepoint - most_starved_task_timepoint;

        // (STARVATION_PERIOD-1ns) added since division by STARVATION_PERIOD
        // ONLY will result in the remainder of the division operation being
        // trimmed off and hence not cover the required span
        int64_t multiplier = (diff + (STARVATION_PERIOD - nanoseconds{1})) / STARVATION_PERIOD;

        selection_period_span += STARVATION_PERIOD * multiplier;

        selection++;
        continue;
      }
      else
      {
        break;
      }
    }

    // sort selection span by priority
    std::sort(starving.begin(), selection, [](Task const &a, Task const &b) { return a.priority > b.priority; });

    size_t num_selected = selection - starving.begin();

    // the number of selected starving tasks might be more than the number of
    // available ones, so we select the top tasks (by priority)
    num_selected = std::min(num_slots, num_selected);

    return num_selected;
  }

  // slots uses Rc because we need a stable address
  void tick(Span<Rc<ThreadSlot *> const> slots, TimePoint present_timepoint)
  {
    // cancelation and suspension isn't handled in here, it doesn't really make
    // sense to handle here. if the task is fine-grained enough, it'll be
    // canceled as soon as its first phase finishes execution. this has the
    // advantage that we don't waste scheduling efforts.

    size_t const num_slots = slots.size();

    thread_slots_capture.resize(num_slots, ThreadSlot::Query{}).unwrap();

    // fetch the status of each thread slot
    slots.map([](Rc<ThreadSlot *> const &rc_slot) { return rc_slot.handle->slot.query(); }, thread_slots_capture.span());

    poll_tasks(present_timepoint);
    execute_resume_requests();
    remove_done_tasks();

    if (starvation_timeline.is_empty())
    {
      return;
    }

    size_t const num_selected = select_tasks_for_slots(num_slots);

    // request preemption of non-selected tasks since they might be running.
    //
    // we only do this if the task is not already force suspended since it could
    // be potentially expensive if the promises are very far apart in memory.
    //
    // we don't expect just-suspended tasks to suspend immediately, even if
    // they do we'll process them in the next tick.
    //
    for (Task const &task : starvation_timeline.span().slice(num_selected))
    {
      task.promise.request_preempt();
    }

    // push the tasks onto the task slots if the task is not already on any of
    // the slots.
    //
    // the selected tasks might not get slots assigned to them. i.e. if tasks
    // are still using some of the slots. tasks that don't get assigned to slots
    // here will get assigned in the next tick
    //
    // add tasks to slot if not already on the slots
    size_t next_slot = 0;

    for (Task const &task : starvation_timeline.span().slice(0, num_selected))
    {
      bool has_slot = !thread_slots_capture.span().which([&task](ThreadSlot::Query const &query) { return query.executing_task.contains(task.id) || query.pending_task.contains(task.id); }).is_empty();

      if (has_slot)
      {
        continue;
      }

      while (next_slot < num_slots && !has_slot)
      {
        if (thread_slots_capture[next_slot].can_push)
        {
          // possibly a preempted task.
          // tasks are expected to check their request states.
          //
          // we have to unpreempt the task
          //
          task.promise.clear_preempt_request();
          slots[next_slot].handle->slot.push_task(ThreadSlot::Task{task.fn.share(), task.id});
          has_slot = true;
        }

        next_slot++;
      }
    }
  }

  Vec<Task>              starvation_timeline;
  Vec<ThreadSlot::Query> thread_slots_capture;
};

STX_END_NAMESPACE
