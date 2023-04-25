#pragma once

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <new>
#include <string_view>
#include <utility>

#include "stx/config.h"
#include "stx/enum.h"
#include "stx/limits.h"
#include "stx/panic/report.h"
#include "stx/rc.h"
#include "stx/result.h"
#include "stx/spinlock.h"
#include "stx/struct.h"

// exception-safety: absolute zero
// we don't use exceptions and neither do we plan to support it

STX_BEGIN_NAMESPACE

// interactions are ordered in such a way that the executor will not get in the
// way of the user and the user will not get in the way of the executor. this is
// the desired behaviour for User Interfaces.

// source:
// https://en.cppreference.com/w/cpp/thread/hardware_destructive_interference_size
#ifdef __cpp_lib_hardware_interference_size
constexpr size_t HARDWARE_CONSTRUCTIVE_INTERFERENCE_SIZE =
    std::hardware_constructive_interference_size;
constexpr size_t HARDWARE_DESTRUCTIVE_INTERFERENCE_SIZE =
    std::hardware_destructive_interference_size;
#else
// 64 bytes on x86-64 │ L1_CACHE_BYTES │ L1_CACHE_SHIFT │ __cacheline_aligned │
// ...
constexpr size_t HARDWARE_CONSTRUCTIVE_INTERFERENCE_SIZE =
    2 * sizeof(std::max_align_t);
constexpr size_t HARDWARE_DESTRUCTIVE_INTERFERENCE_SIZE =
    2 * sizeof(std::max_align_t);
#endif

STX_END_NAMESPACE

// each CPU core has its cache line, cache lines optimize for reading and
// writing to main memory which is slow. while multi-threading or using async,
// we need to communicate across threads which could map to CPU cores. the
// memory addresses are shared across CPU cores, as such we need to ensure we
// are not performing false sharing across these cores.
//
// False sharing leads to excessive cache flushes and thus reduces
// multi-threaded performances as the CPU now has to read from main memory which
// is the slowest read path. sharing atomics that arent aligned to the L1 cache
// size would lead to excessive flushing across each CPU core's cache line on
// write to the cache line of either core.
//
// A ripple effect as each CPU core's cache line entry for the cached address of
// the atomics has now been invalidated and each CPU core's cache now has to
// reload from main memory
#define STX_CACHELINE_ALIGNED alignas(::stx::HARDWARE_DESTRUCTIVE_INTERFERENCE_SIZE)

STX_BEGIN_NAMESPACE

/// the future's status are mutually exclusive. i.e. no two can exist at once.
/// and some states might be skipped or never occur or observed during the async
/// operation.
/// NOTE: only the future's terminal states are guaranteed to have any side
/// effect on the program's state. the other states are just for informational
/// purposes as such, can't be relied upon.
///
///
///
/// Implementation Note: this enum is typically used in relaxed memory order. It
/// is only used in release memory order if it enters the `Completed` state and
/// the executor makes non-atomic changes within the task's scope i.e. setting a
/// completion result to the shared future state object.
///
/// future status are updated only by the executor.
///
///
/// what is a terminal state? A terminal state is a state where the executor no
/// longer sends notifications or values via the Promise object.
///
///
enum class FutureStatus : uint8_t
{
  /// the async operation has been submitted to the scheduler and is scheduled
  /// for execution.
  ///
  /// `REQUIRED STATE?`: Yes. this is the default-initialized state of the
  /// future.
  ///
  /// `INTENDED FOR`: executors that wish to notify of task scheduling state.
  ///
  Scheduled,
  /// the async operation has been submitted by the scheduler to the execution
  /// unit for execution.
  ///
  /// `REQUIRED STATE?`: No, can be skipped. Set only if the executor has a
  /// task scheduler.
  ///
  /// `INTENDED FOR`: executors that wish to notify of task submission.
  ///
  Submitted,
  /// the task has been preempted by the scheduler.
  ///
  /// `REQUIRED STATE?`: No, can be skipped. Set only if the executor has a
  /// task scheduler.
  ///
  /// `INTENDED FOR`: executors that wish to notify of task preemption.
  ///
  Preempted,
  /// the async operation is now being executed by the executor.
  /// this can also mean that the task has been resumed from the suspended or
  /// force suspended state.
  ///
  /// `REQUIRED STATE?`: No, can be skipped.
  ///
  /// `INTENDED FOR`: executors that wish to notify of task execution. an
  /// immediately-executing executor might need to avoid the
  /// overhead of an atomic operation (i.e. notification of the Future's
  /// state).
  ///
  Executing,
  /// the async operation is now being canceled due to a cancelation request.
  ///
  /// `REQUIRED STATE?`: No, can be skipped. Set only if the executor supports
  /// cancelation and cancelation has been requested.
  ///
  /// `INTENDED FOR`: cancelable executors with prolonged or staged
  /// cancelation procedures.
  ///
  Canceling,
  /// the async operation is now being suspended.
  ///
  /// `REQUIRED STATE?`: No, can be skipped. Set only if the executor supports
  /// suspension and suspension has been requested.
  ///
  /// `INTENDED FOR`: suspendable executors with prolonged or staged
  /// suspension procedures.
  ///
  /// `IMPLEMENTATION REQUIREMENT`: must be preceded by the `Executing` state.
  ///
  Suspending,
  /// the async operation is now being suspended.
  ///
  /// `REQUIRED STATE?`: No, can be skipped. Set only if the executor supports
  /// suspension and suspension has been requested.
  ///
  /// `INTENDED FOR`: suspendable executors with prolonged or staged
  /// suspension procedures.
  ///
  /// `IMPLEMENTATION REQUIREMENT`: must be preceded by the `Executing` state.
  ///
  Suspended,
  /// the async operation is being resumed.
  ///
  /// `REQUIRED STATE?`: No, can be skipped. Set only if the executor supports
  /// suspension and resumption has been requested.
  ///
  /// `INTENDED FOR`: executors with prolonged or staged resumption
  /// procedures.
  ///
  /// `IMPLEMENTATION REQUIREMENT`: must preceded by the `Executing` and
  /// `Suspending` states.
  ///
  Resuming,
  /// the async operation has been canceled.
  ///
  /// `REQUIRED STATE?`: No, can be skipped. Set only if the executor supports
  /// cancelation and cancelation has been requested.
  ///
  /// `IMPLEMENTATION REQUIREMENT`: must be a terminal state for cancelable
  /// executors.
  ///
  Canceled,
  /// the async operation is being completed.
  ///
  /// `REQUIRED STATE?`: No, can be skipped.
  ///
  /// `INTENDED FOR`: executors with prolonged or staged completion procedure.
  ///
  Completing,
  /// the async operation has been completed.
  ///
  /// `REQUIRED STATE?`: Yes, if async operation is complete-able. must be set
  /// once
  /// the async operation has been completed.
  /// this implies that completion is not required i.e. a forever-running task
  /// that never completes.
  ///
  /// `IMPLEMENTATION REQUIREMENT`: must be a terminal state for executors on
  /// complete-able tasks.
  ///
  Completed,
  // reserved
  ____Pending = U8_MAX
};

enum class InfoFutureStatus : uint8_t
{
  Scheduled  = enum_uv(FutureStatus::Scheduled),
  Submitted  = enum_uv(FutureStatus::Submitted),
  Preempted  = enum_uv(FutureStatus::Preempted),
  Executing  = enum_uv(FutureStatus::Executing),
  Canceling  = enum_uv(FutureStatus::Canceling),
  Suspending = enum_uv(FutureStatus::Suspending),
  Suspended  = enum_uv(FutureStatus::Suspended),
  Resuming   = enum_uv(FutureStatus::Resuming),
};

enum class TerminalFutureStatus : uint8_t
{
  Canceled   = enum_uv(FutureStatus::Canceled),
  Completing = enum_uv(FutureStatus::Completing),
  Completed  = enum_uv(FutureStatus::Completed),
  Pending    = enum_uv(FutureStatus::____Pending)
};

enum class FutureError : uint8_t
{
  /// the async operation is pending and not yet finalized
  Pending,
  /// the async operation has been canceled
  Canceled
};

inline std::string_view operator>>(ReportQuery, FutureError const &error)
{
  switch (error)
  {
    case FutureError::Canceled:
      return "Canceled";
    case FutureError::Pending:
      return "Pending";
    default:
      return "";
  }
}

/// the executor might not be able to immediately respond to the requested
/// states of the async operations. the executor might not even be able to
/// attend to it at all.
enum class CancelState : uint8_t
{
  // the target is has not requested for cancelation
  Executing,
  // the target has requested for cancelation
  Canceled
};

/// the executor might not be able to immediately respond to the requested
/// states of the async operations. the executor might not even be able to
/// attend to it at all. this implies that if the user requests for resumption
/// of the task and immediately requests for suspension, the last requested
/// state takes effect and is the state observed by the executor.
///
///
enum class SuspendState : uint8_t
{
  // the target has requested for resumption
  Executing,
  // the target has requested for suspension
  Suspended
};

enum class PreemptState : uint8_t
{
  // the target has requested for resumption
  Executing,
  // the target has requested for preemption
  Preempted
};

enum class RequestType : uint8_t
{
  Suspend,
  Cancel,
  Preempt
};

/// returned by functions to signify why they returned.
///
/// NOTE: this is a plain data structure and doesn't check if a request was sent
/// or not
struct ServiceToken
{
  explicit constexpr ServiceToken(RequestType req_type) :
      type{req_type}
  {}

  RequestType type = RequestType::Suspend;
};

// this struct helps gurantee ordering of instructions relative
// to the future's shared state. it doesn't guarentee ordering of
// instructions relative to the function scope or program state itself. or even
// the async operation's associated task, the user has to take care of that
// themselves.
//
// non-terminal unsequenced updates to the future's states don't affect what the
// user observes. this helps with consistency of observation of terminal state
// but not for the non-terminal ones.
//
//
struct FutureExecutionState
{
  STX_DEFAULT_CONSTRUCTOR(FutureExecutionState)
  STX_MAKE_PINNED(FutureExecutionState)

  void executor____notify_scheduled()
  {
    notify_info(InfoFutureStatus::Scheduled);
  }

  void executor____notify_submitted()
  {
    notify_info(InfoFutureStatus::Submitted);
  }

  void executor____notify_preempted()
  {
    notify_info(InfoFutureStatus::Preempted);
  }

  void executor____notify_executing()
  {
    notify_info(InfoFutureStatus::Executing);
  }

  void executor____notify_canceling()
  {
    notify_info(InfoFutureStatus::Canceling);
  }

  void executor____notify_suspending()
  {
    notify_info(InfoFutureStatus::Suspending);
  }

  void executor____notify_suspended()
  {
    notify_info(InfoFutureStatus::Suspended);
  }

  void executor____notify_resuming()
  {
    notify_info(InfoFutureStatus::Resuming);
  }

  void executor____notify_canceled()
  {
    notify_term_no_result(TerminalFutureStatus::Canceled);
  }

  void executor____complete____with_void()
  {
    notify_term_no_result(TerminalFutureStatus::Completed);
  }

  template <typename Lambda>
  void executor____complete____with_result(Lambda &&setter_op)
  {
    {
      TerminalFutureStatus expected = TerminalFutureStatus::Pending;
      TerminalFutureStatus target   = TerminalFutureStatus::Completing;

      // if the future has already reached a terminal state (canceled,
      // completed, force-canceled) or another thread is completing the future,
      // then we can't proceed to modify the future's terminal state or  the
      // shared storage
      //
      // satisfies: the shared data storage is only ever written to once by all
      // the executors satisfies: the terminal state is only ever updated once.
      //

      // the future has not already reached a terminal state
      if (term.compare_exchange_strong(expected, target,
                                       std::memory_order_relaxed,
                                       std::memory_order_relaxed))
      {
        std::forward<Lambda>(setter_op)();
        term.store(TerminalFutureStatus::Completed, std::memory_order_release);
      }
      else
      {
        // already completed, completing, canceled, or force canceled
      }
    }
  }

  FutureStatus user____fetch_status____with_no_result() const
  {
    return fetch_status(std::memory_order_relaxed);
  }

  // acquires write operations and stored value that happened on the
  // executor thread, ordered around the terminal status
  FutureStatus user____fetch_status____with_result() const
  {
    // satisfies: the ordering of instructions to the shared storage is
    // consistent with the order in which it was written from the executor
    // thread.
    return fetch_status(std::memory_order_acquire);
  }

  bool user____is_done() const
  {
    switch (user____fetch_status____with_no_result())
    {
      case FutureStatus::Canceled:
      case FutureStatus::Completed:
        return true;

      default:
        return false;
    }
  }

private:
  FutureStatus fetch_status(std::memory_order terminal_load_mem_order) const
  {
    // satisfies: the terminal status overrides the informational status and is
    // always checked before the informational status
    //
    // NOTE: that no data is shared or communicated with the user prior to
    // completion of the future so the sequence in which the user observes the
    // statuses don't matter.
    //
    TerminalFutureStatus term_status = term.load(terminal_load_mem_order);

    switch (term_status)
    {
      case TerminalFutureStatus::Pending:
      {
        InfoFutureStatus info_status = info.load(std::memory_order_relaxed);
        return FutureStatus{enum_uv(info_status)};
      }

      default:
      {
        return FutureStatus{enum_uv(term_status)};
      }
    }
  }

  void notify_info(InfoFutureStatus status)
  {
    // the informational status can come in any order and don't need to be
    // sequenced
    // it doesn't need to be coordinated with the terminal status.
    info.store(status, std::memory_order_relaxed);
  }

  void notify_term_no_result(TerminalFutureStatus const status)
  {
    // satisfies that terminal state is only ever updated once
    TerminalFutureStatus expected = TerminalFutureStatus::Pending;
    term.compare_exchange_strong(expected, status, std::memory_order_relaxed,
                                 std::memory_order_relaxed);
  }

  std::atomic<InfoFutureStatus>     info{InfoFutureStatus::Scheduled};
  std::atomic<TerminalFutureStatus> term{TerminalFutureStatus::Pending};
};

struct FutureRequestState
{
  STX_DEFAULT_CONSTRUCTOR(FutureRequestState)
  STX_MAKE_PINNED(FutureRequestState)

  CancelState proxy____fetch_cancel_request() const
  {
    return requested_cancel_state.load(std::memory_order_relaxed);
  }

  SuspendState proxy____fetch_suspend_request() const
  {
    // when in a force suspended state, it is the sole responsibilty of the
    // executor to bring the async operation back to the resumed state and clear
    // the force suspend request
    //

    // satisfies: suspend and cancelation requests can come in any order and be
    // observed in any order.
    return requested_suspend_state.load(std::memory_order_relaxed);
  }

  PreemptState proxy____fetch_preempt_request() const
  {
    return requested_preempt_state.load(std::memory_order_relaxed);
  }

  void user____request_cancel()
  {
    // satisfies: cancelation request can only happen once and can't be cleared
    requested_cancel_state.store(CancelState::Canceled,
                                 std::memory_order_relaxed);
  }

  void user____request_resume()
  {
    // satisfies: suspend and resume can be requested across threads
    requested_suspend_state.store(SuspendState::Executing,
                                  std::memory_order_relaxed);
  }

  void user____request_suspend()
  {
    // satisfies: suspend and resume can be requested across threads
    requested_suspend_state.store(SuspendState::Suspended,
                                  std::memory_order_relaxed);
  }

  void executor____request_preempt()
  {
    requested_preempt_state.store(PreemptState::Preempted,
                                  std::memory_order_relaxed);
  }

  void executor____clear_preempt_request()
  {
    requested_preempt_state.store(PreemptState::Executing,
                                  std::memory_order_relaxed);
  }

private:
  // not cacheline aligned since this is usually requested by a single thread
  // and serviced by a single thread and we aren't performing millions of
  // cancelation/suspend requests at once (cold path).
  std::atomic<CancelState>  requested_cancel_state{CancelState::Executing};
  std::atomic<SuspendState> requested_suspend_state{SuspendState::Executing};
  std::atomic<PreemptState> requested_preempt_state{PreemptState::Executing};
};

struct FutureBaseState : public FutureExecutionState,
                         public FutureRequestState
{};

template <typename T>
struct FutureState : public FutureBaseState
{
  STX_DEFAULT_CONSTRUCTOR(FutureState)
  STX_MAKE_PINNED(FutureState)

  // this only happens once across all threads and the lifetime of the
  // futurestate.
  // only one executor will have access to this so no locking is required.
  void executor____complete_with_object(T &&value)
  {
    FutureBaseState::executor____complete____with_result(
        [&value, this] { executor____unsafe_init_storage(std::move(value)); });
  }

  Result<T, FutureError> user____copy_result()
  {
    FutureStatus status =
        FutureBaseState::user____fetch_status____with_result();

    switch (status)
    {
      case FutureStatus::Completed:
      {
        LockGuard guard{storage_lock};
        return Ok(unsafe_copy());
      }

      case FutureStatus::Canceled:
      {
        return Err(FutureError::Canceled);
      }

      default:
        return Err(FutureError::Pending);
    }
  }

  Result<T, FutureError> user____move_result()
  {
    FutureStatus status =
        FutureBaseState::user____fetch_status____with_result();

    switch (status)
    {
      case FutureStatus::Completed:
      {
        LockGuard guard{storage_lock};
        return Ok(unsafe_move());
      }

      case FutureStatus::Canceled:
      {
        return Err(FutureError::Canceled);
      }

      default:
        return Err(FutureError::Pending);
    }
  }

  Result<T *, FutureError> user____ref_result()
  {
    FutureStatus status =
        FutureBaseState::user____fetch_status____with_result();

    switch (status)
    {
      case FutureStatus::Completed:
      {
        LockGuard guard{storage_lock};
        return Ok(std::launder(reinterpret_cast<T *>(&storage)));
      }

      case FutureStatus::Canceled:
      {
        return Err(FutureError::Canceled);
      }

      default:
        return Err(FutureError::Pending);
    }
  }

  // allow drain operation to modify object. alt: reap
  // allow inspect object to copy object.
  //
  // destructor only runs when there's no reference to the future's state
  //
  ~FutureState()
  {
    // destructor only runs once when no other operation is happening on the
    // future. and only happens when it isn't used across threads, so locking is
    // not necessary.
    //
    // we do need to ensure the ordering of writes to the shared state relative
    // to this destroy operation (acquire).
    //
    // we don't need to release after destroying since changes can't be observed
    // after destructor is run.
    //
    FutureStatus status =
        FutureBaseState::user____fetch_status____with_result();

    switch (status)
    {
      case FutureStatus::Completed:
      {
        unsafe_destroy();
        return;
      }
      default:
        return;
    }
  }

private:
  // NOTE: we don't use mutexes on the final result of the async operation
  // since the executor will have exclusive access to the storage address
  // until the async operation is finished (completed, force canceled, or,
  // canceled).
  //
  std::aligned_storage_t<sizeof(T), alignof(T)> storage;

  // the producer (executor) only writes once to the future, but the future is a
  // shared one by default and is intended to be shared by multiple executors so
  // we need to use an extremely fast non-blocking lock so we don't get in the
  // way of the executors, and the result acquisition operation is expected to
  // be very fast, typically spanning 10 loads/writes.
  //
  // completion operations are extremely fast. i.e. std::move.
  //
  // For a shared future, if you're performing a deep copy and then another
  // executor or execution unit happens to be requesting access to the shared
  // storage whilst you're doing so, you'd degrade your performance benefits as
  // the executor would block for prolonged period of time wasting processor
  // cycles. You should ideally perform copy of trivial types or shallow copies
  // during this.
  //
  // i.e. copying a very large vector whilst holding this lock, will block all
  // other executors requesting access to the storage and thus waste a lot of
  // processor time. it is therefore recommended to return a shared vector and
  // then shallow copy it. but be careful of modifications, if you need to
  // modify across threads then perform the task of copying across different
  // tasks.
  //
  SpinLock storage_lock;

  // sends in the result of the async operation.
  // calling this function implies that the async operation has completed.
  // this function must only be called once otherwise could potentially lead to
  // a memory leak.
  // this is enforced by the atomic terminal state check via CAS.
  void executor____unsafe_init_storage(T &&value)
  {
    new (&storage) T{std::move(value)};
  }

  T &unsafe_launder_writable()
  {
    return *std::launder(reinterpret_cast<T *>(&storage));
  }

  T const &unsafe_launder_readable() const
  {
    return *std::launder(reinterpret_cast<T const *>(&storage));
  }

  T &&unsafe_move()
  {
    return std::move(unsafe_launder_writable());
  }

  T unsafe_copy() const
  {
    return unsafe_launder_readable();
  }

  void unsafe_destroy()
  {
    unsafe_launder_writable().~T();
  }
};

template <>
struct FutureState<void> : public FutureBaseState
{
  STX_DEFAULT_CONSTRUCTOR(FutureState)
  STX_MAKE_PINNED(FutureState)
};

template <typename T>
struct FutureBase
{
  STX_DISABLE_DEFAULT_CONSTRUCTOR(FutureBase)

  explicit FutureBase(Rc<FutureState<T> *> init_state) :
      state{std::move(init_state)}
  {}

  FutureStatus fetch_status() const
  {
    return state.handle->user____fetch_status____with_no_result();
  }

  void request_cancel() const
  {
    state.handle->user____request_cancel();
  }

  void request_suspend() const
  {
    state.handle->user____request_suspend();
  }

  void request_resume() const
  {
    state.handle->user____request_resume();
  }

  bool is_done() const
  {
    return state.handle->user____is_done();
  }

  Rc<FutureState<T> *> state;
};

// this is contrary to the on-finished callback approach in which the user is
// very likely to use incorrectly due instruction re-ordering or order of
// observation of changes.
//
//
// This Future type helps the user from writing excessive code to track the
// state of the async operation or having to maintain, manage, track, and
// implement numerous cancelation tokens and suspension tokens. or ad-hoc,
// error-prone, and costly approaches like helps prevent the user from writing
// ugly hacks like `std::shared_ptr<std::atomic<CancelationRequest>>` which they
// might not even use correctly.
//
// Futures observes effects of changes from the executor
//
template <typename T>
struct Future : public FutureBase<T>
{
  using Base = FutureBase<T>;

  STX_DISABLE_DEFAULT_CONSTRUCTOR(Future)

  explicit Future(Rc<FutureState<T> *> init_state) :
      Base{std::move(init_state)}
  {}

  // copy operations should be extremely fast
  Result<T, FutureError> copy() const
  {
    return Base::state.handle->user____copy_result();
  }

  // move operations should be extremely fast
  Result<T, FutureError> move() const
  {
    return Base::state.handle->user____move_result();
  }

  Result<T *, FutureError> ref() const
  {
    return Base::state.handle->user____ref_result();
  }

  Future share() const
  {
    return Future{Base::state.share()};
  }

  // user ref result
  // invocable(ref)

  // TODO(lamarrr):
  // mutable or immutable ref arg as appropriate
  // map operations should be extremely fast
  // .map () -> Result<U, FutureError> {}
};

template <>
struct Future<void> : public FutureBase<void>
{
  using Base = FutureBase<void>;

  STX_DISABLE_DEFAULT_CONSTRUCTOR(Future)

  explicit Future(Rc<FutureState<void> *> init_state) :
      Base{std::move(init_state)}
  {}
};

struct FutureAny
{
  STX_DISABLE_DEFAULT_CONSTRUCTOR(FutureAny)

  template <typename T>
  explicit FutureAny(Future<T> future) :
      state{cast<FutureBaseState *>(std::move(future.state))}
  {}

  explicit FutureAny(Rc<FutureBaseState *> istate) :
      state{std::move(istate)}
  {}

  FutureStatus fetch_status() const
  {
    return state.handle->user____fetch_status____with_no_result();
  }

  void request_cancel() const
  {
    state.handle->user____request_cancel();
  }

  void request_suspend() const
  {
    state.handle->user____request_suspend();
  }

  void request_resume() const
  {
    state.handle->user____request_resume();
  }

  bool is_done() const
  {
    return state.handle->user____is_done();
  }

  FutureAny share() const
  {
    return FutureAny{state.share()};
  }

  Rc<FutureBaseState *> state;
};

template <typename T>
struct PromiseBase
{
  STX_DISABLE_DEFAULT_CONSTRUCTOR(PromiseBase)

  explicit PromiseBase(Rc<FutureState<T> *> init_state) :
      state{std::move(init_state)}
  {}

  void notify_scheduled() const
  {
    state.handle->executor____notify_scheduled();
  }

  void notify_submitted() const
  {
    state.handle->executor____notify_submitted();
  }

  void notify_preempted() const
  {
    state.handle->executor____notify_preempted();
  }

  void notify_executing() const
  {
    state.handle->executor____notify_executing();
  }

  void notify_cancel_begin() const
  {
    state.handle->executor____notify_canceling();
  }

  void notify_canceled() const
  {
    state.handle->executor____notify_canceled();
  }

  void notify_suspend_begin() const
  {
    state.handle->executor____notify_suspending();
  }

  void notify_suspended() const
  {
    state.handle->executor____notify_suspended();
  }

  void notify_resume_begin() const
  {
    state.handle->executor____notify_resuming();
  }

  void request_cancel() const
  {
    state.handle->user____request_cancel();
  }

  void request_suspend() const
  {
    state.handle->user____request_suspend();
  }

  void request_resume() const
  {
    state.handle->user____request_resume();
  }

  void request_preempt() const
  {
    state.handle->executor____request_preempt();
  }

  void clear_preempt_request() const
  {
    state.handle->executor____clear_preempt_request();
  }

  CancelState fetch_cancel_request() const
  {
    return state.handle->proxy____fetch_cancel_request();
  }

  PreemptState fetch_preempt_request() const
  {
    return state.handle->proxy____fetch_preempt_request();
  }

  SuspendState fetch_suspend_request() const
  {
    return state.handle->proxy____fetch_suspend_request();
  }

  FutureStatus fetch_status() const
  {
    return state.handle->user____fetch_status____with_no_result();
  }

  bool is_done() const
  {
    return state.handle->user____is_done();
  }

  Future<T> get_future() const
  {
    return Future<T>{state.share()};
  }

  Rc<FutureState<T> *> state;
};

template <typename T>
struct Promise : public PromiseBase<T>
{
  using Base = PromiseBase<T>;

  STX_DISABLE_DEFAULT_CONSTRUCTOR(Promise)

  explicit Promise(Rc<FutureState<T> *> init_state) :
      Base{std::move(init_state)}
  {}

  void notify_completed(T &&value) const
  {
    Base::state.handle->executor____complete_with_object(std::move(value));
  }

  Promise share() const
  {
    return Promise{Base::state.share()};
  }
};

template <>
struct Promise<void> : public PromiseBase<void>
{
  using Base = PromiseBase<void>;

  STX_DISABLE_DEFAULT_CONSTRUCTOR(Promise)

  explicit Promise(Rc<FutureState<void> *> init_state) :
      Base{std::move(init_state)}
  {}

  void notify_completed() const
  {
    Base::state.handle->executor____complete____with_void();
  }

  Promise share() const
  {
    return Promise{Base::state.share()};
  }
};

struct PromiseAny
{
  STX_DISABLE_DEFAULT_CONSTRUCTOR(PromiseAny)

  template <typename T>
  explicit PromiseAny(Promise<T> promise) :
      state{cast<FutureBaseState *>(std::move(promise.state))}
  {}

  explicit PromiseAny(Rc<FutureBaseState *> istate) :
      state{std::move(istate)}
  {}

  void notify_scheduled() const
  {
    state.handle->executor____notify_scheduled();
  }

  void notify_submitted() const
  {
    state.handle->executor____notify_submitted();
  }

  void notify_preempted() const
  {
    state.handle->executor____notify_preempted();
  }

  void notify_executing() const
  {
    state.handle->executor____notify_executing();
  }

  void notify_cancel_begin() const
  {
    state.handle->executor____notify_canceling();
  }

  void notify_canceled() const
  {
    state.handle->executor____notify_canceled();
  }

  void notify_suspend_begin() const
  {
    state.handle->executor____notify_suspending();
  }

  void notify_suspended() const
  {
    state.handle->executor____notify_suspended();
  }

  void notify_resume_begin() const
  {
    state.handle->executor____notify_resuming();
  }

  void request_cancel() const
  {
    state.handle->user____request_cancel();
  }

  void request_suspend() const
  {
    state.handle->user____request_suspend();
  }

  void request_resume() const
  {
    state.handle->user____request_resume();
  }

  void request_preempt() const
  {
    state.handle->executor____request_preempt();
  }

  void clear_preempt_request() const
  {
    state.handle->executor____clear_preempt_request();
  }

  CancelState fetch_cancel_request() const
  {
    return state.handle->proxy____fetch_cancel_request();
  }

  PreemptState fetch_preempt_request() const
  {
    return state.handle->proxy____fetch_preempt_request();
  }

  SuspendState fetch_suspend_request() const
  {
    return state.handle->proxy____fetch_suspend_request();
  }

  FutureStatus fetch_status() const
  {
    return state.handle->user____fetch_status____with_no_result();
  }

  bool is_done() const
  {
    return state.handle->user____is_done();
  }

  FutureAny get_future() const
  {
    return FutureAny{state.share()};
  }

  PromiseAny share() const
  {
    return PromiseAny{state.share()};
  }

  Rc<FutureBaseState *> state;
};

struct RequestProxy
{
  STX_DISABLE_DEFAULT_CONSTRUCTOR(RequestProxy)

  template <typename T>
  explicit RequestProxy(Promise<T> const &promise) :
      state{cast<FutureBaseState *>(promise.state.share())}
  {}

  template <typename T>
  explicit RequestProxy(Future<T> const &future) :
      state{cast<FutureBaseState *>(future.state.share())}
  {}

  explicit RequestProxy(FutureAny const &future) :
      state{future.state.share()}
  {}

  explicit RequestProxy(Rc<FutureBaseState *> istate) :
      state{std::move(istate)}
  {}

  CancelState fetch_cancel_request() const
  {
    return state.handle->proxy____fetch_cancel_request();
  }

  PreemptState fetch_preempt_request() const
  {
    return state.handle->proxy____fetch_preempt_request();
  }

  SuspendState fetch_suspend_request() const
  {
    return state.handle->proxy____fetch_suspend_request();
  }

  RequestProxy share() const
  {
    return RequestProxy{state.share()};
  }

  Rc<FutureBaseState *> state;
};

template <typename T>
Result<Promise<T>, AllocError> make_promise(Allocator allocator)
{
  TRY_OK(shared_state, rc::make_inplace<FutureState<T>>(allocator));
  return Ok(Promise<T>{std::move(shared_state)});
}

STX_END_NAMESPACE
