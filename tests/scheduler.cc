
#include "stx/scheduler.h"

#include <iostream>

#include "stx/scheduler/scheduling/delay.h"
#include "stx/scheduler/scheduling/schedule.h"
#include "gtest/gtest.h"

#define STX_LOG(...) std::cout << __VA_ARGS__ << std::endl

TEST(SchedulerTest, Main)
{
  using namespace stx;

  TaskScheduler scheduler{
      stx::os_allocator,
      std::chrono::steady_clock::now(),
  };

  scheduler.tick(std::chrono::nanoseconds{1});
  sched::chain(scheduler,
               stx::Chain{[](stx::Void) {
                            STX_LOG("first");
                            return 2;
                          },
                          [](int a) {
                            (void) a;
                            STX_LOG("second");
                            return stx::Void{};
                          }},
               stx::NORMAL_PRIORITY, {});
  sched::fn(scheduler, []() { STX_LOG("hello"); }, stx::NORMAL_PRIORITY, {});
  sched::fn(scheduler, []() { STX_LOG("world!"); }, stx::NORMAL_PRIORITY, {});

  EXPECT_EQ(scheduler.entries.size(), 3);
  scheduler.tick(std::chrono::nanoseconds{1});
  EXPECT_EQ(scheduler.entries.size(), 0);
}
