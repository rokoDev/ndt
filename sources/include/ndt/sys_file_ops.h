#ifndef ndt_sys_file_ops_h
#define ndt_sys_file_ops_h

#include <cstdio>

#include "sys_error_code.h"
#include "useful_base_types.h"

struct stat;

namespace ndt
{
class SysFileOps
{
   public:
    [[nodiscard]] static FILE* fopen(const char* filename,
                                     const char* mode) noexcept;
    [[nodiscard]] static int fclose(FILE* stream) noexcept;
    [[nodiscard]] static int remove(const char *filename) noexcept;
    [[nodiscard]] static size_t fread(void *ptr, size_t size_of_elements,
                                      size_t number_of_elements,
                                      FILE *a_file) noexcept;
    [[nodiscard]] static size_t fwrite(const void *ptr, size_t size_of_elements,
                                       size_t number_of_elements,
                                       FILE *a_file) noexcept;
    [[nodiscard]] static int ferror(FILE *stream) noexcept;
    [[nodiscard]] static int feof(FILE *stream) noexcept;
    [[nodiscard]] static int stat(const char *path, struct stat *buf) noexcept;
    [[nodiscard]] static int fflush(FILE *stream) noexcept;
    [[nodiscard]] static int fseek(FILE *stream, long int offset,
                                   int origin) noexcept;
    [[nodiscard]] static long int ftell(FILE *stream) noexcept;
    static void rewind(FILE *stream) noexcept;
};

class FileOps
    : public NoConstructibleNoDestructible
    , public SysErrorCode
    , public SysFileOps
{
};

}  // namespace ndt

#endif /* ndt_sys_file_ops_h */