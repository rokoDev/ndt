#include "ndt/platform/nix/executor_select_impl.h"

namespace ndt
{
ExecutorSelect::~ExecutorSelect() = default;

ExecutorSelect::ExecutorSelect() noexcept = default;

sock_t ExecutorSelect::numDescrs(const sock_t aCurrentMax, const fd_set *aFDs)
{
    sock_t maxDescr = 0;
    for (int i = aCurrentMax - 2; i >= 0; --i)
    {
        if (FD_ISSET(i, aFDs))
        {
            maxDescr = i + 1;
            break;
        }
    }
    return maxDescr;
}

void ExecutorSelect::updateNumfds() noexcept
{
    const auto numReadFDs = numDescrs(numfds_, &masterReadFDs_);
    const auto numWriteFDs = numDescrs(numfds_, &masterWriteFDs_);
    const auto numExceptFDs = numDescrs(numfds_, &masterExceptFDs_);
    numfds_ = std::max(std::max(numReadFDs, numWriteFDs), numExceptFDs);
}

void ExecutorSelect::addSocketImpl(SocketBase *aSocket) noexcept
{
    const auto kHandle = aSocket->nativeHandle();
    if (kHandle >= numfds_)
    {
        numfds_ = kHandle + 1;
    }
    fdInfos_[static_cast<std::size_t>(kHandle)] = aSocket;
}

void ExecutorSelect::delNativeHandleImpl(const sock_t aHandle) noexcept
{
    if (aHandle == numfds_ - 1)
    {
        updateNumfds();
    }

    fdInfos_[static_cast<std::size_t>(aHandle)] = nullptr;
}

void ExecutorSelect::delHandlerImpl(HandlerSelectBase *aHandler) noexcept
{
    const std::size_t kNumFDs = static_cast<std::size_t>(numfds_);
    int maxDeletedHandle = -1;
    for (std::size_t i = 0; i < kNumFDs; ++i)
    {
        if (fdInfos_[i] && (fdInfos_[i]->handler() == aHandler))
        {
            maxDeletedHandle = static_cast<int>(i);
            setFlag(maxDeletedHandle, false, &masterReadFDs_);
            setFlag(maxDeletedHandle, false, &masterWriteFDs_);
            setFlag(maxDeletedHandle, false, &masterExceptFDs_);
            fdInfos_[i]->handler_ = nullptr;
            fdInfos_[i] = nullptr;
        }
    }
    if (maxDeletedHandle == numfds_ - 1)
    {
        updateNumfds();
    }
}
}  // namespace ndt