#ifndef NetException_h
#define NetException_h

#include <fmt/core.h>

#include <exception>
#include <memory>
#include <stdexcept>
#include <string>

namespace net
{
namespace exception
{
inline const std::string kOnlyIPv4OrkIPv6FamilyAllowed =
    "address family can be kIPv4 or kIPv6 only";
inline const std::string kUnknownAddressFamily = "unknown address family: ";
inline const std::string kSocketAlreadyOpened = "socket already opened";
inline const std::string kSocketMustBeOpenToBind =
    "socket must be opened to bind";
inline const std::string kSocketOpen = "socket error";
inline const std::string kSocketBind = "bind error";
inline const std::string kSocketSendTo = "sendto error";
inline const std::string kSocketRecvFrom = "recvfrom error";
inline const std::string kSocketClose = "close error";

struct ErrorDescr
{
    ErrorDescr(const int aErrorCode, const int aLine, const char* aFile,
               const char* aFunc)
        : errorCode(aErrorCode), line(aLine), file(aFile), func(aFunc)
    {
    }
    int errorCode = 0;
    int line = 0;
    std::string file;
    std::string func;
};

template <class STD_ERROR>
class Error;

template <class STD_ERROR>
std::ostream& operator<<(std::ostream& aOut, const Error<STD_ERROR>& aError);

std::ostream& operator<<(std::ostream& aOut,
                         const Error<std::logic_error>& aError);

template <class STD_ERROR>
class Error : public STD_ERROR
{
   public:
    Error(const char* aReason, const int aErrorCode, const int aLine,
          const char* aFile, const char* aFunc);
    Error(const std::string& aReason, const int aErrorCode, const int aLine,
          const char* aFile, const char* aFunc);

    Error(const Error& aOther) noexcept;

    Error& operator=(const Error& aOther) noexcept;

    virtual const char* what() const noexcept;

    virtual void raise();

    int errorCode() const noexcept;
    int line() const noexcept;
    const std::string& file() const noexcept;
    const std::string& func() const noexcept;

    friend std::ostream& operator<<<>(std::ostream& aOut,
                                      const Error<STD_ERROR>& aError);
    friend std::ostream& operator<<(std::ostream& aOut,
                                    const Error<std::logic_error>& aError);

   private:
    std::shared_ptr<ErrorDescr> _errorDescr = nullptr;
};

template <class STD_ERROR>
Error<STD_ERROR>::Error(const char* aReason, const int aErrorCode,
                        const int aLine, const char* aFile, const char* aFunc)
    : STD_ERROR(aReason)
    , _errorDescr(std::make_shared<ErrorDescr>(aErrorCode, aLine, aFile, aFunc))
{
}

template <class STD_ERROR>
Error<STD_ERROR>::Error(const std::string& aReason, const int aErrorCode,
                        const int aLine, const char* aFile, const char* aFunc)
    : STD_ERROR(aReason)
    , _errorDescr(std::make_shared<ErrorDescr>(aErrorCode, aLine, aFile, aFunc))
{
}

template <class STD_ERROR>
Error<STD_ERROR>::Error(const Error& aOther) noexcept
    : STD_ERROR(aOther.what()), _errorDescr(aOther._errorDescr)
{
}

template <class STD_ERROR>
Error<STD_ERROR>& Error<STD_ERROR>::operator=(const Error& aOther) noexcept
{
    STD_ERROR::operator=(aOther);
    _errorDescr = aOther._errorDescr;
    return *this;
}

template <class STD_ERROR>
const char* Error<STD_ERROR>::what() const noexcept
{
    return STD_ERROR::what();
}

template <class STD_ERROR>
void Error<STD_ERROR>::raise()
{
    throw *this;
}

template <class STD_ERROR>
int Error<STD_ERROR>::errorCode() const noexcept
{
    return _errorDescr->errorCode;
}

template <class STD_ERROR>
int Error<STD_ERROR>::line() const noexcept
{
    return _errorDescr->line;
}

template <class STD_ERROR>
const std::string& Error<STD_ERROR>::file() const noexcept
{
    return _errorDescr->file;
}

template <class STD_ERROR>
const std::string& Error<STD_ERROR>::func() const noexcept
{
    return _errorDescr->func;
}

template <class STD_ERROR>
std::ostream& operator<<(std::ostream& aOut, const Error<STD_ERROR>& aError)
{
    const auto errorStr = fmt::format(
        "Runtime error:\nFile: {}\nFunction: {}\nLine: {}\nWhat: {}\nError "
        "code: {}\nError msg: {}\n",
        aError.file(), aError.func(), aError.line(), aError.what(),
        aError.errorCode(), std::strerror(aError.errorCode()));
    aOut << errorStr;
    return aOut;
}

using LogicError = Error<std::logic_error>;
using RuntimeError = Error<std::runtime_error>;

#define CheckLogicError(IsThrow, Reason)                             \
    {                                                                \
        if (IsThrow)                                                 \
        {                                                            \
            net::exception::LogicError logicError(                   \
                Reason, 0, __LINE__, __FILE__, __PRETTY_FUNCTION__); \
            logicError.raise();                                      \
        }                                                            \
    }

#define CheckRuntimeError(IsThrow, Reason)                               \
    {                                                                    \
        if (IsThrow)                                                     \
        {                                                                \
            net::exception::RuntimeError runtimeError(                   \
                Reason, errno, __LINE__, __FILE__, __PRETTY_FUNCTION__); \
            runtimeError.raise();                                        \
        }                                                                \
    }

}  // namespace exception
}  // namespace net

#endif /* NetException_h */