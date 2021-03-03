#ifndef ndt_sys_error_code_getter_h
#define ndt_sys_error_code_getter_h

#include "ndt/common.h"

namespace ndt
{
namespace test
{
class SysErrorCodeGetter
{
   public:
    enum class eState
    {
        kFalse = 0,
        kTrue,
        kSystem
    };
    SysErrorCodeGetter(const eState aIsError = eState::kTrue) noexcept
    {
        state(aIsError);
    }

    ~SysErrorCodeGetter() { state(eState::kSystem); }

    eState state() const noexcept
    {
        if (ndt::systemErrorCodeGetter == &systemErrorDidNotHappenGetter)
        {
            return eState::kFalse;
        }
        else if (ndt::systemErrorCodeGetter == &systemErrorHappenGetter)
        {
            return eState::kTrue;
        }
        else if (ndt::systemErrorCodeGetter == &ndt::details::lastErrorCode)
        {
            return eState::kSystem;
        }
        else
        {
            assert(false &&
                   "ndt::systemErrorCodeGetter has value which does not map to "
                   "any known state");
            return eState::kFalse;
        }
    }

    void state(const eState aIsError) noexcept
    {
        switch (aIsError)
        {
            case eState::kFalse:
            {
                ndt::systemErrorCodeGetter = &systemErrorDidNotHappenGetter;
                break;
            }
            case eState::kTrue:
            {
                ndt::systemErrorCodeGetter = &systemErrorHappenGetter;
                break;
            }
            case eState::kSystem:
            {
                ndt::systemErrorCodeGetter = &ndt::details::lastErrorCode;
                break;
            }
            default:
                assert(false &&
                       "invalid argument of type: SysErrorCodeGetter::eError");
                break;
        }
    }

   private:
    static int systemErrorHappenGetter() noexcept { return 10; }
    static int systemErrorDidNotHappenGetter() noexcept { return 0; }
};
}  // namespace test
}  // namespace ndt

#endif /* ndt_sys_error_code_getter_h */