#include "NetException.h"

#include <iostream>

namespace net
{
namespace exception
{
std::ostream& operator<<(std::ostream& aOut,
                         const Error<std::logic_error>& aError)
{
    const auto errorStr = fmt::format(
        "Logic error:\nFile: {}\nFunction: {}\nLine: {}\nWhat: {}\n",
        aError.file(), aError.func(), aError.line(), aError.what());
    aOut << errorStr;
    return aOut;
}

}  // namespace exception
}  // namespace net