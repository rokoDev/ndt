#ifndef ndt_event_handler_select_h
#define ndt_event_handler_select_h

#include <cstdint>
#include <type_traits>

namespace ndt
{
class Context;
class SocketBase;

class HandlerSelectBase
{
   private:
    template <typename SocketT, typename HandlerT>
    friend class HandlerSelect;

    template <typename ImplT>
    friend class ExecutorSelectBase;

    using InDataHandlerT = void (*)(ndt::SocketBase &, void *);
    using OutDataHandlerT = void (*)(ndt::SocketBase &, void *);
    using ExceptCondHandlerT = void (*)(ndt::SocketBase &, void *);

    enum eTrakingEvents : uint8_t
    {
        kNone = 0,
        kRead = 1 << 0,
        kWrite = 1 << 1,
        kExceptCond = 1 << 2,
        kAll = kRead | kWrite | kExceptCond
    };

    ~HandlerSelectBase();
    HandlerSelectBase() = delete;
    HandlerSelectBase(const HandlerSelectBase &);
    HandlerSelectBase &operator=(const HandlerSelectBase &) = delete;
    HandlerSelectBase(HandlerSelectBase &&);
    HandlerSelectBase &operator=(HandlerSelectBase &&) = delete;

    HandlerSelectBase(uint8_t aEventMask, InDataHandlerT aReadCallback,
                      OutDataHandlerT aWriteCallback,
                      ExceptCondHandlerT aExceptCondCallback,
                      Context &aContext);

    const uint8_t eventMask_;
    const InDataHandlerT inDataHandler_ = nullptr;
    const OutDataHandlerT outDataHandler_ = nullptr;
    const ExceptCondHandlerT exceptCondHandler_ = nullptr;

   protected:
    Context &context_;
};

#define CREATE_MEMBER_METHOD_EXISTANCE_CHECKER(methodName)                   \
    template <class C, class R, class... Args>                               \
    class CheckMethod_##methodName                                           \
    {                                                                        \
        template <class T, class ReturnT, class... ArgT>                     \
        static std::true_type testSignature(ReturnT (T::*)(ArgT...));        \
                                                                             \
        template <class T, class ReturnT, class... ArgT>                     \
        static decltype(testSignature(&T::methodName)) test(std::nullptr_t); \
        template <class T, class ReturnT, class... ArgT>                     \
        static std::false_type test(...);                                    \
                                                                             \
       public:                                                               \
        using type = decltype(test<C, R, Args...>(nullptr));                 \
        static constexpr bool value = type::value;                           \
    };

CREATE_MEMBER_METHOD_EXISTANCE_CHECKER(readHandlerImpl);
CREATE_MEMBER_METHOD_EXISTANCE_CHECKER(writeHandlerImpl);
CREATE_MEMBER_METHOD_EXISTANCE_CHECKER(exceptionConditionHandlerImpl);

template <typename ActualSocketT, typename ActualHandlerT>
class HandlerSelect : public HandlerSelectBase
{
   private:
    static constexpr uint8_t eventMask()
    {
        constexpr eTrakingEvents kReadMask =
            CheckMethod_readHandlerImpl<ActualHandlerT, void,
                                        ActualSocketT &>::value
                ? eTrakingEvents::kRead
                : eTrakingEvents::kNone;
        constexpr eTrakingEvents kWriteMask =
            CheckMethod_writeHandlerImpl<ActualHandlerT, void,
                                         ActualSocketT &>::value
                ? eTrakingEvents::kWrite
                : eTrakingEvents::kNone;
        constexpr eTrakingEvents kExceptCondMask =
            CheckMethod_exceptionConditionHandlerImpl<ActualHandlerT, void,
                                                      ActualSocketT &>::value
                ? eTrakingEvents::kExceptCond
                : eTrakingEvents::kNone;
        return kReadMask | kWriteMask | kExceptCondMask;
    }
    static void readHandler(ndt::SocketBase &s, void *aHandler)
    {
        if constexpr (static_cast<bool>(
                          eventMask() &
                          HandlerSelectBase::eTrakingEvents::kRead))
        {
            ActualSocketT &actualSocket = static_cast<ActualSocketT &>(s);
            ActualHandlerT *actualHandler =
                static_cast<ActualHandlerT *>(aHandler);
            actualHandler->readHandlerImpl(actualSocket);
        }
        static_assert(
            static_cast<bool>(eventMask() &
                              HandlerSelectBase::eTrakingEvents::kAll),
            "In order to be not useless handler class must have at least one "
            "of 'void readHandlerImpl(SocketT&)', 'void "
            "writeHandlerImpl(SocketT&)' or 'void "
            "exceptionConditionHandlerImpl(SocketT&)' methods to be defined.");
    }
    static void writeHandler(ndt::SocketBase &s, void *aHandler)
    {
        if constexpr (static_cast<bool>(
                          eventMask() &
                          HandlerSelectBase::eTrakingEvents::kWrite))
        {
            ActualSocketT &actualSocket = static_cast<ActualSocketT &>(s);
            ActualHandlerT *actualHandler =
                static_cast<ActualHandlerT *>(aHandler);
            actualHandler->writeHandlerImpl(actualSocket);
        }
    }
    static void exceptionConditionHandler(ndt::SocketBase &s, void *aHandler)
    {
        if constexpr (static_cast<bool>(
                          eventMask() &
                          HandlerSelectBase::eTrakingEvents::kExceptCond))
        {
            ActualSocketT &actualSocket = static_cast<ActualSocketT &>(s);
            ActualHandlerT *actualHandler =
                static_cast<ActualHandlerT *>(aHandler);
            actualHandler->exceptionConditionHandlerImpl(actualSocket);
        }
    }

   protected:
    ~HandlerSelect() = default;
    HandlerSelect(Context &aContext)
        : HandlerSelectBase(eventMask(), &HandlerSelect::readHandler,
                            &HandlerSelect::writeHandler,
                            &HandlerSelect::exceptionConditionHandler, aContext)
    {
    }
    HandlerSelect(const HandlerSelect &) = default;
    HandlerSelect &operator=(const HandlerSelect &) = delete;
    HandlerSelect(HandlerSelect &&) = default;
    HandlerSelect &operator=(HandlerSelect &&) = delete;
};
}  // namespace ndt

#endif /* ndt_event_handler_select_h */