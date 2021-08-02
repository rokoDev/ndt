#ifndef ndt_sys_error_code_h
#define ndt_sys_error_code_h

#include "common.h"
#include "useful_base_types.h"

namespace ndt
{
class SysErrorCode
{
   public:
    inline static int lastErrorCode() { return err_code; }
};
}  // namespace ndt

#endif /* ndt_sys_error_code_h */