
#include "stx/scheduler.h"

#include "gtest/gtest.h"
#include "stx/scheduler/scheduling/deferred.h"
#include "stx/scheduler/scheduling/delay.h"
#include "stx/scheduler/scheduling/schedule.h"


// TODO(lamarrr): implement or totally remove logging
#define STX_LOG(...)

TEST(SchedulerTest, Main) {
  using namespace stx;

  TaskScheduler scheduler{std::chrono::steady_clock::now(), stx::os_allocator};

  scheduler.tick(std::chrono::nanoseconds{1});

  sched::chain(scheduler,
               stx::Chain{[](stx::Void) {
                            STX_LOG("first");
                            return 2;
                          },
                          [](int a) {
                            STX_LOG("second");
                            return stx::Void{};
                          }},
               stx::NORMAL_PRIORITY, {});
  sched::fn(scheduler, []() { STX_LOG("hello"); }, stx::NORMAL_PRIORITY, {});
  sched::fn(scheduler, []() { STX_LOG("world!"); }, stx::NORMAL_PRIORITY, {});

  scheduler.tick(std::chrono::nanoseconds{1});
}
