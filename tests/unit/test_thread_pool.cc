#include <gtest/gtest.h>
#include "judge/judge_manager.h"
#include <atomic>
#include <chrono>

// We test only the ThreadPool, not JudgeManager (which needs Docker)

TEST(ThreadPoolTest, ExecuteOneTask) {
    ThreadPool pool(2);
    std::atomic<int> executed{0};
    pool.enqueue([&executed]() { executed = 42; });
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    EXPECT_EQ(executed.load(), 42);
}

TEST(ThreadPoolTest, ExecuteMultipleTasks) {
    ThreadPool pool(4);
    std::atomic<int> counter{0};
    for (int i = 0; i < 100; ++i) {
        pool.enqueue([&counter]() { counter++; });
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    EXPECT_EQ(counter.load(), 100);
}

TEST(ThreadPoolTest, TasksExecuteInBackground) {
    ThreadPool pool(2);
    std::atomic<bool> started{false};
    std::atomic<bool> finished{false};

    pool.enqueue([&started, &finished]() {
        started = true;
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        finished = true;
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    EXPECT_TRUE(started.load());
    EXPECT_FALSE(finished.load());
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    EXPECT_TRUE(finished.load());
}

TEST(ThreadPoolTest, OrderPreservedPerThread) {
    ThreadPool pool(1);
    std::vector<int> order;
    for (int i = 0; i < 10; ++i) {
        pool.enqueue([&order, i]() { order.push_back(i); });
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    ASSERT_EQ(order.size(), 10);
    for (size_t i = 0; i < order.size(); ++i) {
        EXPECT_EQ(order[i], (int)i);
    }
}
