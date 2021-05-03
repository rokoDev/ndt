#ifndef ndt_executor_select_impl_h
#define ndt_executor_select_impl_h

#include "../../common.h"
#include "../../executor_select_base.h"

namespace ndt
{
template <typename SysWrapperT>
class ExecutorSelect
    : public ExecutorSelectBase<ExecutorSelect<SysWrapperT>, SysWrapperT>
{
    friend class ExecutorSelectBase<ExecutorSelect<SysWrapperT>, SysWrapperT>;
    using BaseT = ExecutorSelectBase<ExecutorSelect<SysWrapperT>, SysWrapperT>;

   public:
    ~ExecutorSelect();
    ExecutorSelect() noexcept;

   private:
    inline int selectImpl() noexcept
    {
        return SysWrapperT::select(numfds_, &(BaseT::readfds_),
                                   &(BaseT::writefds_), &(BaseT::exceptfds_),
                                   BaseT::timeoutPtr_);
    }
    inline sock_t nativeHandleImpl(std::size_t aIndex) const noexcept
    {
        return static_cast<sock_t>(aIndex);
    }

    void addSocketImpl(SocketBase<SysWrapperT>* aSocket) noexcept;
    void delNativeHandleImpl(const sock_t aHandle) noexcept;
    void delHandlerImpl(HandlerSelectBase<SysWrapperT>* aHandler) noexcept;

    static sock_t numDescrs(const sock_t aCurrentMax, fd_set const* aFDs);
    void updateNumfds() noexcept;

    int numfds_ = 0;
};

template <typename SysWrapperT>
ExecutorSelect<SysWrapperT>::~ExecutorSelect() = default;

template <typename SysWrapperT>
ExecutorSelect<SysWrapperT>::ExecutorSelect() noexcept = default;

template <typename SysWrapperT>
sock_t ExecutorSelect<SysWrapperT>::numDescrs(const sock_t aCurrentMax,
                                              const fd_set* aFDs)
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

template <typename SysWrapperT>
void ExecutorSelect<SysWrapperT>::updateNumfds() noexcept
{
    const auto numReadFDs = numDescrs(numfds_, &(BaseT::masterReadFDs_));
    const auto numWriteFDs = numDescrs(numfds_, &(BaseT::masterWriteFDs_));
    const auto numExceptFDs = numDescrs(numfds_, &(BaseT::masterExceptFDs_));
    numfds_ = std::max(std::max(numReadFDs, numWriteFDs), numExceptFDs);
}

template <typename SysWrapperT>
void ExecutorSelect<SysWrapperT>::addSocketImpl(
    SocketBase<SysWrapperT>* aSocket) noexcept
{
    const auto kHandle = aSocket->nativeHandle();
    if (kHandle >= numfds_)
    {
        numfds_ = kHandle + 1;
    }
    BaseT::fdInfos_[static_cast<std::size_t>(kHandle)] = aSocket;
}

template <typename SysWrapperT>
void ExecutorSelect<SysWrapperT>::delNativeHandleImpl(
    const sock_t aHandle) noexcept
{
    if (aHandle == numfds_ - 1)
    {
        updateNumfds();
    }

    BaseT::fdInfos_[static_cast<std::size_t>(aHandle)] = nullptr;
}

template <typename SysWrapperT>
void ExecutorSelect<SysWrapperT>::delHandlerImpl(
    HandlerSelectBase<SysWrapperT>* aHandler) noexcept
{
    const std::size_t kNumFDs = static_cast<std::size_t>(numfds_);
    int maxDeletedHandle = -1;
    for (std::size_t i = 0; i < kNumFDs; ++i)
    {
        if (BaseT::fdInfos_[i] && (BaseT::fdInfos_[i]->handler() == aHandler))
        {
            maxDeletedHandle = static_cast<int>(i);
            BaseT::setFlag(maxDeletedHandle, false, &(BaseT::masterReadFDs_));
            BaseT::setFlag(maxDeletedHandle, false, &(BaseT::masterWriteFDs_));
            BaseT::setFlag(maxDeletedHandle, false, &(BaseT::masterExceptFDs_));
            BaseT::fdInfos_[i]->handler_ = nullptr;
            BaseT::fdInfos_[i] = nullptr;
        }
    }
    if (maxDeletedHandle == numfds_ - 1)
    {
        updateNumfds();
    }
}
}  // namespace ndt

#endif /* ndt_executor_select_impl_h */