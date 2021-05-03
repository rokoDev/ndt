#include <fmt/core.h>
#include <fmt/ostream.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <array>
#include <chrono>
#include <random>
#include <set>
#include <string>

#include "ndt/address.h"
#include "ndt/exception.h"
#include "ndt/thread_pool.h"

void check(ndt::thread_pool &pool, const std::size_t taskCount,
           const bool isQueueOverflowAllowed)
{
    std::vector<std::optional<std::future<int>>> results;
    results.reserve(taskCount);
    for (int i = 0; i < taskCount; ++i)
    {
        results.emplace_back(pool.push([i]() -> int {
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_int_distribution<> dist(1000000, 10000000);

            uint32_t k = 3;
            const auto kTargetVal = dist(gen);
            for (int j = 0; j < kTargetVal; ++j)
            {
                k = (j / k) + 2;
            }
            k = 0;
            return i * 2;
        }));
    }

    std::set<int> rejected;
    std::set<int> readyValues;
    for (int i = 0; i < taskCount;)
    {
        if (results[i].has_value())
        {
            if (results[i].value().valid() &&
                (results[i].value().wait_for(std::chrono::seconds(0)) ==
                 std::future_status::ready))
            {
                readyValues.insert(i);
            }
        }
        else
        {
            ASSERT_EQ(isQueueOverflowAllowed, true);
            rejected.insert(i);
        }
        ++i;
        if ((taskCount == i) &&
            (readyValues.size() + rejected.size() != taskCount))
        {
            i = 0;
        }
    }
    fmt::print("[          ] {} processed\n", readyValues.size());
    fmt::print("[          ] {} rejected\n", rejected.size());
}

TEST(ThreadPoolTests, DefaultConstructor)
{
    ndt::thread_pool pool;
    ASSERT_EQ(pool.maxThreadCount(), std::thread::hardware_concurrency());
    ASSERT_EQ(pool.maxQueueSize(), std::numeric_limits<std::size_t>::max());
    check(pool, 5, false);
}

TEST(ThreadPoolTests, ConstructorWithThreadCount)
{
    constexpr std::size_t kMaxThreads = 6;
    ndt::thread_pool pool(kMaxThreads);
    ASSERT_EQ(pool.maxThreadCount(), kMaxThreads);
    ASSERT_EQ(pool.maxQueueSize(), std::numeric_limits<std::size_t>::max());
    check(pool, 10, true);
}

TEST(ThreadPoolTests, ConstructorWithThreadCountAndQueueSize)
{
    constexpr std::size_t kMaxThreads = 6;
    constexpr std::size_t kMaxQueueSize = 5;
    ndt::thread_pool pool(kMaxThreads, kMaxQueueSize);
    ASSERT_EQ(pool.maxThreadCount(), kMaxThreads);
    ASSERT_EQ(pool.maxQueueSize(), kMaxQueueSize);
    check(pool, 10, true);
}

TEST(ThreadPoolTests, ConstructorWithHugeThreadCountAndQueueSize)
{
    const std::size_t kMaxThreads = std::thread::hardware_concurrency() + 10;
    constexpr std::size_t kMaxQueueSize = 5;
    ndt::thread_pool pool(kMaxThreads, kMaxQueueSize);
    ASSERT_EQ(pool.maxThreadCount(), kMaxThreads);
    ASSERT_EQ(pool.maxQueueSize(), kMaxQueueSize);
    check(pool, 25, true);
}

TEST(ThreadPoolTests, ExceptionTest)
{
    EXPECT_THROW(
        {
            try
            {
                ndt::thread_pool pool;
                auto result = pool.push([]() {
                    std::error_code ec(
                        ndt::eAddressErrorCode::kInvalidAddressFamily);
                    ndt::Error ndtError(ec);
                    throw ndtError;
                });
                result.value().get();
            }
            catch (const ndt::Error &re)
            {
                EXPECT_THAT(re.what(), testing::StartsWith(
                                           ndt::kInvalidAddressFamilyDescr));
                throw;
            }
        },
        ndt::Error);
}