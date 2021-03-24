#include "ndt/socket.h"

#include "ndt/context.h"
#include "ndt/event_handler_select.h"
#include "ndt/executor_select.h"
namespace ndt
{
SocketBase::~SocketBase()
{
    assert(!isOpen_ && "Error: socket must be closed before destruction");
}

SocketBase::SocketBase(Context &aContext) noexcept
    : socketHandle_(kInvalidSocket)
    , isOpen_(false)
    , isNonBlocking_(false)
    , context_(aContext)
    , handler_(nullptr)
{
}

SocketBase::SocketBase(SocketBase &&aOther) noexcept
    : socketHandle_(std::exchange(aOther.socketHandle_, kInvalidSocket))
    , isOpen_(std::exchange(aOther.isOpen_, false))
    , isNonBlocking_(std::exchange(aOther.isNonBlocking_, false))
    , context_(aOther.context_)
    , handler_(nullptr)
{
    handler(aOther.handler_);
}

SocketBase &SocketBase::operator=(SocketBase &&aOther) noexcept
{
    assert(!isOpen_ &&
           "Error: move assignment to opened socket is not allowed");
    std::swap(aOther.socketHandle_, socketHandle_);
    std::swap(aOther.isOpen_, isOpen_);
    std::swap(aOther.isNonBlocking_, isNonBlocking_);
    context_ = aOther.context_;
    handler_ = aOther.handler_;
    return *this;
}

sock_t SocketBase::nativeHandle() const noexcept { return socketHandle_; }

bool SocketBase::nonBlocking() const noexcept { return isNonBlocking_; }

bool SocketBase::isOpen() const noexcept { return isOpen_; }

HandlerSelectBase *SocketBase::handler() const noexcept { return handler_; }

void SocketBase::handler(HandlerSelectBase *aHandler)
{
    if (aHandler == handler_)
    {
        return;
    }
    handler_ = aHandler;
    if (!isOpen())
    {
        return;
    }
    if (aHandler != nullptr)
    {
        context_.get().executor().addSocket(this);
    }
    else
    {
        context_.get().executor().delSocket(this);
    }
}

void SocketBase::setIsSubscribed(bool aIsSubscribed)
{
    if (handler_)
    {
        if (aIsSubscribed)
        {
            context_.get().executor().addSocket(this);
        }
        else
        {
            context_.get().executor().delSocket(this);
        }
    }
}
}  // namespace ndt