#pragma once

#include <tuple>
#include <utility>

#include "stx/scheduler.h"

namespace stx {
namespace sched {

// prepare a deferred task. Deferred tasks run on the  main thread and are
// typically used for dynamic scheduling.
template <typename Fn, typename FirstInput, typename... OtherInputs>
auto deferred(stx::TaskScheduler &scheduler, Fn schedule_task,
              Future<FirstInput> first_input,
              Future<OtherInputs>... other_inputs) {
  static_assert(std::is_invocable_v<Fn &, Future<FirstInput> &&,
                                    Future<OtherInputs> &&...>);

  using output = std::invoke_result_t<Fn &, Future<FirstInput> &&,
                                      Future<OtherInputs> &&...>;

  auto schedule_timepoint = std::chrono::steady_clock::now();

  stx::Promise promise =
      stx::make_promise<output>(scheduler.allocator).unwrap();
  stx::Future future = promise.get_future();

  std::array<FutureAny, 1 + sizeof...(OtherInputs)> await{
      FutureAny{first_input.share()}, FutureAny{other_inputs.share()}...};

  stx::RcFn<TaskReady(nanoseconds)> readiness =
      stx::fn::rc::make_functor(scheduler.allocator, [await_ = std::move(
                                                          await)](nanoseconds) {
        bool all_ready = std::all_of(
            await_.begin(), await_.end(),
            [](FutureAny const &future) { return future.is_done(); });
        return all_ready ? TaskReady::Yes : TaskReady::No;
      }).unwrap();

  std::tuple<Future<FirstInput>, Future<OtherInputs>...> await_futures{
      std::move(first_input), std::move(other_inputs)...};

  stx::RcFn<void()> schedule =
      stx::fn::rc::make_functor(
          scheduler.allocator,
          [schedule_task_ = std::move(schedule_task),
           await_futures_ = std::move(await_futures),
           promise_ = std::move(promise)]() mutable {
            if constexpr (!std::is_void_v<output>) {
              promise_.notify_completed(
                  std::apply(schedule_task_, std::move(await_futures_)));
            } else {
              schedule_task_(
                  std::apply(schedule_task_, std::move(await_futures_)));
              promise_.notify_completed();
            }
          })
          .unwrap();

  scheduler.deferred_entries =
      stx::vec::push(std::move(scheduler.deferred_entries),
                     DeferredTask{std::move(schedule), schedule_timepoint,
                                  std::move(readiness)})
          .unwrap();

  return future;
}

}  // namespace sched
}  // namespace stx
