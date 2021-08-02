#include "ndt/file.h"

namespace ndt
{
const char *FileErrorCategory::name() const noexcept
{
    return kFileErrorCategoryCStr;
}

std::string FileErrorCategory::message(int c) const
{
    switch (static_cast<eFileErrorCode>(c))
    {
        case eFileErrorCode::kSuccess:
            return "success";
        case eFileErrorCode::kBusyFileDescriptor:
            return kBusyFileDescriptorDescr;
        case eFileErrorCode::kUnexpectedError:
            return kUnexpectedErrorDescr;
        default:
            return "unknown";
    }
}

std::error_condition FileErrorCategory::default_error_condition(
    int c) const noexcept
{
    return std::error_condition(c, *this);
}
}  // namespace ndt