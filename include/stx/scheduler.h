#pragma once

#include <algorithm>
#include <chrono>
#include <cinttypes>
#include <string_view>
#include <thread>
#include <tuple>
#include <utility>
#include <variant>

#include "stx/async.h"
#include "stx/config.h"
#include "stx/fn.h"
#include "stx/option.h"
#include "stx/rc.h"
#include "stx/scheduler/thread_pool.h"
#include "stx/scheduler/thread_slot.h"
#include "stx/scheduler/timeline.h"
#include "stx/stream.h"
#include "stx/string.h"
#include "stx/task/chain.h"
#include "stx/task/priority.h"
#include "stx/vec.h"
#include "stx/void.h"

STX_BEGIN_NAMESPACE

using namespace std::chrono_literals;
using std::chrono::nanoseconds;
using TimePoint = std::chrono::steady_clock::time_point;

struct TaskTraceInfo
{
  Rc<std::string_view> content = string::rc::make_static_view("[Unspecified Context]");
  Rc<std::string_view> purpose = string::rc::make_static_view("[Unspecified Purpose]");
};

enum class TaskReady : uint8_t
{
  No,
  Yes
};

constexpr TaskReady task_is_ready(nanoseconds)
{
  return TaskReady::Yes;
}

// NOTE: scheduler isn't thread-safe. don't submit tasks to them from the
// tasks.  <<<======= this needs to go, we need to allow this somehow
//
// TODO(lamarrr): make scheduler thread-safe
//
//
//
// TODO(lamarrr): instanced tasks, i.e. RcFn<void(usize instance_index)> task; create([](usize){}, 256);
//
// use callbacks for task trace info?
//
struct Task
{
  // this is the final task to be executed on the target thread.
  // must only be invoked by one thread at a point in time.
  //
  RcFn<void()> function = fn::rc::make_static([]() {});

  // used to ask if the task is ready for execution.
  //
  // this is used for awaiting of futures or events.
  //
  // called on scheduler thread.
  //
  //
  // argument is time past since schedule.
  //
  UniqueFn<TaskReady(nanoseconds)> poll_ready = fn::rc::make_unique_static(task_is_ready);

  // used for tracking cancelation request and progress of the task
  PromiseAny scheduler_promise;

  // the task's identifier
  TaskId task_id{0};

  // priority to use in evaluating CPU-time worthiness
  TaskPriority priority = NORMAL_PRIORITY;

  // time since task was scheduled for execution
  TimePoint schedule_timepoint{};

  // information needed for tracing & profiling of tasks
  TaskTraceInfo trace_info{};
};

// scheduler just dispatches to the task timeline once the tasks are
// ready for execution
struct TaskScheduler
{
  TaskScheduler(Allocator iallocator, TimePoint ireference_timepoint) :
      allocator{iallocator},
      reference_timepoint{ireference_timepoint},
      entries{iallocator},
      cancelation_promise{make_promise<void>(iallocator).unwrap()},
      next_task_id{0},
      thread_pool{iallocator},
      timeline{iallocator}
  {}

  // if task is a ready one, add it to the schedule timeline immediately
  void tick(nanoseconds interval)
  {
    TimePoint present = std::chrono::steady_clock::now();

    Span ready_tasks = entries.span().partition([present](Task const &task) { return task.poll_ready.handle(present - task.schedule_timepoint) == TaskReady::No; }).second;

    for (Task &task : ready_tasks)
    {
      timeline
          .add_task(std::move(task.function), std::move(task.scheduler_promise), task.task_id, task.priority, present)
          .unwrap();
    }

    entries.erase(ready_tasks);

    timeline.tick(thread_pool.get_thread_slots(), present);
    thread_pool.tick(interval);

    // if cancelation requested,
    // begin shutdown sequence
    // cancel non-critical tasks
    if (cancelation_promise.fetch_cancel_request() == CancelState::Canceled)
    {
      thread_pool.get_future().request_cancel();
    }
  }

  Allocator        allocator;
  TimePoint        reference_timepoint;
  Vec<Task>        entries;
  Promise<void>    cancelation_promise;
  uint64_t         next_task_id;
  ThreadPool       thread_pool;
  ScheduleTimeline timeline;
};

// Processes stream operations on every tick.
// operations include:
// map
// filter
// reduce (ends on {N} yields or when stream is closed)
// enumerate
// fork
// join
/*

enum class StopStreaming { No, Yes };

// this should run until we poll for data and told that the stream is closed.
// ...
//
// Poll frequency???
//
struct StreamTask {
  // throttle, map, reduce, forward, connect, split, etc.
  // must only be invoked by one thread at a point in time.
  RcFn<void()> function;
  RcFn<StopStreaming(nanoseconds)> should_stop;
  // will be used to schedule tasks onto threads once the stream chunks are
  // available
  TaskPriority priority = NORMAL_PRIORITY;
  TaskTraceInfo trace_info;
};

struct StreamPipeline {
  template <typename Fn, typename T, typename U>
  void map(Fn&& transform, Stream<T>&& input, Generator<U>&& output) {
    fn::rc::make_functor(
        os_allocator,
        [transform_ = std::move(transform), input_ = std::move(input),
         output_ = std::move(output)]() {
          input_.pop().match(
              [](T&& input) { output_.yield(transform_(input)); }, []() {});
        });
  }

  template <typename Predicate, typename T, typename U>
  void filter(Predicate&& predicate, Stream<T>&& input,
              Generator<U>&& output) {
    fn::rc::make_functor(
        os_allocator,
        [predicate_ = std::move(predicate), input_ = std::move(input),
         output_ = std::move(output)]() {
          input_.pop().match(
              [](T&& input) {
                if (predicate_(input)) output_.yield(std::move(input));
              },
              []() {});
        });
  }

  template <typename Operation, typename T, typename... U, typename V>
  void fork(Operation&& operation, Stream<T>&& input,
            Generator<U>&&... ouputs) {
    //
    //
  }

  template <typename Operation, typename T, typename... U>
  void join(Operation&& operation, Generator<T>&&, Stream<U>&&...) {
    //
    //
  }

  Vec<RcFn<void()>> jobs;
};
*/

STX_END_NAMESPACE
