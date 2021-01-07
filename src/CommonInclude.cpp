#ifndef _WIN32
#include <stdio.h>
#endif
#include <cassert>
#include <fmt/format.h>

#include "CommonInclude.h"

namespace ndt
{
std::size_t getSysErrorDescr(const int aErrorCode, ch_t *aBuf,
                             const std::size_t aBufLen)
{
    assert((aBuf != nullptr) && "Error: aBuf can't be nullptr");
    assert((aBufLen != 0) && "Error: aBufLen can't equal 0");
    std::size_t msgSizeWithoutZero = 0;
#ifdef _WIN32
    msgSizeWithoutZero = FormatMessage(
        FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS |
            FORMAT_MESSAGE_MAX_WIDTH_MASK,
        nullptr, aErrorCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR)aBuf, aBufLen, nullptr);
    if (!msgSizeWithoutZero)
    {
        msgSizeWithoutZero =
            fmt::format_to_n(
                aBuf, aBufLen,
                "Fail to get description for error code {} because of error:{}",
                aErrorCode, GetLastError())
                .size;
    }
    return msgSizeWithoutZero;
#else
    if (aErrorCode < sys_nerr)
    {
        msgSizeWithoutZero =
            fmt::format_to_n(aBuf, aBufLen, sys_errlist[aErrorCode]).size;
    }
    else
    {
        msgSizeWithoutZero =
            fmt::format_to_n(aBuf, aBufLen, "Unknown error: {}", aErrorCode)
                .size;
    }
    return msgSizeWithoutZero;
#endif
}
}  // namespace ndt