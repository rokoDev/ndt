#include "ndt/buffer.h"

#include <cstring>

namespace ndt
{
[[nodiscard]] std::error_code Buffer::copyFrom(CBuffer &aBuf) noexcept
{
    std::error_code retVal;
    const auto resultSize = aBuf.size();
    if (resultSize <= size_)
    {
        std::memcpy(data<void>(), aBuf.data<void>(), resultSize);
        size_ = resultSize;
    }
    else
    {
        retVal.assign(ENOMEM, std::system_category());
    }
    return retVal;
}

[[nodiscard]] std::error_code Buffer::copyFrom(Buffer &aBuf) noexcept
{
    CBuffer buf(aBuf);
    return copyFrom(buf);
}

}  // namespace ndt