#include "ndt/platform/win/executor_select_impl.h"

namespace ndt
{
ExecutorSelect::~ExecutorSelect() = default;

ExecutorSelect::ExecutorSelect() noexcept = default;

std::size_t ExecutorSelect::infoIndex(const sock_t aHandle) const noexcept
{
    std::size_t index = 0;
    for (std::size_t i = 0; i < kMaxFDCount; ++i)
    {
        if (fdInfos_[i] && (fdInfos_[i]->nativeHandle() == aHandle))
        {
            index = i;
            break;
        }
    }

    return index;
}

std::size_t ExecutorSelect::firstVacantIndex() const noexcept
{
    std::size_t index = 0;
    for (std::size_t i = 0; i < kMaxFDCount; ++i)
    {
        if (fdInfos_[i] == nullptr)
        {
            index = i;
            break;
        }
    }
    return index;
}

void ExecutorSelect::addSocketImpl(SocketBase* aSocket) noexcept
{
    const auto index = firstVacantIndex();
    fdInfos_[index] = aSocket;
    ++numfds_;
}

void ExecutorSelect::delNativeHandleImpl(const sock_t aHandle) noexcept
{
    const auto index = infoIndex(aHandle);
    --numfds_;
    if (index != numfds_)
    {
        fdInfos_[index] = fdInfos_[numfds_];
        fdInfos_[numfds_] = nullptr;
    }
    else
    {
        fdInfos_[index] = nullptr;
    }
}

void ExecutorSelect::delHandlerImpl(HandlerSelectBase* aHandler) noexcept
{
    for (std::size_t i = 0; i < numfds_;)
    {
        if (fdInfos_[i] && (fdInfos_[i]->handler() == aHandler))
        {
            const auto handleToDelete = fdInfos_[i]->nativeHandle();
            setFlag(handleToDelete, false, &masterReadFDs_);
            setFlag(handleToDelete, false, &masterWriteFDs_);
            setFlag(handleToDelete, false, &masterExceptFDs_);
            if (i + 1 < numfds_)
            {
                fdInfos_[i]->handler_ = nullptr;
                fdInfos_[i] = fdInfos_[numfds_ - 1];
                fdInfos_[numfds_ - 1] = nullptr;
            }
            else
            {
                fdInfos_[i]->handler_ = nullptr;
                fdInfos_[i] = nullptr;
            }
            --numfds_;
        }
        else
        {
            ++i;
        }
    }
}
}  // namespace ndt