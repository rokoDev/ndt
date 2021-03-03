#ifndef ndt_exception_h
#define ndt_exception_h

#include <system_error>

#include "common.h"

namespace ndt
{
class Error : public std::runtime_error
{
   public:
    virtual ~Error() noexcept {}

    explicit Error(std::error_code aEc)
        : std::runtime_error(""), mErrorCode(aEc)
    {
    }

    Error(std::error_code aEc, const std::string& aWhat)
        : std::runtime_error(aWhat), mErrorCode(aEc)
    {
    }

    Error(std::error_code aEc, const char* aWhat)
        : std::runtime_error(aWhat), mErrorCode(aEc)
    {
    }

    Error(int aEv, const std::error_category& aECat)
        : std::runtime_error(""), mErrorCode(aEv, aECat)
    {
    }

    Error(int aEv, const std::error_category& aECat, const std::string& aWhat)
        : std::runtime_error(aWhat), mErrorCode(aEv, aECat)
    {
    }

    Error(int aEv, const std::error_category& aECat, const char* aWhat)
        : std::runtime_error(aWhat), mErrorCode(aEv, aECat)
    {
    }

    std::error_code code() const noexcept { return mErrorCode; }

    const char* what() const noexcept override;

   private:
    std::error_code mErrorCode;
    mutable std::string mWhat;
};

}  // namespace ndt

#endif /* ndt_exception_h */