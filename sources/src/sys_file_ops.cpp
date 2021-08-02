#if _WIN32
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "ndt/sys_file_ops.h"

#include <sys/stat.h>

namespace ndt
{
FILE* SysFileOps::fopen(const char* filename, const char* mode) noexcept
{
    return ::fopen(filename, mode);
}

int SysFileOps::fclose(FILE* stream) noexcept { return ::fclose(stream); }

int SysFileOps::remove(const char *filename) noexcept
{
    return ::remove(filename);
}

size_t SysFileOps::fread(void *ptr, size_t size_of_elements,
                         size_t number_of_elements, FILE *a_file) noexcept
{
    return ::fread(ptr, size_of_elements, number_of_elements, a_file);
}

size_t SysFileOps::fwrite(const void *ptr, size_t size_of_elements,
                          size_t number_of_elements, FILE *a_file) noexcept
{
    return ::fwrite(ptr, size_of_elements, number_of_elements, a_file);
}

int SysFileOps::ferror(FILE *stream) noexcept { return ::ferror(stream); }

int SysFileOps::feof(FILE *stream) noexcept { return ::feof(stream); }

int SysFileOps::stat(const char *path, struct stat *buf) noexcept
{
    return ::stat(path, buf);
}

int SysFileOps::fflush(FILE *stream) noexcept { return ::fflush(stream); }

int SysFileOps::fseek(FILE *stream, long int offset, int origin) noexcept
{
    return ::fseek(stream, offset, origin);
}

long int SysFileOps::ftell(FILE *stream) noexcept { return ::ftell(stream); }

void SysFileOps::rewind(FILE *stream) noexcept { ::rewind(stream); }

}  // namespace ndt