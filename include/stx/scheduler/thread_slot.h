#pragma once

#include <utility>

#include "stx/async.h"
#include "stx/config.h"
#include "stx/fn.h"
#include "stx/option.h"
#include "stx/spinlock.h"
#include "stx/task/id.h"

#define STX_STUB_ENSURE(...)

STX_BEGIN_NAMESPACE

struct ThreadSlot
{
  STX_MAKE_PINNED(ThreadSlot)

  enum class Status : uint8_t
  {
    Ready,
    Busy
  };

  struct Task
  {
    RcFn<void()> fn;
    TaskId       id{};
  };

  struct Query
  {
    bool           can_push = false;
    Option<TaskId> pending_task;
    Option<TaskId> executing_task;
  };

  // designed so tasks can be added even before the thread finishes processing
  // the one it is executing
  struct ____ThreadSlot
  {
    STX_MAKE_PINNED(____ThreadSlot)

    explicit ____ThreadSlot(Promise<void> ipromise) :
        promise{std::move(ipromise)}
    {}

    // sorts the executing and pending task.
    // relies on the fact that only one task can be executed at once.
    // tasks are marked as completed by the scheduler.
    auto try_pop_task() -> Option<RcFn<void()>>
    {
      LockGuard guard{lock};
      executing_task    = None;
      Option<Task> task = pending_task.take();
      if (task.is_none())
      {
        return None;
      }

      executing_task = Some(TaskId{task.value().id});
      return Some(std::move(task.value().fn));
    }

    void push_task(Task new_task)
    {
      LockGuard guard{lock};
      STX_STUB_ENSURE(pending_task.is_none(), "previously added task hasn't been processed yet. can_push() not checked before pushing");
      pending_task = Some(std::move(new_task));
    }

    Query query()
    {
      LockGuard guard{lock};
      Query     query;
      query.can_push       = pending_task.is_none();
      query.executing_task = executing_task;
      query.pending_task   = pending_task.as_cref().map([](Task const &task) { return task.id; });

      return query;
    }

    Promise<void> promise;

  private:
    // task is just 3 pointers and a bool, it's pointless using a mutex or any
    // sleeping lock on its R/W operations
    SpinLock       lock;
    Option<Task>   pending_task;
    Option<TaskId> executing_task;
  };

  explicit ThreadSlot(Promise<void> ipromise) :
      slot{std::move(ipromise)}
  {}

  STX_CACHELINE_ALIGNED ____ThreadSlot slot;
};

STX_END_NAMESPACE
