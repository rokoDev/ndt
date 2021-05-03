#ifndef ndt_executor_select_base_h
#define ndt_executor_select_base_h

#include <array>
#include <functional>
#include <system_error>

#include "common.h"
#include "event_handler_select.h"
#include "socket.h"

#ifndef FD_COPY
#define FD_COPY(src, dest) (*(dest) = *(src))
#endif

namespace ndt
{
template <typename SysWrapperT>
class SocketBase;

template <typename ImplT, typename SysWrapperT>
class ExecutorSelectBase
{
    friend class ExecutorSelect<SysWrapperT>;

   public:
    void operator()();

    void addSocket(SocketBase<SysWrapperT> *aSocket);
    void delHandler(HandlerSelectBase<SysWrapperT> *aHandler);
    void delSocket(SocketBase<SysWrapperT> const *aSocket);

    inline timeval const *timeout() const noexcept;
    void setTimeout(const timeval aTimeout) noexcept;
    void setTimeoutInfinite() noexcept;

    void setTimeoutHandler(
        const std::function<void()> aTimeoutHandler) noexcept;
    void setErrorHandler(
        const std::function<void(std::error_code aEc)> aErrorHandler) noexcept;

   protected:
    ~ExecutorSelectBase();
    ExecutorSelectBase() noexcept;
    static void setFlag(const sock_t aSocketHandle, const bool aState,
                        fd_set *aFDs) noexcept;

    inline ImplT &impl() noexcept;
    bool isTracked(const sock_t aHandle) const noexcept;
    void initSelectArgs() noexcept;
    void iterateResult(const int aResult);

    static constexpr int kMaxFDCount = FD_SETSIZE;
    fd_set masterReadFDs_;
    fd_set masterWriteFDs_;
    fd_set masterExceptFDs_;
    std::array<SocketBase<SysWrapperT> *, kMaxFDCount> fdInfos_{};
    fd_set readfds_;
    fd_set writefds_;
    fd_set exceptfds_;
    timeval *timeoutPtr_ = nullptr;
    timeval timeout_ = {0, 0};
    timeval masterTimeout_ = {0, 0};
    bool infiniteTimeout_ = false;
    std::function<void()> timeoutHandler_ = []() {};
    std::function<void(std::error_code aEc)> errorHandler_ =
        [](std::error_code) {};
};

template <typename ImplT, typename SysWrapperT>
ExecutorSelectBase<ImplT, SysWrapperT>::~ExecutorSelectBase() = default;

template <typename ImplT, typename SysWrapperT>
ExecutorSelectBase<ImplT, SysWrapperT>::ExecutorSelectBase() noexcept
{
    FD_ZERO(&masterReadFDs_);
    FD_ZERO(&masterWriteFDs_);
    FD_ZERO(&masterExceptFDs_);
}

template <typename ImplT, typename SysWrapperT>
ImplT &ExecutorSelectBase<ImplT, SysWrapperT>::impl() noexcept
{
    return static_cast<ImplT &>(*this);
}

template <typename ImplT, typename SysWrapperT>
void ExecutorSelectBase<ImplT, SysWrapperT>::operator()()
{
    initSelectArgs();

    const int result = impl().selectImpl();
    if (result != kSocketError)
    {
        if (result)
        {
            // handle events
            iterateResult(result);
        }
        else
        {
            // handle timeout
            timeoutHandler_();
        }
    }
    else
    {
        // handle error
        errorHandler_(std::error_code(SysWrapperT::lastErrorCode(),
                                      std::system_category()));
    }
}

template <typename ImplT, typename SysWrapperT>
void ExecutorSelectBase<ImplT, SysWrapperT>::addSocket(
    SocketBase<SysWrapperT> *aSocket)
{
    if ((aSocket == nullptr) || !aSocket->isOpen() ||
        (aSocket->handler() == nullptr))
    {
        return;
    }

    const auto socketHandle = aSocket->nativeHandle();
    const auto eventMask = aSocket->handler()->eventMask_;

    setFlag(socketHandle,
            HandlerSelectBase<SysWrapperT>::eTrakingEvents::kRead & eventMask,
            &masterReadFDs_);
    setFlag(socketHandle,
            HandlerSelectBase<SysWrapperT>::eTrakingEvents::kWrite & eventMask,
            &masterWriteFDs_);
    setFlag(
        socketHandle,
        HandlerSelectBase<SysWrapperT>::eTrakingEvents::kExceptCond & eventMask,
        &masterExceptFDs_);

    impl().addSocketImpl(aSocket);
}

template <typename ImplT, typename SysWrapperT>
void ExecutorSelectBase<ImplT, SysWrapperT>::delHandler(
    HandlerSelectBase<SysWrapperT> *aHandler)
{
    if (!aHandler)
    {
        return;
    }
    impl().delHandlerImpl(aHandler);
}

template <typename ImplT, typename SysWrapperT>
void ExecutorSelectBase<ImplT, SysWrapperT>::delSocket(
    SocketBase<SysWrapperT> const *aSocket)
{
    if ((aSocket == nullptr) || !aSocket->isOpen() ||
        (aSocket->handler() == nullptr))
    {
        return;
    }

    const auto socketHandle = aSocket->nativeHandle();
    setFlag(socketHandle, false, &masterReadFDs_);
    setFlag(socketHandle, false, &masterWriteFDs_);
    setFlag(socketHandle, false, &masterExceptFDs_);
    impl().delNativeHandleImpl(socketHandle);
}

template <typename ImplT, typename SysWrapperT>
timeval const *ExecutorSelectBase<ImplT, SysWrapperT>::timeout() const noexcept
{
    return infiniteTimeout_ ? nullptr : &masterTimeout_;
}

template <typename ImplT, typename SysWrapperT>
void ExecutorSelectBase<ImplT, SysWrapperT>::setTimeout(
    const timeval aTimeout) noexcept
{
    masterTimeout_ = aTimeout;
    infiniteTimeout_ = false;
}

template <typename ImplT, typename SysWrapperT>
void ExecutorSelectBase<ImplT, SysWrapperT>::setTimeoutInfinite() noexcept
{
    infiniteTimeout_ = true;
}

template <typename ImplT, typename SysWrapperT>
void ExecutorSelectBase<ImplT, SysWrapperT>::setTimeoutHandler(
    const std::function<void()> aTimeoutHandler) noexcept
{
    if (aTimeoutHandler)
    {
        timeoutHandler_ = aTimeoutHandler;
    }
    else
    {
        timeoutHandler_ = []() {};
    }
}

template <typename ImplT, typename SysWrapperT>
void ExecutorSelectBase<ImplT, SysWrapperT>::setErrorHandler(
    const std::function<void(std::error_code aEc)> aErrorHandler) noexcept
{
    if (aErrorHandler)
    {
        errorHandler_ = aErrorHandler;
    }
    else
    {
        errorHandler_ = [](std::error_code) {};
    }
}

template <typename ImplT, typename SysWrapperT>
void ExecutorSelectBase<ImplT, SysWrapperT>::setFlag(const sock_t aSocketHandle,
                                                     const bool aState,
                                                     fd_set *aFDs) noexcept
{
    if (aState)
    {
        FD_SET(aSocketHandle, aFDs);
    }
    else
    {
        FD_CLR(aSocketHandle, aFDs);
    }
}

template <typename ImplT, typename SysWrapperT>
bool ExecutorSelectBase<ImplT, SysWrapperT>::isTracked(
    const sock_t aHandle) const noexcept
{
    return FD_ISSET(aHandle, &masterReadFDs_) &&
           FD_ISSET(aHandle, &masterWriteFDs_) &&
           FD_ISSET(aHandle, &masterExceptFDs_);
}

template <typename ImplT, typename SysWrapperT>
void ExecutorSelectBase<ImplT, SysWrapperT>::initSelectArgs() noexcept
{
    FD_COPY(&masterReadFDs_, &readfds_);
    FD_COPY(&masterWriteFDs_, &writefds_);
    FD_COPY(&masterExceptFDs_, &exceptfds_);

    if (!infiniteTimeout_)
    {
        timeout_ = masterTimeout_;
        timeoutPtr_ = &timeout_;
    }
    else
    {
        timeoutPtr_ = nullptr;
    }
}

template <typename ImplT, typename SysWrapperT>
void ExecutorSelectBase<ImplT, SysWrapperT>::iterateResult(const int aResult)
{
    std::size_t processed = 0;
    const std::size_t readyFDCount = static_cast<std::size_t>(aResult);
    for (std::size_t i = 0, processedCount = 0; processedCount < readyFDCount;
         ++i)
    {
        processed = 0;
        if (FD_ISSET(impl().nativeHandleImpl(i), &readfds_))
        {
            // data can be read from socket without blocking
            HandlerSelectBase<SysWrapperT> *handler = fdInfos_[i]->handler_;
            handler->inDataHandler_(*fdInfos_[i], handler);
            processed = 1;
        }
        if (FD_ISSET(impl().nativeHandleImpl(i), &writefds_))
        {
            // data can be written to socket without blocking
            HandlerSelectBase<SysWrapperT> *handler = fdInfos_[i]->handler_;
            handler->outDataHandler_(*fdInfos_[i], handler);
            processed = 1;
        }
        if (FD_ISSET(impl().nativeHandleImpl(i), &exceptfds_))
        {
            // exception conditions occured in this socket can be handled
            // without blocking
            HandlerSelectBase<SysWrapperT> *handler = fdInfos_[i]->handler_;
            handler->exceptCondHandler_(*fdInfos_[i], handler);
            processed = 1;
        }
        processedCount += processed;
    }
}
}  // namespace ndt

#endif /* ndt_executor_select_base_h */