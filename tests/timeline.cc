
#include "stx/scheduler/timeline.h"

#include <iostream>

#include "stx/scheduler.h"
#include "stx/scheduler/scheduling/await.h"
#include "stx/scheduler/scheduling/delay.h"
#include "stx/scheduler/scheduling/schedule.h"
#include "gtest/gtest.h"

using namespace std::chrono_literals;
using namespace stx;

#define STX_LOG(str) std::cout << str << std::endl;

TEST(ScheduleTimelineTest, Tick)
{
  auto timepoint = std::chrono::steady_clock::now();
  {
    ScheduleTimeline timeline{os_allocator};

    Vec<Rc<ThreadSlot *>> slots{noop_allocator};

    timeline.tick(slots.span(), timepoint);

    timeline
        .add_task(fn::rc::make_static([]() {}),
                  PromiseAny{make_promise<void>(os_allocator).unwrap()}, {},
                  NORMAL_PRIORITY, timepoint)
        .unwrap();

    timeline.tick(slots.span(), timepoint);

    EXPECT_EQ(timeline.thread_slots_capture.size(), 0);
  }

  {
    ScheduleTimeline timeline{os_allocator};

    Vec<Rc<ThreadSlot *>> slots{os_allocator};

    for (size_t i = 0; i < 10; i++)
      slots
          .push(rc::make_inplace<ThreadSlot>(
                    os_allocator, make_promise<void>(os_allocator).unwrap())
                    .unwrap())
          .unwrap();

    EXPECT_EQ(slots.size(), 10);

    for (size_t i = 0; i < 20; i++)
    {
      timeline
          .add_task(fn::rc::make_static([]() {}),
                    PromiseAny{make_promise<void>(os_allocator).unwrap()},
                    TaskId{0}, NORMAL_PRIORITY, timepoint)
          .unwrap();
    }

    timeline.tick(slots.span(), timepoint);
    EXPECT_EQ(slots.size(), 10);
    EXPECT_EQ(timeline.thread_slots_capture.size(), slots.size());

    EXPECT_EQ(timeline.starvation_timeline.size(), 20);
  }
}

void brr()
{}
int rx()
{
  return 0;
}

int first(Void)
{
  return 0;
}
int rx_loop(int64_t)
{
  return 0;
}

TEST(SchedulerTest, HH)
{
  TaskScheduler scheduler{os_allocator, std::chrono::steady_clock::now()};

  sched::fn(scheduler, []() { return 0; }, CRITICAL_PRIORITY, {});

  Future a = sched::fn(scheduler, rx, CRITICAL_PRIORITY, {});
  Future b =
      sched::chain(scheduler, Chain{first, rx_loop}, INTERACTIVE_PRIORITY, {});

  Future<float> c = sched::await_any(
      scheduler,
      [](Future<int> a, Future<int> b) {
        return (a.copy().unwrap_or(0) + b.copy().unwrap_or(0)) * 20.0f;
      },
      NORMAL_PRIORITY, {}, a.share(), b.share());

  sched::await(
      scheduler, [](Future<int>, Future<int>) {}, CRITICAL_PRIORITY, {},
      a.share(), b.share());

  sched::delay(
      scheduler, []() {}, NORMAL_PRIORITY, {}, 500ms);

  // join stream
  // split stream
  // filter
  // map
  // reduce streams
  // forloop combine with loop combine with others???
}

TEST(Scheduler, ThreadSlots)
{
  using namespace stx;

  ThreadSlot slot{make_promise<void>(os_allocator).unwrap()};

  ThreadSlot::Query query0 = slot.slot.query();

  EXPECT_TRUE(query0.can_push);
  EXPECT_TRUE(query0.executing_task.is_none());
  EXPECT_TRUE(query0.pending_task.is_none());

  slot.slot.push_task(
      ThreadSlot::Task{fn::rc::make_static([]() { STX_LOG("1"); }), TaskId{1}});

  // test that popping tasks from the queue works as expected
  EXPECT_TRUE(slot.slot.try_pop_task().is_some());

  // test for correct organization
  EXPECT_TRUE(query0.executing_task.is_none());
  EXPECT_TRUE(query0.pending_task.is_none());

  slot.slot.push_task(
      ThreadSlot::Task{fn::rc::make_static([]() {}), TaskId{1}});

  EXPECT_TRUE(query0.executing_task.is_none());
  EXPECT_TRUE(query0.pending_task.is_none());
}

TEST(Timeline, Sample)
{
  // Testing for work spreading amongst CPU cores

  ScheduleTimeline timeline{os_allocator};

  (void) timeline.add_task(fn::rc::make_static([]() { STX_LOG("1"); }),
                           PromiseAny{make_promise<void>(os_allocator).unwrap()},
                           TaskId{1}, NORMAL_PRIORITY,
                           std::chrono::steady_clock::now());

  (void) timeline.add_task(fn::rc::make_static([]() { STX_LOG("2"); }),
                           PromiseAny{make_promise<void>(os_allocator).unwrap()},
                           TaskId{2}, NORMAL_PRIORITY,
                           std::chrono::steady_clock::now());

  (void) timeline.add_task(fn::rc::make_static([]() { STX_LOG("3"); }),
                           PromiseAny{make_promise<void>(os_allocator).unwrap()},
                           TaskId{3}, NORMAL_PRIORITY,
                           std::chrono::steady_clock::now());

  (void) timeline.add_task(fn::rc::make_static([]() { STX_LOG("4"); }),
                           PromiseAny{make_promise<void>(os_allocator).unwrap()},
                           TaskId{4}, NORMAL_PRIORITY,
                           std::chrono::steady_clock::now());

  std::array<Rc<ThreadSlot *>, 4> slot{
      rc::make_inplace<ThreadSlot>(os_allocator,
                                   make_promise<void>(os_allocator).unwrap())
          .unwrap(),
      rc::make_inplace<ThreadSlot>(os_allocator,
                                   make_promise<void>(os_allocator).unwrap())
          .unwrap(),
      rc::make_inplace<ThreadSlot>(os_allocator,
                                   make_promise<void>(os_allocator).unwrap())
          .unwrap(),
      rc::make_inplace<ThreadSlot>(os_allocator,
                                   make_promise<void>(os_allocator).unwrap())
          .unwrap()};

  EXPECT_TRUE(slot[0]->slot.query().can_push);
  EXPECT_TRUE(slot[1]->slot.query().can_push);
  EXPECT_TRUE(slot[2]->slot.query().can_push);
  EXPECT_TRUE(slot[3]->slot.query().can_push);

  timeline.tick(Span{slot}.slice(0, 2), std::chrono::steady_clock::now());

  EXPECT_FALSE(slot[0]->slot.query().can_push);
  EXPECT_FALSE(slot[1]->slot.query().can_push);
  EXPECT_TRUE(slot[2]->slot.query().can_push);
  EXPECT_TRUE(slot[3]->slot.query().can_push);

  EXPECT_FALSE(slot[0]->slot.query().executing_task.is_some());
  EXPECT_FALSE(slot[1]->slot.query().executing_task.is_some());
  EXPECT_FALSE(slot[2]->slot.query().executing_task.is_some());
  EXPECT_FALSE(slot[3]->slot.query().executing_task.is_some());

  EXPECT_TRUE(slot[0]->slot.query().pending_task.is_some());
  EXPECT_TRUE(slot[1]->slot.query().pending_task.is_some());
  EXPECT_FALSE(slot[2]->slot.query().pending_task.is_some());
  EXPECT_FALSE(slot[3]->slot.query().pending_task.is_some());

  EXPECT_TRUE(timeline.starvation_timeline.span().is_sorted(
      [](ScheduleTimeline::Task const &a, ScheduleTimeline::Task const &b) {
        return a.priority < b.priority;
      }));
}
