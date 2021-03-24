#ifndef ndt_executor_select_impl_h
#define ndt_executor_select_impl_h

#include "../../common.h"
#include "../../executor_select_base.h"

namespace ndt
{
class ExecutorSelect : public ExecutorSelectBase<ExecutorSelect>
{
    friend class ExecutorSelectBase<ExecutorSelect>;

   public:
    ~ExecutorSelect();
    ExecutorSelect() noexcept;

   private:
    inline int selectImpl() noexcept
    {
        return System::select(0, &readfds_, &writefds_, &exceptfds_,
                              timeoutPtr_);
    }
    inline sock_t nativeHandleImpl(std::size_t aIndex) const noexcept
    {
        return fdInfos_[aIndex]->nativeHandle();
    }
    void addSocketImpl(SocketBase* aSocket) noexcept;
    void delNativeHandleImpl(const sock_t aHandle) noexcept;
    void delHandlerImpl(HandlerSelectBase* aHandler) noexcept;

    std::size_t infoIndex(const sock_t aHandle) const noexcept;
    std::size_t firstVacantIndex() const noexcept;

    std::size_t numfds_ = 0;
};
}  // namespace ndt

#endif /* ndt_executor_select_impl_h */