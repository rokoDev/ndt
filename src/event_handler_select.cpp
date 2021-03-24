#include "ndt/event_handler_select.h"

#include "ndt/context.h"
#include "ndt/executor_select.h"

namespace ndt
{
HandlerSelectBase::~HandlerSelectBase()
{
    context_.executor().delHandler(this);
}

HandlerSelectBase::HandlerSelectBase(const HandlerSelectBase &) = default;

HandlerSelectBase::HandlerSelectBase(HandlerSelectBase &&) = default;

HandlerSelectBase::HandlerSelectBase(uint8_t aEventMask,
                                     InDataHandlerT aReadCallback,
                                     OutDataHandlerT aWriteCallback,
                                     ExceptCondHandlerT aExceptCondCallback,
                                     Context &aContext)
    : eventMask_(aEventMask)
    , inDataHandler_(aReadCallback)
    , outDataHandler_(aWriteCallback)
    , exceptCondHandler_(aExceptCondCallback)
    , context_(aContext)
{
}
}  // namespace ndt