#ifndef ndt_thread_pool_h
#define ndt_thread_pool_h

#include <condition_variable>
#include <deque>
#include <functional>
#include <future>
#include <limits>
#include <mutex>
#include <optional>
#include <thread>
#include <vector>

namespace ndt
{
class thread_pool final
{
   public:
    enum class eStopMode : uint8_t
    {
        kMakeOne = 0,  // thread pool has received stop command and every thread
                       // will exetuce one job before exit(if any)
        kFast  // thread pool has received stop command and every thread will
               // finish as soon as possible without regard to remaining jobs
    };
    ~thread_pool();
    thread_pool();
    thread_pool(const eStopMode aStopMode);
    thread_pool(const std::size_t aMaxThreadCount);
    thread_pool(const std::size_t aMaxThreadCount,
                const std::size_t aMaxQueueSize);
    thread_pool(const std::size_t aMaxThreadCount,
                const std::size_t aMaxQueueSize, const eStopMode aStopMode);

    thread_pool(thread_pool &&aOther) = delete;
    thread_pool &operator=(thread_pool &&aOther) = delete;

    thread_pool(const thread_pool &) = delete;
    thread_pool &operator=(const thread_pool &) = delete;

    void start(const std::size_t aMaxThreadCount);
    void stop();
    void stopMode(const eStopMode aStopMode);

    std::size_t maxThreadCount() const noexcept;
    std::size_t maxQueueSize() const noexcept;

    template <typename Func, typename... Args>
    auto push(Func &&f, Args &&... args) -> std::enable_if_t<
        std::is_invocable_v<Func, Args &&...>,
        std::optional<std::future<typename std::result_of_t<Func(Args...)>>>>;

   private:
    void executeJob(std::unique_lock<std::mutex> aULock);

    std::mutex mMutex;
    std::condition_variable mCondition;
    eStopMode stopMode_ = eStopMode::kFast;
    std::size_t mMaxQueueSize = std::numeric_limits<std::size_t>::max();
    bool active_ = true;
    std::deque<std::function<void()>> mTaskQueue;
    std::vector<std::thread> mWorkers;
};

template <typename Func, typename... Args>
auto thread_pool::push(Func &&f, Args &&... args) -> std::enable_if_t<
    std::is_invocable_v<Func, Args &&...>,
    std::optional<std::future<typename std::result_of_t<Func(Args...)>>>>
{
    using result_type = typename std::result_of_t<Func(Args...)>;
    std::future<result_type> retVal;
    {
        std::unique_lock<std::mutex> uLock(mMutex);
        if (mTaskQueue.size() < mMaxQueueSize)
        {
            auto shared_task =
                std::make_shared<std::packaged_task<result_type()>>(std::bind(
                    std::forward<Func>(f), std::forward<Args>(args)...));
            mTaskQueue.emplace_back([shared_task]() { (*shared_task)(); });
            retVal = shared_task->get_future();
            uLock.unlock();
            mCondition.notify_one();
            return retVal;
        }
    }
    return std::nullopt;
}
}  // namespace ndt

#endif /* ndt_thread_pool_h */