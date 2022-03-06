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
#include "stx/fn.h"
#include "stx/mem.h"
#include "stx/option.h"
#include "stx/scheduler/thread_pool.h"
#include "stx/scheduler/thread_slot.h"
#include "stx/scheduler/timeline.h"
#include "stx/stream.h"
#include "stx/string.h"
#include "stx/task/chain.h"
#include "stx/task/priority.h"
#include "stx/vec.h"
#include "stx/void.h"

namespace stx {

using namespace std::chrono_literals;
using std::string_view;
using std::chrono::nanoseconds;
using stx::Chain;
using stx::Fn;
using stx::Future;
using stx::FutureAny;
using stx::FutureStatus;
using stx::Promise;
using stx::PromiseAny;
using stx::Rc;
using stx::RcFn;
using stx::String;
using stx::StringView;
using stx::TaskPriority;
using stx::Vec;
using stx::Void;
using timepoint = std::chrono::steady_clock::time_point;
using stx::AllocError;
using stx::Result;

struct TaskTraceInfo {
  Rc<StringView> content =
      stx::string::rc::make_static_view("[Unspecified Context]");
  Rc<StringView> purpose =
      stx::string::rc::make_static_view("[Unspecified Purpose]");
};

namespace task_target {
struct main_thread {};
struct worker_threads {};
}  // namespace task_target

enum class TaskReady { No, Yes };

constexpr TaskReady task_is_ready(nanoseconds) { return TaskReady::Yes; }

// NOTE: scheduler isn't thread-safe. don't submit tasks to them from the
// tasks.
//
struct Task {
  // this is the final task to be executed on the target thread.
  // must only be invoked by one thread at a point in time.
  //
  RcFn<void()> function;

  // used to ask if the task is ready for execution.
  // called on scheduler thread.
  //
  // argument is time since schedule.
  //
  // this is used for awaiting of futures or events.
  //
  RcFn<TaskReady(nanoseconds)> poll_ready;

  std::chrono::steady_clock::time_point schedule_timepoint;

  stx::PromiseAny scheduler_promise;

  stx::TaskId task_id{0};

  TaskPriority priority = stx::NORMAL_PRIORITY;

  TaskTraceInfo trace_info;
};

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
  TaskPriority priority = stx::NORMAL_PRIORITY;
  TaskTraceInfo trace_info;
};

// used for:
//
// - conditional deferred scheduling i.e. if  a future is canceled, propagate
// the cancelation down the chain, or if an image decode task fails, propagate
// the error and don't schedule for loading on the GPU.
// - dynamic scheduling i.e. scheduling more tasks after a task has finished
//
// presents an advantage: shutdown is handled properly if all tasks are provided
// ahead of time.
//
// TODO(lamarrr): system cancelation??? coordination by the widgets??
//
struct DeferredTask {
  // always called on the main scheduler thread once the task is done. it will
  // always be executed even if the task is canceled or the executor begins
  // shutdown.
  //
  // used for mapping the output of a future onto another ??? i.e. wanting to
  // submit tasks from the task itself.
  //
  //
  // can be used to extend itself??? what about if it dynamically wants to
  // schedule on another executor??? will it be able to make that decision on
  // the executor
  //
  // can other executors do same?? i.e. if we want to do same for http executor
  //
  //
  // it's associated futures are pre-created and type-erased since we can't
  // figure that out later on
  //
  //
  // this can be used for implementing generators, though it'd probably need a
  // collection mechanism.
  //
  //
  //
  RcFn<void()> schedule;
  RcFn<TaskReady(nanoseconds)> poll_ready;
};

/*struct TaskData {
  Task task;
  // used to observe terminal state of the task by the scheduler,
  //
  // this is used for deferred_schedule and removing the task from the queue.
  //
  // shared across threads and needs to be captured by the packaged_task, thus
  // requiring it to be placed in a different address space from the
  // packaged_task.
  //
  //
  // we also shouldn't be relying on this future as a source of truth?
  // FutureAny future;

  // FutureStatus status_capture = FutureStatus::Scheduled;
};
*/

// TODO(lamarrr): scheduler just dispatches to the timeline once the tasks are
// ready
struct TaskScheduler {
  struct TaskThread {
    Rc<ThreadSlot*> task_slot;
    std::thread thread;
  };

  // if task is a ready one, add it to the schedule timeline immediately. this
  // should probably be renamed to the execution timeline.
  //
  TaskScheduler(timepoint ireference_timepoint, Allocator iallocator)
      : reference_timepoint{ireference_timepoint},
        entries{iallocator},
        cancelation_promise{stx::make_promise<void>(iallocator).unwrap()},
        timeline{iallocator},
        deferred_entries{iallocator},
        thread_pool{iallocator},
        allocator{iallocator} {}

  void tick(nanoseconds interval) {
    timepoint present = std::chrono::steady_clock::now();
    auto [___r, ready_tasks] =
        entries.span().partition([present](Task const& task) {
          return task.poll_ready.handle(present - task.schedule_timepoint) !=
                 TaskReady::Yes;
        });

    for (auto& task : ready_tasks) {
      timeline
          .add_task(std::move(task.function), std::move(task.scheduler_promise),
                    present, task.task_id, task.priority)
          .unwrap();
    }

    entries = stx::vec::erase(std::move(entries), ready_tasks);

    timeline.tick(thread_pool.get_thread_slots(), present);
    thread_pool.tick(interval);

    // if cancelation requested,
    // begin shutdown sequence
    // cancel non-critical tasks
    if (cancelation_promise.fetch_cancel_request().state ==
        RequestedCancelState::Canceled) {
      thread_pool.get_future().request_cancel();
    }
  }

  timepoint reference_timepoint;
  Vec<Task> entries;
  ScheduleTimeline timeline;
  Promise<void> cancelation_promise;
  uint64_t next_task_id{0};
  Vec<DeferredTask> deferred_entries;
  ThreadPool thread_pool;
  Allocator allocator;

  // Vec<Task> main_thread_tasks;
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
struct StreamPipeline {
  template <typename Fn, typename T, typename U>
  void map(Fn&& transform, stx::Stream<T>&& input, stx::Generator<U>&& output) {
    stx::fn::rc::make_functor(
        stx::os_allocator,
        [transform_ = std::move(transform), input_ = std::move(input),
         output_ = std::move(output)]() {
          input_.pop().match(
              [](T&& input) { output_.yield(transform_(input)); }, []() {});
        });
  }

  template <typename Predicate, typename T, typename U>
  void filter(Predicate&& predicate, stx::Stream<T>&& input,
              stx::Generator<U>&& output) {
    stx::fn::rc::make_functor(
        stx::os_allocator,
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
  void fork(Operation&& operation, stx::Stream<T>&& input,
            stx::Generator<U>&&... ouputs) {
    //
    //
  }

  template <typename Operation, typename T, typename... U>
  void join(Operation&& operation, stx::Generator<T>&&, stx::Stream<U>&&...) {
    //
    //
  }

  stx::Vec<stx::RcFn<void()>> jobs;
};
*/
}  // namespace stx
