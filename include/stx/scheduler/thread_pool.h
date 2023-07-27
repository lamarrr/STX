#pragma once
#include <algorithm>
#include <chrono>
#include <cinttypes>
#include <thread>
#include <utility>

#include "stx/async.h"
#include "stx/config.h"
#include "stx/fn.h"
#include "stx/option.h"
#include "stx/rc.h"
#include "stx/scheduler/thread_slot.h"
#include "stx/spinlock.h"
#include "stx/vec.h"

STX_BEGIN_NAMESPACE

using std::chrono::milliseconds;
using std::chrono::nanoseconds;

using TimePoint = std::chrono::steady_clock::time_point;

namespace impl
{

// maximum must be a multiple of 2
//
// sleep: 1ms -> 2ms -> 4ms -> 8ms -> 16ms -> 32ms ..... clamped at `maximum`
//
// nanosecond/microsecond sleeps typically don't save much of power.
//
constexpr milliseconds bounded_exponential_backoff(uint64_t     iteration,
                                                   milliseconds maximum)
{
  uint32_t delay = static_cast<uint32_t>(1) << std::clamp<uint64_t>(iteration, 0, 31);

  milliseconds delay_ms{delay};

  return std::clamp<milliseconds>(delay_ms, milliseconds{}, maximum);
}

}        // namespace impl

// NOTE: `std::thread` performs memory allocations, and possibly throws. we can
// afford this since it'd happen at program startup anyway.
//
//
struct ThreadPool
{
  enum class State : uint8_t
  {
    Running,
    ShuttingDown,
    Shutdown
  };

  static constexpr milliseconds STALL_TIMEOUT{16};
  static constexpr milliseconds CANCELATION_POLL_MIN_PERIOD{32};

  static_assert((STALL_TIMEOUT.count() % 2) == 0);

  explicit ThreadPool(Allocator allocator) :
      num_threads{std::max<size_t>(std::thread::hardware_concurrency(), 1)},
      threads{vec::make_fixed<std::thread>(allocator, num_threads).unwrap()},
      thread_slots{
          vec::make_fixed<Rc<ThreadSlot *>>(allocator, num_threads).unwrap()},
      promise{make_promise<void>(allocator).unwrap()}
  {
    promise.notify_executing();

    for (size_t i = 0; i < num_threads; i++)
    {
      thread_slots.push_inplace(rc::make_inplace<ThreadSlot>(allocator, make_promise<void>(allocator).unwrap()).unwrap()).unwrap();
    }

    for (size_t i = 0; i < num_threads; i++)
    {
      threads
          .push_inplace([slot = &thread_slots.span()[i].handle->slot] {
            // TODO(lamarrr): separate this and ensure no thread
            // panics whilst holding a lock or even communicating
            uint64_t eventless_polls = 0;
            while (true)
            {
              CancelState requested_cancel_state = slot->promise.fetch_cancel_request();

              if (requested_cancel_state == CancelState::Canceled)
              {
                slot->promise.notify_canceled();
                return;
              }

              TimePoint task_poll_begin = std::chrono::steady_clock::now();

              // keep polling for tasks and executing them as long as
              // we are within the time limit.
              //
              // once the time limit is reached we need to poll for
              // cancelation of the thread
              //
              auto now = task_poll_begin;
              while ((now - task_poll_begin) < CANCELATION_POLL_MIN_PERIOD)
              {
                Option task = slot->try_pop_task();
                if (task.is_some())
                {
                  task.value().handle();
                  eventless_polls = 0;
                }
                else
                {
                  eventless_polls++;
                  milliseconds sleep_duration = impl::bounded_exponential_backoff(eventless_polls, STALL_TIMEOUT);
                  std::this_thread::sleep_until(now + sleep_duration);
                }

                now = std::chrono::steady_clock::now();
              }
            }
          })
          .unwrap();
    }
  }

  ~ThreadPool()
  {
    for (auto &slot : thread_slots.span())
    {
      slot.handle->slot.promise.request_cancel();
    }

    for (std::thread &thread : threads.span())
    {
      thread.join();
    }
  }

  Span<Rc<ThreadSlot *> const> get_thread_slots() const
  {
    return thread_slots.span().as_const();
  }

  FutureAny get_future()
  {
    return FutureAny{promise.get_future()};
  }

  void tick(nanoseconds)
  {
    switch (state)
    {
      case State::Running:
      {
        if (promise.fetch_cancel_request() == CancelState::Canceled)
        {
          for (auto &slot : thread_slots.span())
          {
            slot.handle->slot.promise.get_future().request_cancel();
          }
          state = State::ShuttingDown;
        }

        return;
      }

      case State::ShuttingDown:
      {
        if (thread_slots.span().is_all([](auto const &slot) { return slot.handle->slot.promise.is_done(); }))
        {
          state = State::Shutdown;
          promise.notify_canceled();
        }

        return;
      }

      case State::Shutdown:
      {
        return;
      }

      default:
      {
        return;
      }
    }
  }

private:
  size_t                num_threads = 0;
  FixedVec<std::thread> threads;
  // slots are not stored in the thread's lambda since we want to push tasks
  // onto them
  FixedVec<Rc<ThreadSlot *>> thread_slots;
  Promise<void>              promise;
  State                      state = State::Running;
};

STX_END_NAMESPACE