#include "ndt/platform/win/context_base_error.h"

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
}  // namespace ndt