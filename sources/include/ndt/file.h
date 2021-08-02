#ifndef ndt_file_h
#define ndt_file_h

#include <sys/stat.h>

#include <string>
#include <system_error>
#include <type_traits>
#include <utility>

#include "ndt/buffer.h"
#include "ndt/sys_file_ops.h"
#include "ndt/useful_base_types.h"

namespace ndt
{
inline const char *kFileErrorCategoryCStr = "ndt_file_ec";
inline const std::string kBusyFileDescriptorDescr =
    "File descriptor is busy. You should call File::close(std::error_code&) "
    "before associate it with another file";
inline const std::string kUnexpectedErrorDescr =
    "Some unexpected error occured.";

enum class eFileErrorCode
{
    kSuccess = 0,
    kBusyFileDescriptor,
    kUnexpectedError,
};

class FileErrorCategory : public std::error_category
{
   public:
    virtual const char *name() const noexcept override final;
    virtual std::string message(int c) const override final;
    virtual std::error_condition default_error_condition(
        int c) const noexcept override final;
};

inline const FileErrorCategory &getFileErrorCategory()
{
    static FileErrorCategory c;
    return c;
}

inline std::error_code make_error_code(eFileErrorCode e)
{
    return {static_cast<int>(e), getFileErrorCategory()};
}
}  // namespace ndt

namespace std
{
// Tell the C++ 11 STL metaprogramming that enum ndt::eAddressErrorCode
// is registered with the standard error code system
template <>
struct is_error_code_enum<ndt::eFileErrorCode> : true_type
{
};
}  // namespace std

namespace ndt
{
/// Is a strongly typed enum class representing opening file' modes
enum class eFileMode : std::uint8_t
{
    kTxtReadExistent, /**< Opens existing file in text mode for reading or
                         return error if file does not exist*/
    kBinReadExistent, /**< Opens existing file in binary mode for reading or
                         return error if file does not exist*/

    kTxtOpenCreateOverW, /**< Creates new or open existing file in text mode for
                            writing. Previous content will be lost.*/
    kBinOpenCreateOverW, /**< Creates new or open existing file in binary mode
                            for writing. Previous content will be lost.*/

    kTxtOpenCreateAppend, /**< Creates new or open existing file in text mode
                             for appending additional content.*/
    kBinOpenCreateAppend, /**< Creates new or open existing file in binary mode
                             for appending additional content.*/

    kTxtRWExistent, /**< Opens existing file in text mode for reading and
                       writing or return error if file doesn't exist. Position
                       indicator will be set on the file start so write
                       operation can erase previous content.*/
    kBinRWExistent, /**< Opens existing file in binary mode for reading and
                       writing or return error if file doesn't exist. Position
                       indicator will be set on the file start so write
                       operation can erase previous content.*/

    kTxtOpenCreateROverW, /**< Creates new or open existing file in text mode
                             for reading and writing. Previous content will be
                             lost.*/
    kBinOpenCreateROverW, /**< Creates new or open existing file in binary mode
                             for reading and writing. Previous content will be
                             lost.*/

    kTxtOpenCreateRA, /**< Creates new or open existing file in text mode for
                         reading and appending. Position indicator will be set
                         to the end of file. Previous content will not be
                         lost.*/
    kBinOpenCreateRA, /**< Creates new or open existing file in binary mode for
                         reading and appending. Position indicator will be set
                         to the end of file. Previous content will not be
                         lost.*/
};

using DefaultErrorHandler = void (*)(const std::error_code &aEc) noexcept;
template <typename DestructorErrorHandlerT = DefaultErrorHandler,
          typename SysWrapperT = FileOps>
class File final : public NoCopyAble
{
   public:
    ~File();

    File(DestructorErrorHandlerT &&aDestructorErrorHandler = nullptr) noexcept;

    File(File &&aFile) noexcept;

    File &operator=(File &&aFile) noexcept;

    void open(const char *aFileName, const char *aMode,
              std::error_code &aEc) noexcept;

    template <typename BufferT>
    void write(BufferT &&aBuf, std::error_code &aEc) noexcept;

    template <typename SuccessHandlerT, typename ErrorHandlerT,
              typename EOFHandlerT>
    void read(ndt::Buffer &aBuf, SuccessHandlerT &&aSuccessHandler,
              ErrorHandlerT &&aErrorHandler, EOFHandlerT &&aEOFHander) noexcept;

    void read(ndt::Buffer &aBuf, std::error_code &aEc) noexcept;

    void rewind() noexcept;

    template <eFileMode kMode>
    void open(const char *aFileName, std::error_code &aEc) noexcept;

    template <>
    void open<eFileMode::kTxtReadExistent>(const char *aFileName,
                                           std::error_code &aEc) noexcept
    {
        open(aFileName, "r", aEc);
    }

    template <>
    void open<eFileMode::kBinReadExistent>(const char *aFileName,
                                           std::error_code &aEc) noexcept
    {
        open(aFileName, "rb", aEc);
    }

    template <>
    void open<eFileMode::kTxtOpenCreateOverW>(const char *aFileName,
                                              std::error_code &aEc) noexcept
    {
        open(aFileName, "w", aEc);
    }

    template <>
    void open<eFileMode::kBinOpenCreateOverW>(const char *aFileName,
                                              std::error_code &aEc) noexcept
    {
        open(aFileName, "wb", aEc);
    }

    template <>
    void open<eFileMode::kTxtOpenCreateAppend>(const char *aFileName,
                                               std::error_code &aEc) noexcept
    {
        open(aFileName, "a", aEc);
    }

    template <>
    void open<eFileMode::kBinOpenCreateAppend>(const char *aFileName,
                                               std::error_code &aEc) noexcept
    {
        open(aFileName, "ab", aEc);
    }

    template <>
    void open<eFileMode::kTxtRWExistent>(const char *aFileName,
                                         std::error_code &aEc) noexcept
    {
        open(aFileName, "r+", aEc);
    }

    template <>
    void open<eFileMode::kBinRWExistent>(const char *aFileName,
                                         std::error_code &aEc) noexcept
    {
        open(aFileName, "rb+", aEc);
    }

    template <>
    void open<eFileMode::kTxtOpenCreateROverW>(const char *aFileName,
                                               std::error_code &aEc) noexcept
    {
        open(aFileName, "w+", aEc);
    }

    template <>
    void open<eFileMode::kBinOpenCreateROverW>(const char *aFileName,
                                               std::error_code &aEc) noexcept
    {
        open(aFileName, "wb+", aEc);
    }

    template <>
    void open<eFileMode::kTxtOpenCreateRA>(const char *aFileName,
                                           std::error_code &aEc) noexcept
    {
        open(aFileName, "a+", aEc);
    }

    template <>
    void open<eFileMode::kBinOpenCreateRA>(const char *aFileName,
                                           std::error_code &aEc) noexcept
    {
        open(aFileName, "ab+", aEc);
    }

    void close(std::error_code &aEc) noexcept;
    void flush(std::error_code &aEc) noexcept;
    static void remove(const char *aFileName, std::error_code &aEc) noexcept;
    static auto size(const char *aPath, std::error_code &aEc) noexcept;

   private:
    template <typename SuccessHandlerT, typename ErrorHandlerT,
              typename EOFHandlerT>
    void read(ndt::Buffer &aBuf, std::error_code &aEc,
              SuccessHandlerT &&aSuccessHandler, ErrorHandlerT &&aErrorHandler,
              EOFHandlerT &&aEOFHander) noexcept;

    FILE *fd_ = nullptr;
    DestructorErrorHandlerT destructorErrorHandler_;
};

template <typename DestructorErrorHandlerT, typename SysWrapperT>
File<DestructorErrorHandlerT, SysWrapperT>::~File()
{
    std::error_code errorCode;
    close(errorCode);
    if constexpr (std::is_pointer_v<DestructorErrorHandlerT>)
    {
        if (errorCode && destructorErrorHandler_)
        {
            destructorErrorHandler_(errorCode);
        }
    }
    else
    {
        if (errorCode)
        {
            destructorErrorHandler_(errorCode);
        }
    }
}

template <typename DestructorErrorHandlerT, typename SysWrapperT>
File<DestructorErrorHandlerT, SysWrapperT>::File(
    DestructorErrorHandlerT &&aDestructorErrorHandler) noexcept
    : destructorErrorHandler_(aDestructorErrorHandler)
{
    static_assert(std::is_nothrow_invocable_v<DestructorErrorHandlerT,
                                              const std::error_code &>,
                  "DestructorErrorHandlerT must have prototype like this: void "
                  "aDestructorErrorHandler(const std::error_code &) noexcept");
}

template <typename DestructorErrorHandlerT, typename SysWrapperT>
File<DestructorErrorHandlerT, SysWrapperT>::File(File &&aFile) noexcept
    : fd_(aFile.fd_), destructorErrorHandler_(aFile.destructorErrorHandler_)
{
    aFile.fd_ = nullptr;
}

template <typename DestructorErrorHandlerT, typename SysWrapperT>
File<DestructorErrorHandlerT, SysWrapperT> &
File<DestructorErrorHandlerT, SysWrapperT>::operator=(File &&aFile) noexcept
{
    fd_ = aFile.fd_;
    destructorErrorHandler_ = aFile.destructorErrorHandler_;
    aFile.fd_ = nullptr;
    return *this;
}

template <typename DestructorErrorHandlerT, typename SysWrapperT>
void File<DestructorErrorHandlerT, SysWrapperT>::open(
    const char *aFileName, const char *aMode, std::error_code &aEc) noexcept
{
    if (fd_ == nullptr)
    {
        fd_ = SysWrapperT::fopen(aFileName, aMode);
        if (fd_ == nullptr)
        {
            aEc.assign(SysWrapperT::lastErrorCode(), std::system_category());
        }
    }
    else
    {
        aEc = eFileErrorCode::kBusyFileDescriptor;
    }
}

template <typename DestructorErrorHandlerT, typename SysWrapperT>
template <typename BufferT>
void File<DestructorErrorHandlerT, SysWrapperT>::write(
    BufferT &&aBuf, std::error_code &aEc) noexcept
{
    constexpr size_t kElementsToWrite = 1;
    const size_t writtenCount =
        SysWrapperT::fwrite(aBuf.data(), kElementsToWrite, aBuf.size(), fd_);
    if ((writtenCount != aBuf.size()) && SysWrapperT::ferror(fd_))
    {
        aEc.assign(SysWrapperT::lastErrorCode(), std::system_category());
    }
}

template <typename DestructorErrorHandlerT, typename SysWrapperT>
template <typename SuccessHandlerT, typename ErrorHandlerT,
          typename EOFHandlerT>
void File<DestructorErrorHandlerT, SysWrapperT>::read(
    ndt::Buffer &aBuf, SuccessHandlerT &&aSuccessHandler,
    ErrorHandlerT &&aErrorHandler, EOFHandlerT &&aEOFHander) noexcept
{
    std::error_code errorCode;
    read(aBuf, errorCode, std::forward<SuccessHandlerT>(aSuccessHandler),
         std::forward<ErrorHandlerT>(aErrorHandler),
         std::forward<EOFHandlerT>(aEOFHander));
}

template <typename DestructorErrorHandlerT, typename SysWrapperT>
void File<DestructorErrorHandlerT, SysWrapperT>::read(
    ndt::Buffer &aBuf, std::error_code &aEc) noexcept
{
    read(
        aBuf, aEc, [](ndt::Buffer &) noexcept {},
        [](const std::error_code &) noexcept {}, [](ndt::Buffer &) noexcept {});
}

template <typename DestructorErrorHandlerT, typename SysWrapperT>
void File<DestructorErrorHandlerT, SysWrapperT>::rewind() noexcept
{
    SysWrapperT::rewind(fd_);
}

template <typename DestructorErrorHandlerT, typename SysWrapperT>
void File<DestructorErrorHandlerT, SysWrapperT>::close(
    std::error_code &aEc) noexcept
{
    if (fd_ != nullptr)
    {
        const int isError = fclose(fd_);
        fd_ = nullptr;
        if (isError)
        {
            aEc.assign(SysWrapperT::lastErrorCode(), std::system_category());
        }
    }
}

template <typename DestructorErrorHandlerT, typename SysWrapperT>
void File<DestructorErrorHandlerT, SysWrapperT>::flush(
    std::error_code &aEc) noexcept
{
    const int isError = SysWrapperT::fflush(fd_);
    if (isError)
    {
        if (SysWrapperT::ferror(fd_))
        {
            aEc.assign(SysWrapperT::lastErrorCode(), std::system_category());
        }
        else
        {
            aEc = eFileErrorCode::kUnexpectedError;
        }
    }
}

template <typename DestructorErrorHandlerT, typename SysWrapperT>
void File<DestructorErrorHandlerT, SysWrapperT>::remove(
    const char *aFileName, std::error_code &aEc) noexcept
{
    const int isError = SysWrapperT::remove(aFileName);
    if (isError)
    {
        aEc.assign(SysWrapperT::lastErrorCode(), std::system_category());
    }
}

template <typename DestructorErrorHandlerT, typename SysWrapperT>
auto File<DestructorErrorHandlerT, SysWrapperT>::size(
    const char *aPath, std::error_code &aEc) noexcept
{
    struct stat statBuf;
    decltype(statBuf.st_size) fileSize = 0;
    const int isError = SysWrapperT::stat(aPath, &statBuf);
    if (!isError)
    {
        fileSize = statBuf.st_size;
    }
    else
    {
        aEc.assign(SysWrapperT::lastErrorCode(), std::system_category());
    }
    return fileSize;
}

template <typename DestructorErrorHandlerT, typename SysWrapperT>
template <typename SuccessHandlerT, typename ErrorHandlerT,
          typename EOFHandlerT>
void File<DestructorErrorHandlerT, SysWrapperT>::read(
    ndt::Buffer &aBuf, std::error_code &aEc, SuccessHandlerT &&aSuccessHandler,
    ErrorHandlerT &&aErrorHandler, EOFHandlerT &&aEOFHander) noexcept
{
    static_assert(std::is_nothrow_invocable_v<SuccessHandlerT, ndt::Buffer &>,
                  "SuccessHandlerT must have prototype like this: void "
                  "aSuccessHandler(ndt::Buffer&) noexcept");
    static_assert(
        std::is_nothrow_invocable_v<ErrorHandlerT, const std::error_code &>,
        "ErrorHandlerT must have prototype like this: void aErrorHandler(const "
        "std::error_code &) noexcept");
    static_assert(std::is_nothrow_invocable_v<EOFHandlerT, ndt::Buffer &>,
                  "EOFHandlerT must have prototype like this: void "
                  "aEOFHander(ndt::Buffer&) noexcept");

    if constexpr (std::is_pointer_v<SuccessHandlerT>)
    {
        assert(aSuccessHandler && "Error: aSuccessHandler may not be nullptr");
    }

    if constexpr (std::is_pointer_v<ErrorHandlerT>)
    {
        assert(aErrorHandler && "Error: aErrorHandler may not be nullptr");
    }

    if constexpr (std::is_pointer_v<EOFHandlerT>)
    {
        assert(aEOFHander && "Error: aEOFHander may not be nullptr");
    }

    constexpr size_t kOneElementSize = 1;
    const size_t bytesRead = SysWrapperT::fread(
        aBuf.data<void>(), kOneElementSize, aBuf.size<size_t>(), fd_);
    if (bytesRead == aBuf.size<size_t>())
    {
        aBuf.setSize(bytesRead);
        aSuccessHandler(aBuf);
    }
    else
    {
        if (SysWrapperT::ferror(fd_))
        {
            aEc.assign(SysWrapperT::lastErrorCode(), std::system_category());
            aErrorHandler(aEc);
        }
        else if (SysWrapperT::feof(fd_))
        {
            aBuf.setSize(bytesRead);
            aEOFHander(aBuf);
        }
        else
        {
            aEc = eFileErrorCode::kUnexpectedError;
            aErrorHandler(aEc);
        }
    }
}

}  // namespace ndt

#endif /* ndt_file_h */