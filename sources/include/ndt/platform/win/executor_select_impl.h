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
        return SysWrapperT::select(0, &(BaseT::readfds_), &(BaseT::writefds_),
                                   &(BaseT::exceptfds_), BaseT::timeoutPtr_);
    }
    inline sock_t nativeHandleImpl(std::size_t aIndex) const noexcept
    {
        return BaseT::fdInfos_[aIndex]->nativeHandle();
    }
    void addSocketImpl(SocketBase<SysWrapperT>* aSocket) noexcept;
    void delNativeHandleImpl(const sock_t aHandle) noexcept;
    void delHandlerImpl(HandlerSelectBase<SysWrapperT>* aHandler) noexcept;

    std::size_t infoIndex(const sock_t aHandle) const noexcept;
    std::size_t firstVacantIndex() const noexcept;

    std::size_t numfds_ = 0;
};

template <typename SysWrapperT>
ExecutorSelect<SysWrapperT>::~ExecutorSelect() = default;

template <typename SysWrapperT>
ExecutorSelect<SysWrapperT>::ExecutorSelect() noexcept = default;

template <typename SysWrapperT>
std::size_t ExecutorSelect<SysWrapperT>::infoIndex(
    const sock_t aHandle) const noexcept
{
    std::size_t index = 0;
    for (std::size_t i = 0; i < BaseT::kMaxFDCount; ++i)
    {
        if (BaseT::fdInfos_[i] &&
            (BaseT::fdInfos_[i]->nativeHandle() == aHandle))
        {
            index = i;
            break;
        }
    }

    return index;
}

template <typename SysWrapperT>
std::size_t ExecutorSelect<SysWrapperT>::firstVacantIndex() const noexcept
{
    std::size_t index = 0;
    for (std::size_t i = 0; i < BaseT::kMaxFDCount; ++i)
    {
        if (BaseT::fdInfos_[i] == nullptr)
        {
            index = i;
            break;
        }
    }
    return index;
}

template <typename SysWrapperT>
void ExecutorSelect<SysWrapperT>::addSocketImpl(
    SocketBase<SysWrapperT>* aSocket) noexcept
{
    const auto index = firstVacantIndex();
    BaseT::fdInfos_[index] = aSocket;
    ++numfds_;
}

template <typename SysWrapperT>
void ExecutorSelect<SysWrapperT>::delNativeHandleImpl(
    const sock_t aHandle) noexcept
{
    const auto index = infoIndex(aHandle);
    --numfds_;
    if (index != numfds_)
    {
        BaseT::fdInfos_[index] = BaseT::fdInfos_[numfds_];
        BaseT::fdInfos_[numfds_] = nullptr;
    }
    else
    {
        BaseT::fdInfos_[index] = nullptr;
    }
}

template <typename SysWrapperT>
void ExecutorSelect<SysWrapperT>::delHandlerImpl(
    HandlerSelectBase<SysWrapperT>* aHandler) noexcept
{
    for (std::size_t i = 0; i < numfds_;)
    {
        if (BaseT::fdInfos_[i] && (BaseT::fdInfos_[i]->handler() == aHandler))
        {
            const auto handleToDelete = BaseT::fdInfos_[i]->nativeHandle();
            BaseT::setFlag(handleToDelete, false, &(BaseT::masterReadFDs_));
            BaseT::setFlag(handleToDelete, false, &(BaseT::masterWriteFDs_));
            BaseT::setFlag(handleToDelete, false, &(BaseT::masterExceptFDs_));
            if (i + 1 < numfds_)
            {
                BaseT::fdInfos_[i]->handler_ = nullptr;
                BaseT::fdInfos_[i] = BaseT::fdInfos_[numfds_ - 1];
                BaseT::fdInfos_[numfds_ - 1] = nullptr;
            }
            else
            {
                BaseT::fdInfos_[i]->handler_ = nullptr;
                BaseT::fdInfos_[i] = nullptr;
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

#endif /* ndt_executor_select_impl_h */