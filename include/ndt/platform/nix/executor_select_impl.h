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
        return System::select(numfds_, &readfds_, &writefds_, &exceptfds_,
                              timeoutPtr_);
    }
    inline sock_t nativeHandleImpl(std::size_t aIndex) const noexcept
    {
        return static_cast<sock_t>(aIndex);
    }
    void addSocketImpl(SocketBase* aSocket) noexcept;
    void delNativeHandleImpl(const sock_t aHandle) noexcept;
    void delHandlerImpl(HandlerSelectBase* aHandler) noexcept;

    static sock_t numDescrs(const sock_t aCurrentMax, fd_set const* aFDs);
    void updateNumfds() noexcept;

    int numfds_ = 0;
};
}  // namespace ndt

#endif /* ndt_executor_select_impl_h */