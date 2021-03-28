#include <fmt/core.h>

#include <memory>
#include <vector>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "ndt/context.h"
#include "ndt/exception.h"
#include "ndt/thread_pool.h"

TEST(ContextTest, MultithreadConstructionDestruction)
{
    constexpr std::size_t kOriginContextCount = 50000;
    const std::function<void()> createContexts_1 = []() {
        constexpr std::size_t kContextCount = kOriginContextCount;
        std::vector<std::unique_ptr<ndt::Context<ndt::System>>> contexts;
        contexts.reserve(kContextCount);
        for (int i = 0; i < kContextCount; ++i)
        {
            contexts.push_back(std::make_unique<ndt::Context<ndt::System>>());
        }
    };

    const std::function<void()> createContexts_2 = []() {
        constexpr std::size_t kContextCount = 2 * kOriginContextCount;
        std::vector<std::unique_ptr<ndt::Context<ndt::System>>> contexts;
        contexts.reserve(kContextCount);
        for (int i = 0; i < kContextCount; ++i)
        {
            contexts.push_back(std::make_unique<ndt::Context<ndt::System>>());
        }
    };

    const auto action = [createContexts_1 = std::move(createContexts_1),
                         createContexts_2 = std::move(createContexts_2)]() {
        ndt::thread_pool pool(ndt::thread_pool::eStopMode::kMakeOne);
        pool.push(
            [](const std::function<void()> createContexts) {
                {
                    createContexts();
                }
                {
                    createContexts();
                }
                {
                    createContexts();
                }
                {
                    createContexts();
                }
            },
            std::move(createContexts_1));
        pool.push(
            [](const std::function<void()> createContexts) {
                {
                    createContexts();
                }
                createContexts();
            },
            std::move(createContexts_2));
    };

    EXPECT_NO_THROW(action());
    ASSERT_EQ(ndt::Context<ndt::System>::instanceCount(), 0);
}