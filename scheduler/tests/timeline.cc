// #include "stx/dynamic.h"
#include <iostream>

#include "stx/scheduler.h"
#include "stx/scheduler/scheduling/deferred.h"
using namespace std::chrono_literals;

#define STX_LOG(str) std::cout << str << std::endl;

/*
#define

float rawrrr(float arg) {
  STX_LOG("rawwwrrrrrrr!!!!!!!!!!!!! {}", arg);
  return arg;
}

void gggg() { 
  stx::Promise promise = stx::make_promise<int>(stx::os_allocator).unwrap();
  stx::Future future = promise.get_future();
  // auto [future2, promise2] = stx::make_future<void>();

  auto fn = stx::fn::rc::make_functor(stx::os_allocator, [](int value) {
              STX_LOG("value {}", value);
            }).unwrap();

  auto g = stx::fn::rc::make_static(rawrrr);

  g.handle(5);

  auto xg = stx::fn::rc::make_static([](float a) { return rawrrr(a); });

  xg.handle(34);

  fn.handle(8);

  auto d = stx::fn::rc::make_static(
      [](stx::Future<int>, stx::Future<void>) { STX_LOG("all ready!"); });

  stx::Chain{[](stx::Void) -> int {
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

  stx::TaskPriority::Background, vlk::TaskTraceInfo{}, future,
  future2);

promise.notify_completed(8);
promise2.notify_completed();
*/
  /*
    sched.schedule(stx::make_static_fn([]() { return; }),
                   stx::TaskPriority::Background, {});
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
                         stx::TaskPriority::Critical, vlk::TaskTraceInfo{});

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

#include "gtest/gtest.h"
#include "stx/scheduler/timeline.h"

struct alignas(64) fuck {
  int y;
};

TEST(ScheduleTimelineTest, Tick) {
  auto timepoint = std::chrono::steady_clock::now();
  {
    stx::ScheduleTimeline timeline{stx::os_allocator};

    stx::Vec<stx::Rc<stx::ThreadSlot *>> slots{stx::noop_allocator};

    timeline.tick(slots.span(), timepoint);

    timeline
        .add_task(stx::fn::rc::make_static([]() {}),
                  stx::PromiseAny{
                      stx::make_promise<void>(stx::os_allocator).unwrap()},
                  timepoint, {}, stx::NORMAL_PRIORITY)
        .unwrap();

    timeline.tick(slots.span(), timepoint);

    EXPECT_EQ(timeline.thread_slots_capture.size(), 0);
  }

  {
    stx::ScheduleTimeline timeline{stx::os_allocator};

    stx::Vec<stx::Rc<stx::ThreadSlot *>> slots{stx::os_allocator};

    for (size_t i = 0; i < 10; i++)
      slots = stx::vec::push(
                  std::move(slots),
                  stx::rc::make_inplace<stx::ThreadSlot>(
                      stx::os_allocator,
                      stx::make_promise<void>(stx::os_allocator).unwrap())
                      .unwrap())
                  .unwrap();

    for (size_t i = 0; i < 20; i++) {
      timeline
          .add_task(stx::fn::rc::make_static([]() {}),
                    stx::PromiseAny{
                        stx::make_promise<void>(stx::os_allocator).unwrap()},
                    timepoint, stx::TaskId{0}, stx::NORMAL_PRIORITY)
          .unwrap();
    }

    timeline.tick(slots.span(), timepoint);
    EXPECT_EQ(slots.size(), 10);
    EXPECT_EQ(timeline.thread_slots_capture.size(), slots.size());

    EXPECT_EQ(timeline.starvation_timeline.size(), 20);
  }

  // stx::Dynamic dyn_array =
  //   stx::dyn::make(stx::os_allocator, std::array<fuck, 400>{}).unwrap();
  // dyn_array->size();
  // dyn_array->empty();

  stx::String h = stx::string::make_static("Hello boy");
  stx::String y = stx::string::make(stx::os_allocator, "Hello boy").unwrap();
  EXPECT_TRUE(h.equals("Hello boy"));
  EXPECT_FALSE(h.equals("Hello Boy"));

  EXPECT_TRUE(y.equals("Hello boy"));
  EXPECT_TRUE(h.equals(y));

  EXPECT_FALSE(h.starts_with("Hello world"));
}

#include "stx/scheduler/scheduling/await.h"
#include "stx/scheduler/scheduling/delay.h"
#include "stx/scheduler/scheduling/schedule.h"

void brr() {}
int rx() { return 0; }

int first(stx::Void) { return 0; }
int rx_loop(int64_t) { return 0; }

TEST(SchedulerTest, HH) {
  using namespace stx;
  using namespace std::chrono_literals;

  TaskScheduler scheduler{std::chrono::steady_clock::now(), stx::os_allocator};

  sched::fn(scheduler, []() { return 0; }, CRITICAL_PRIORITY, {});

  Future a = sched::fn(scheduler, rx, CRITICAL_PRIORITY, {});
  Future b =
      sched::chain(scheduler, Chain{first, rx_loop}, INTERACTIVE_PRIORITY, {});

  //
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

  Future<Option<Future<int>>> schedule = sched::deferred(
      scheduler,
      [&scheduler](Future<int> a, Future<int> b) -> Option<Future<int>> {
        if (a.fetch_status() == FutureStatus::Canceled ||
            b.fetch_status() == FutureStatus::Canceled) {
          return stx::None;
        } else {
          return Some(sched::fn(scheduler, []() -> int { return 8; },
                                INTERACTIVE_PRIORITY, {}));
        }
      },
      a.share(), b.share());

  // join stream
  // split stream
  // filter
  // map
  // reduce streams
  // forloop combine with loop combine with others???
}

TEST(Scheduler, ThreadSlots) {
  using namespace stx;
  ThreadSlot slot{stx::make_promise<void>(stx::os_allocator).unwrap()};

  ThreadSlot::Query query0 = slot.slot.query();

  EXPECT_TRUE(query0.can_push);
  EXPECT_TRUE(query0.executing_task.is_none());
  EXPECT_TRUE(query0.pending_task.is_none());

  slot.slot.push_task(ThreadSlot::Task{
      stx::fn::rc::make_static([]() { STX_LOG("1"); }), stx::TaskId{1}});

  // test that popping tasks from the queue works as expected
  EXPECT_TRUE(slot.slot.try_pop_task().is_some());

  // test for correct organization
  EXPECT_TRUE(query0.executing_task.is_none());
  EXPECT_TRUE(query0.pending_task.is_none());

  slot.slot.push_task(
      ThreadSlot::Task{stx::fn::rc::make_static([]() {}), stx::TaskId{1}});

  EXPECT_TRUE(query0.executing_task.is_none());
  EXPECT_TRUE(query0.pending_task.is_none());
}

TEST(Timeline, Sample) {
  using namespace stx;

  // Testing for work spreading amongst CPU cores

  ScheduleTimeline timeline{stx::os_allocator};

  (void)timeline.add_task(
      stx::fn::rc::make_static([]() { STX_LOG("1"); }),
      stx::PromiseAny{stx::make_promise<void>(stx::os_allocator).unwrap()},
      std::chrono::steady_clock::now(), stx::TaskId{1}, stx::NORMAL_PRIORITY);

  (void)timeline.add_task(
      stx::fn::rc::make_static([]() { STX_LOG("2"); }),
      stx::PromiseAny{stx::make_promise<void>(stx::os_allocator).unwrap()},
      std::chrono::steady_clock::now(), stx::TaskId{2}, stx::NORMAL_PRIORITY);

  (void)timeline.add_task(
      stx::fn::rc::make_static([]() { STX_LOG("3"); }),
      stx::PromiseAny{stx::make_promise<void>(stx::os_allocator).unwrap()},
      std::chrono::steady_clock::now(), stx::TaskId{3}, stx::NORMAL_PRIORITY);

  (void)timeline.add_task(
      stx::fn::rc::make_static([]() { STX_LOG("4"); }),
      stx::PromiseAny{stx::make_promise<void>(stx::os_allocator).unwrap()},
      std::chrono::steady_clock::now(), stx::TaskId{4}, stx::NORMAL_PRIORITY);

  std::array<Rc<ThreadSlot *>, 4> slot{
      stx::rc::make_inplace<ThreadSlot>(
          stx::os_allocator,
          stx::make_promise<void>(stx::os_allocator).unwrap())
          .unwrap(),
      stx::rc::make_inplace<ThreadSlot>(
          stx::os_allocator,
          stx::make_promise<void>(stx::os_allocator).unwrap())
          .unwrap(),
      stx::rc::make_inplace<ThreadSlot>(
          stx::os_allocator,
          stx::make_promise<void>(stx::os_allocator).unwrap())
          .unwrap(),
      stx::rc::make_inplace<ThreadSlot>(
          stx::os_allocator,
          stx::make_promise<void>(stx::os_allocator).unwrap())
          .unwrap()};

  timeline.tick(stx::Span{slot.data(), slot.size()},
                std::chrono::steady_clock::now());

  EXPECT_FALSE(slot[0].handle->slot.query().can_push);
  EXPECT_FALSE(slot[1].handle->slot.query().can_push);
  EXPECT_FALSE(slot[2].handle->slot.query().can_push);
  EXPECT_FALSE(slot[3].handle->slot.query().can_push);

  EXPECT_FALSE(slot[0].handle->slot.query().executing_task.is_some());
  EXPECT_FALSE(slot[1].handle->slot.query().executing_task.is_some());
  EXPECT_FALSE(slot[2].handle->slot.query().executing_task.is_some());
  EXPECT_FALSE(slot[3].handle->slot.query().executing_task.is_some());

  EXPECT_TRUE(slot[0].handle->slot.query().pending_task.is_some());
  EXPECT_TRUE(slot[1].handle->slot.query().pending_task.is_some());
  EXPECT_TRUE(slot[2].handle->slot.query().pending_task.is_some());
  EXPECT_TRUE(slot[3].handle->slot.query().pending_task.is_some());

  EXPECT_TRUE(std::is_sorted(
      timeline.starvation_timeline.iterator____begin(),
      timeline.starvation_timeline.iterator____end(),
      [](ScheduleTimeline::Task const &a, ScheduleTimeline::Task const &b) {
        return a.priority < b.priority;
      }));
}
