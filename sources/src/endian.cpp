#include "ndt/endian.h"

namespace ndt
{
std::string_view endianName() noexcept
{
    using namespace std::literals;
    if (endian() == eEndian::kLittle)
    {
        return "kLittle"sv;
    }
    else if (endian() == eEndian::kBig)
    {
        return "kBig"sv;
    }
    else
    {
        return "kMixed"sv;
    }
}
}  // namespace ndt