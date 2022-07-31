
#include "stx/scheduler/timeline.h"

#include <iostream>

#include "gtest/gtest.h"
#include "stx/scheduler.h"
#include "stx/scheduler/scheduling/await.h"
#include "stx/scheduler/scheduling/delay.h"
#include "stx/scheduler/scheduling/schedule.h"

using namespace std::chrono_literals;
using namespace stx;

#define STX_LOG(str) std::cout << str << std::endl;

/*
#define

float rawrrr(float arg) {
  STX_LOG("rawwwrrrrrrr!!!!!!!!!!!!! {}", arg);
  return arg;
}

void gggg() {
  Promise promise = make_promise<int>(os_allocator).unwrap();
  Future future = promise.get_future();
  // auto [future2, promise2] = make_future<void>();

  auto fn = fn::rc::make_functor(os_allocator, [](int value) {
              STX_LOG("value {}", value);
            }).unwrap();

  auto g = fn::rc::make_static(rawrrr);

  g.handle(5);

  auto xg = fn::rc::make_static([](float a) { return rawrrr(a); });

  xg.handle(34);

  fn.handle(8);

  auto d = fn::rc::make_static(
      [](Future<int>, Future<void>) { STX_LOG("all ready!"); });

  Chain{[](Void) -> int {
               STX_LOG("executing 1 ...");
               std::this_thread::sleep_for(1s);
               return 0;
             },
             [](int x) {
               STX_LOG("executing 2 ...");
               std::this_thread::sleep_for(1s);
               return x + 1;
             },
             [](int x) {
               STX_LOG("executing 3 ...");
               std::this_thread::sleep_for(1s);
               return x + 2.5;
             },
             [](float y) {
               STX_LOG("executing 4 ...");
               std::this_thread::sleep_for(1s);
               return y + 5;
             },
             rawrrr};

  // d.get()();

  TaskPriority::Background, vlk::TaskTraceInfo{}, future,
  future2);

promise.notify_completed(8);
promise2.notify_completed();
*/
/*
  sched.schedule(make_static_fn([]() { return; }),
                 TaskPriority::Background, {});
  sched.schedule_chain(vlk::Chain{[](vlk::Void) -> int {
                                    STX_LOG("executing 1 ...");
                                    std::this_thread::sleep_for(1s);
                                    return 0;
                                  },
                                  [](int x) {
                                    STX_LOG("executing 2 ...");
                                    std::this_thread::sleep_for(1s);
                                    return x + 1;
                                  },
                                  [](int x) {
                                    STX_LOG("executing 3 ...");
                                    std::this_thread::sleep_for(1s);
                                    return x + 2.5;
                                  },
                                  [](float y) {
                                    STX_LOG("executing 4 ...");
                                    std::this_thread::sleep_for(1s);
                                    return y + 5;
                                  },
                                  rawrrr},
                       TaskPriority::Critical, vlk::TaskTraceInfo{});

  for (auto& task : sched.entries) {
    if (task.state == vlk::TaskScheduler::EntryState::Scheduled &&
        task.is_ready.get()()) {
      task.state = vlk::TaskScheduler::EntryState::Executing;
      task.packaged_task.get()();
    } else if (true) {
      // ... and others
    }
  }
  */

struct alignas(64) fuck {
  int y;
};

TEST(ScheduleTimelineTest, Tick) {
  auto timepoint = std::chrono::steady_clock::now();
  {
    ScheduleTimeline timeline{os_allocator};

    Vec<Rc<ThreadSlot *>> slots{noop_allocator};

    timeline.tick(slots.span(), timepoint);

    timeline
        .add_task(fn::rc::make_static([]() {}),
                  PromiseAny{make_promise<void>(os_allocator).unwrap()},
                  timepoint, {}, NORMAL_PRIORITY)
        .unwrap();

    timeline.tick(slots.span(), timepoint);

    EXPECT_EQ(timeline.thread_slots_capture.size(), 0);
  }

  {
    ScheduleTimeline timeline{os_allocator};

    Vec<Rc<ThreadSlot *>> slots{os_allocator};

    for (size_t i = 0; i < 10; i++)
      slots =
          vec::push(std::move(slots),
                    rc::make_inplace<ThreadSlot>(
                        os_allocator, make_promise<void>(os_allocator).unwrap())
                        .unwrap())
              .unwrap();

    for (size_t i = 0; i < 20; i++) {
      timeline
          .add_task(fn::rc::make_static([]() {}),
                    PromiseAny{make_promise<void>(os_allocator).unwrap()},
                    timepoint, TaskId{0}, NORMAL_PRIORITY)
          .unwrap();
    }

    timeline.tick(slots.span(), timepoint);
    EXPECT_EQ(slots.size(), 10);
    EXPECT_EQ(timeline.thread_slots_capture.size(), slots.size());

    EXPECT_EQ(timeline.starvation_timeline.size(), 20);
  }

  String h = string::make_static("Hello boy");
  String y = string::make(os_allocator, "Hello boy").unwrap();
  EXPECT_TRUE(h == "Hello boy");
  EXPECT_FALSE(h == "Hello Boy");

  EXPECT_TRUE(y == "Hello boy");
  EXPECT_TRUE(h == y);

  EXPECT_FALSE(h.starts_with("Hello world"));
}

void brr() {}
int rx() { return 0; }

int first(Void) { return 0; }
int rx_loop(int64_t) { return 0; }

TEST(SchedulerTest, HH) {
  using namespace stx;
  using namespace std::chrono_literals;

  TaskScheduler scheduler{os_allocator, std::chrono::steady_clock::now()};

  sched::fn(scheduler, []() { return 0; }, CRITICAL_PRIORITY, {});

  Future a = sched::fn(scheduler, rx, CRITICAL_PRIORITY, {});
  Future b =
      sched::chain(scheduler, Chain{first, rx_loop}, INTERACTIVE_PRIORITY, {});

  // TODO(lamarrr):
  // auto [a_d, b_d] =  reap/drain(a, b)
  // we need an operator to finialize on the await operation.
  // i.e. drain
  //
  //

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

TEST(Scheduler, ThreadSlots) {
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

TEST(Timeline, Sample) {
  using namespace stx;

  // Testing for work spreading amongst CPU cores

  ScheduleTimeline timeline{os_allocator};

  (void)timeline.add_task(fn::rc::make_static([]() { STX_LOG("1"); }),
                          PromiseAny{make_promise<void>(os_allocator).unwrap()},
                          std::chrono::steady_clock::now(), TaskId{1},
                          NORMAL_PRIORITY);

  (void)timeline.add_task(fn::rc::make_static([]() { STX_LOG("2"); }),
                          PromiseAny{make_promise<void>(os_allocator).unwrap()},
                          std::chrono::steady_clock::now(), TaskId{2},
                          NORMAL_PRIORITY);

  (void)timeline.add_task(fn::rc::make_static([]() { STX_LOG("3"); }),
                          PromiseAny{make_promise<void>(os_allocator).unwrap()},
                          std::chrono::steady_clock::now(), TaskId{3},
                          NORMAL_PRIORITY);

  (void)timeline.add_task(fn::rc::make_static([]() { STX_LOG("4"); }),
                          PromiseAny{make_promise<void>(os_allocator).unwrap()},
                          std::chrono::steady_clock::now(), TaskId{4},
                          NORMAL_PRIORITY);

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

  timeline.tick(Span{slot.data(), slot.size()},
                std::chrono::steady_clock::now());

  EXPECT_TRUE(slot[0].handle->slot.query().can_push);
  EXPECT_TRUE(slot[1].handle->slot.query().can_push);
  EXPECT_TRUE(slot[2].handle->slot.query().can_push);
  EXPECT_TRUE(slot[3].handle->slot.query().can_push);

  EXPECT_FALSE(slot[0].handle->slot.query().executing_task.is_some());
  EXPECT_FALSE(slot[1].handle->slot.query().executing_task.is_some());
  EXPECT_FALSE(slot[2].handle->slot.query().executing_task.is_some());
  EXPECT_FALSE(slot[3].handle->slot.query().executing_task.is_some());

  EXPECT_FALSE(slot[0].handle->slot.query().pending_task.is_some());
  EXPECT_FALSE(slot[1].handle->slot.query().pending_task.is_some());
  EXPECT_FALSE(slot[2].handle->slot.query().pending_task.is_some());
  EXPECT_FALSE(slot[3].handle->slot.query().pending_task.is_some());

  EXPECT_TRUE(std::is_sorted(
      timeline.starvation_timeline.begin(), timeline.starvation_timeline.end(),
      [](ScheduleTimeline::Task const &a, ScheduleTimeline::Task const &b) {
        return a.priority < b.priority;
      }));
}
