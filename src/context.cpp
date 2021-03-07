#include "ndt/context.h"

#include <fmt/core.h>

#include <cassert>

#include "ndt/exception.h"

namespace ndt
{
const char *ContextErrorCategory::name() const noexcept
{
    return kContextErrorCategoryCStr;
}

std::string ContextErrorCategory::message(int c) const
{
    switch (static_cast<eContextErrorCode>(c))
    {
        case eContextErrorCode::kSuccess:
            return "success";
        case eContextErrorCode::kWinSockDLLNotFound:
            return kContextWinSockDLLNotFoundDescr;
        default:
            return "unknown";
    }
}

std::error_condition ContextErrorCategory::default_error_condition(
    int c) const noexcept
{
    return std::error_condition(c, *this);
}

Context::~Context()
{
    if (instanceCount_.fetch_sub(1, std::memory_order_relaxed) == 1)
    {
        return;
    }
    std::error_code ec;
    cleanUp(ec);
    if (ec)
    {
        fmt::print("error >>\ncategory: {}\ncode: {}\nmessage: {}\n",
                   ec.category().name(), ec.value(), ec.message());
        assert(false && "context clean up error");
    }
}

Context::Context()
{
    if (instanceCount_.fetch_add(1, std::memory_order_relaxed))
    {
        return;
    }
    std::error_code ec;
    startUp(ec);
    throw_if_error(ec);
}

Context::Context(std::error_code &aEc)
{
    if (instanceCount_.fetch_add(1, std::memory_order_relaxed))
    {
        return;
    }
    startUp(aEc);
}

void Context::startUp([[maybe_unused]] std::error_code &aEc) noexcept
{
#ifdef _WIN32
    WSADATA wsaData;
    constexpr uint8_t vMajor = 2;
    constexpr uint8_t vMinor = 2;
    const int err = ::WSAStartup(MAKEWORD(vMajor, vMinor), &wsaData);
    if (err != 0)
    {
        /* Tell the user that we could not find a usable */
        /* WinSock DLL.                                  */
        aEc.assign(systemErrorCodeGetter(), std::system_category());
        return;
    }

    /* Confirm that the WinSock DLL supports 2.2.*/
    /* Note that if the DLL supports versions greater    */
    /* than 2.2 in addition to 2.2, it will still return */
    /* 2.2 in wVersion since that is the version we      */
    /* requested.                                        */

    if (LOBYTE(wsaData.wVersion) != vMajor ||
        HIBYTE(wsaData.wVersion) != vMinor)
    {
        /* Tell the user that we could not find a usable */
        /* WinSock DLL.                                  */
        aEc = eContextErrorCode::kWinSockDLLNotFound;
        return;
    }

    /* The WinSock DLL is acceptable. Proceed. */
#endif
}
void Context::cleanUp([[maybe_unused]] std::error_code &aEc) noexcept
{
#ifdef _WIN32
    WSACleanup();
#endif
}

}  // namespace ndt