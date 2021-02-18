#include <iostream>

#include "ndt/exception.h"

namespace ndt
{
const char* Error::what() const noexcept
{
    if (mWhat.empty())
    {
        try
        {
            mWhat = this->std::runtime_error::what();
            if (!mWhat.empty())
            {
                mWhat += ": ";
            }
            mWhat += mErrorCode.message();
        }
        catch (...)
        {
            return std::runtime_error::what();
        }
    }
    return mWhat.c_str();
}

}  // namespace ndt