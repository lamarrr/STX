#include <chrono>
#include <utility>

#include "stx/config.h"
#include "stx/scheduler.h"

STX_BEGIN_NAMESPACE

using namespace std::chrono_literals;
using std::chrono::nanoseconds;

namespace sched {

template <typename Fn>
auto delay(TaskScheduler &scheduler, Fn fn_task, TaskPriority priority,
           TaskTraceInfo trace_info, nanoseconds delay) {
  auto timepoint = std::chrono::steady_clock::now();
  TaskId task_id{scheduler.next_task_id};
  scheduler.next_task_id++;

  static_assert(std::is_invocable_v<Fn &>);

  using output = std::invoke_result_t<Fn &>;

  Promise promise{make_promise<output>(scheduler.allocator).unwrap()};
  Future future{promise.get_future()};
  PromiseAny scheduler_promise{promise.share()};

  UniqueFn<TaskReady(nanoseconds)> readiness_fn =
      fn::rc::make_unique_functor(scheduler.allocator, [delay](nanoseconds
                                                                   time_past) {
        return time_past >= delay ? TaskReady::Yes : TaskReady::No;
      }).unwrap();

  RcFn<void()> sched_fn =
      fn::rc::make_functor(scheduler.allocator, [fn_task_ = std::move(fn_task),
                                                 promise_ =
                                                     std::move(promise)]() {
        if constexpr (std::is_void_v<std::invoke_result_t<Fn &>>) {
          fn_task_();
          promise_.notify_completed();
        } else {
          promise_.notify_completed(fn_task_());
        }
      }).unwrap();

  scheduler.entries =
      vec::push(std::move(scheduler.entries),
                Task{std::move(sched_fn), std::move(readiness_fn), timepoint,
                     std::move(scheduler_promise), task_id, priority,
                     std::move(trace_info)})
          .unwrap();

  return future;
}

}  // namespace sched

STX_END_NAMESPACE