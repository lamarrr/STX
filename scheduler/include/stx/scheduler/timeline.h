#pragma once

#include <algorithm>
#include <chrono>
#include <thread>
#include <utility>

#include "stx/async.h"
#include "stx/config.h"
#include "stx/scheduler/thread_slot.h"
#include "stx/task/id.h"
#include "stx/task/priority.h"
#include "stx/vec.h"

STX_BEGIN_NAMESPACE

using timepoint = std::chrono::steady_clock::time_point;
using namespace std::chrono_literals;
using std::chrono::nanoseconds;

enum class ThreadId : uint32_t {};

/// Scheduler Documentation
/// - Tasks are added to the timeline once they become ready for execution
/// - We first split tasks into timelines based on when they last ran
/// - We then select the most starved tasks in a sliding window containing at
/// least a certain amount of tasks
/// - We then sort the sliding window using their priorities
/// - We repeat these steps
/// - if any of the selected tasks is in the thread slot then we need not
/// suspend or cancel them
///
///
///

struct ScheduleTimeline {
  static constexpr nanoseconds INTERRUPT_PERIOD{16ms};
  static constexpr uint8_t STARVATION_FACTOR{4};
  static constexpr nanoseconds STARVATION_PERIOD =
      INTERRUPT_PERIOD * STARVATION_FACTOR;

  struct Task {
    RcFn<void()> fn = stx::fn::rc::make_static([]() {});

    PromiseAny promise;

    // scheduling parameters
    // must be initialized to the timepoint the task became ready
    timepoint last_suspend_timepoint{};

    TaskId id{};

    TaskPriority priority = stx::NORMAL_PRIORITY;

    FutureStatus last_status_poll = FutureStatus::Scheduled;

    RequestedCancelState last_requested_cancel_state_poll =
        RequestedCancelState::None;
  };

  explicit ScheduleTimeline(Allocator allocator)
      : starvation_timeline{allocator}, thread_slots_capture{allocator} {}

  Result<Void, AllocError> add_task(RcFn<void()>&& fn, PromiseAny&& promise,
                                    timepoint present_timepoint, TaskId id,
                                    TaskPriority priority) {
    promise.notify_suspended();
    TRY_OK(new_timeline_flex,
           stx::vec::push(std::move(starvation_timeline),
                          Task{std::move(fn), std::move(promise),
                               present_timepoint, id, priority}));

    starvation_timeline = std::move(new_timeline_flex);

    return Ok(Void{});
  }

  void remove_done_and_canceled_tasks() {
    auto span = starvation_timeline.span();

    starvation_timeline =
        stx::vec::erase(std::move(starvation_timeline),
                        span.partition([](Task const& task) {
                              FutureStatus status = task.last_status_poll;
                              return status == FutureStatus::Completed ||
                                     status == FutureStatus::Canceled;
                            })
                            .first);
  }

  void poll_tasks(timepoint present_timepoint) {
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
    //

    for (Task& task : starvation_timeline.span()) {
      RequestedCancelState const cancel_request =
          task.promise.fetch_cancel_request();

      task.last_requested_cancel_state_poll = cancel_request;

      if (cancel_request == RequestedCancelState::Canceled) {
        task.promise.notify_canceled();
      }

      // the status could have been modified in another thread, so we need
      // to fetch the status
      FutureStatus new_status = task.promise.fetch_status();

      // if preempt timepoint not already updated, update it
      if (task.last_status_poll != FutureStatus::Suspended &&
          new_status == FutureStatus::Suspended) {
        task.last_suspend_timepoint = present_timepoint;
      }

      task.last_status_poll = new_status;
    }

    remove_done_and_canceled_tasks();
  }

  // returns the end of the starving tasks (non-suspended)
  auto sort_and_partition_timeline() {
    // TODO(lamarrr): (UNPROVEN): The tasks are mostly sorted so we are very
    // unlikely to pay much cost in sorting???
    //
    //
    // split into ready to execute (pre-empted or running) and user-suspended
    // partition
    auto span = starvation_timeline.span();
    auto const starvation_timeline_starving_end =
        std::partition(span.begin(), span.end(), [](Task const& task) {
          return task.last_status_poll != FutureStatus::Suspended;
        });

    // sort by preemption/starvation duration (descending)
    std::sort(span.begin(), starvation_timeline_starving_end,
              [](Task const& a, Task const& b) {
                return a.last_suspend_timepoint > b.last_suspend_timepoint;
              });

    return starvation_timeline_starving_end;
  }

  // returns end of selections (sorted by priority in descending order)
  auto select_tasks_for_slots(size_t num_slots) {
    auto span = starvation_timeline.span();
    auto* timeline_selection_it = span.begin();

    // select only starving and ready tasks
    auto* starving_end = sort_and_partition_timeline();

    timepoint const most_starved_task_timepoint =
        span[0].last_suspend_timepoint;

    nanoseconds selection_period_span = STARVATION_PERIOD;

    while (timeline_selection_it < starving_end) {
      if ((timeline_selection_it->last_suspend_timepoint -
           most_starved_task_timepoint) <= selection_period_span) {
        // add to timeline selection
      } else if (static_cast<size_t>(timeline_selection_it - span.begin()) <
                 num_slots) {
        // if there's not enough tasks to fill up all the slots, extend
        // the starvation period span
        selection_period_span += STARVATION_PERIOD;
      } else {
        break;
      }

      timeline_selection_it++;
    }

    // sort selection span by priority
    std::sort(
        span.begin(), timeline_selection_it,
        [](Task const& a, Task const& b) { return a.priority > b.priority; });

    size_t num_selected = timeline_selection_it - span.begin();

    return std::min(num_slots, num_selected);
  }

  // slots uses Rc because we need a stable address
  void tick(Span<Rc<ThreadSlot*> const> slots, timepoint present_timepoint) {
    // cancelation and suspension isn't handled in here, it doesn't really make
    // sense to handle here. if the task is fine-grained enough, it'll be
    // canceled as soon as its first phase finishes execution. this has the
    // advantage that we don't waste scheduling efforts.

    size_t const num_slots = slots.size();

    thread_slots_capture = stx::vec::resize(std::move(thread_slots_capture),
                                            num_slots, ThreadSlot::Query{})
                               .unwrap();

    // fetch the status of each thread slot
    slots.map(
        [](Rc<ThreadSlot*> const& rc_slot) {
          return rc_slot.handle->slot.query();
        },
        thread_slots_capture.span());

    poll_tasks(present_timepoint);

    if (starvation_timeline.empty()) return;

    size_t const num_selected = select_tasks_for_slots(num_slots);

    // request suspend of non-selected tasks,
    //
    // we only do this if the task is not already force suspended since it could
    // be potentially expensive if the promises are very far apart in memory.
    //
    // we don't expect just-suspended tasks to suspend immediately, even if
    // they do we'll process them in the next tick and our we account for that.
    //
    for (Task const& task : starvation_timeline.span().slice(num_selected)) {
      if (task.last_status_poll != FutureStatus::Suspended) {
        task.promise.request_suspend();
      }
    }

    // add tasks to slot if not already on the slots
    size_t next_slot = 0;

    // push the tasks onto the task slots if the task is not already on any of
    // the slots.
    //
    // the selected tasks might not get slots assigned to them. i.e. if tasks
    // are still using some of the slots. tasks that don't get assigned to slots
    // here will get assigned in the next tick
    //
    for (Task const& task : starvation_timeline.span().slice(0, num_selected)) {
      auto selection = thread_slots_capture.span().which(
          [&task](ThreadSlot::Query const& query) {
            return query.executing_task.contains(task.id) ||
                   query.pending_task.contains(task.id);
          });

      bool has_slot = !selection.is_empty();

      while (next_slot < num_slots && !has_slot) {
        if (thread_slots_capture.span()[next_slot].can_push) {
          // possibly a suspended task
          task.promise.clear_suspension_request();
          slots[next_slot].handle->slot.push_task(
              ThreadSlot::Task{task.fn.share(), task.id});
          has_slot = true;
        }

        next_slot++;
      }
    }
  }

  // pending tasks
  // TODO(lamarrr):  newly ready tasks must be marked as suspended
  //
  Vec<Task> starvation_timeline;
  Vec<ThreadSlot::Query> thread_slots_capture;
};

// TODO(lamarrr): move execution of the timeline to the worker threads
// they go to the slots and sort them, satisfy pending streams if any
//
// they need to sleep if no task is available
//

STX_END_NAMESPACE
