#include "ndt/thread_pool.h"

namespace ndt
{
thread_pool::~thread_pool() { stopAndMakeOne(); }

thread_pool::thread_pool(const std::size_t aMaxThreadCount,
                         const std::size_t aMaxQueueSize)
    : mMaxQueueSize(aMaxQueueSize)
{
    start(aMaxThreadCount);
}

thread_pool::thread_pool(const std::size_t aMaxThreadCount)
    : thread_pool(aMaxThreadCount, std::numeric_limits<std::size_t>::max())
{
}

thread_pool::thread_pool()
    : thread_pool(std::thread::hardware_concurrency(),
                  std::numeric_limits<std::size_t>::max())
{
}

void thread_pool::start(const std::size_t aMaxThreadCount)
{
    mWorkers.reserve(aMaxThreadCount);
    for (unsigned i = 0; i < aMaxThreadCount; ++i)
    {
        mWorkers.emplace_back([this]() {
            while (true)
            {
                std::unique_lock<std::mutex> uLock(mMutex);
                mCondition.wait(uLock, [this]() {
                    return !mTaskQueue.empty() ||
                           (stopMode_ != eStopMode::kBusy);
                });
                if (stopMode_ != eStopMode::kBusy)
                {
                    if ((stopMode_ == eStopMode::kMakeOne) &&
                        !mTaskQueue.empty())
                    {
                        executeJob(std::move(uLock));
                    }
                    break;
                }
                executeJob(std::move(uLock));
            }
        });
    }
}

void thread_pool::stop() { stopMode(eStopMode::kFast); }

void thread_pool::stopAndMakeOne() { stopMode(eStopMode::kMakeOne); }

void thread_pool::stopMode(const eStopMode aStopMode)
{
    {
        std::unique_lock<std::mutex> uLock(mMutex);
        stopMode_ = aStopMode;
    }
    mCondition.notify_all();
    for (auto& worker: mWorkers)
    {
        if (worker.joinable())
        {
            worker.join();
        }
    }
    mWorkers.clear();
}

void thread_pool::executeJob(std::unique_lock<std::mutex> aULock)
{
    const auto curTask = std::move(mTaskQueue.front());
    mTaskQueue.pop_front();
    aULock.unlock();
    curTask();
}

std::size_t thread_pool::maxThreadCount() const noexcept
{
    return mWorkers.size();
}

std::size_t thread_pool::maxQueueSize() const noexcept { return mMaxQueueSize; }
}  // namespace ndt