#ifndef ndt_context_base_h
#define ndt_context_base_h

#include <atomic>
#include <system_error>

#include "../../common.h"
#include "../../useful_base_types.h"
#include "../../utils.h"
#include "context_base_error.h"

namespace ndt
{
template <typename SysWrapperT>
class ContextBase : private NoCopyAble
{
   public:
    ~ContextBase();
    ContextBase() noexcept;
    static int instanceCount() noexcept;

   private:
    void startUp(std::error_code &aEc) noexcept;
    void cleanUp(std::error_code &aEc) noexcept;

    static inline std::atomic_int32_t instanceCount_ = 0;
};

template <typename SysWrapperT>
ContextBase<SysWrapperT>::~ContextBase()
{
    if (instanceCount_.fetch_sub(1, std::memory_order_relaxed) == 1)
    {
        std::error_code ec;
        cleanUp(ec);
        utils::logErrorAndExit(ec, "context clean up error\n");
    }
}

template <typename SysWrapperT>
ContextBase<SysWrapperT>::ContextBase() noexcept
{
    if (instanceCount_.fetch_add(1, std::memory_order_relaxed))
    {
        return;
    }
    std::error_code ec;
    startUp(ec);
    utils::logErrorAndExit(ec, "context start up error\n");
}

template <typename SysWrapperT>
int ContextBase<SysWrapperT>::instanceCount() noexcept
{
    return std::atomic_load_explicit(&instanceCount_,
                                     std::memory_order_relaxed);
}

template <typename SysWrapperT>
void ContextBase<SysWrapperT>::startUp(std::error_code &aEc) noexcept
{
    WSADATA wsaData;
    constexpr uint8_t vMajor = 2;
    constexpr uint8_t vMinor = 2;
    const int err = SysWrapperT::WSAStartup(MAKEWORD(vMajor, vMinor), &wsaData);
    if (err != 0)
    {
        /* Tell the user that we could not find a usable */
        /* WinSock DLL.                                  */
        aEc.assign(SysWrapperT::lastErrorCode(), std::system_category());
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
}

template <typename SysWrapperT>
void ContextBase<SysWrapperT>::cleanUp(std::error_code &aEc) noexcept
{
    const auto result = SysWrapperT::WSACleanup();
    if (result == kSocketError)
    {
        aEc.assign(SysWrapperT::lastErrorCode(), std::system_category());
    }
}

}  // namespace ndt

#endif /* ndt_context_base_h */