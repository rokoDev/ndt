#ifndef ndt_buffer_h
#define ndt_buffer_h

#include <cstddef>

#include "common.h"

namespace ndt
{
class Buffer
{
   public:
    Buffer(void *aData, std::size_t aSize) noexcept
        : data_(static_cast<bufp_t>(aData)), size_(aSize)
    {
    }
    Buffer(char *aData, std::size_t aSize) noexcept
        : data_(static_cast<bufp_t>(aData)), size_(aSize)
    {
    }

    template <size_t N, typename T>
    explicit Buffer(T (&aData)[N])
        : data_(static_cast<bufp_t>(aData)), size_(sizeof(T) * N)
    {
    }

    void setSize(const std::size_t aSize) { size_ = aSize; }

    template <typename T = buf_t>
    T *data() noexcept
    {
        return static_cast<T *>(data_);
    }

    template <typename T = dlen_t>
    T size() const noexcept
    {
        return static_cast<T>(size_);
    }

   private:
    bufp_t data_ = nullptr;
    dlen_t size_ = 0;
};

class CBuffer
{
   public:
    CBuffer(void const *aData, std::size_t aSize) noexcept
        : data_(static_cast<cbufp_t>(aData)), size_(aSize)
    {
    }
    CBuffer(char const *aData, std::size_t aSize) noexcept
        : data_(static_cast<cbufp_t>(aData)), size_(aSize)
    {
    }

    CBuffer(Buffer &aBuffer) noexcept
        : data_(aBuffer.data()), size_(aBuffer.size())
    {
    }

    template <size_t N, typename T>
    explicit CBuffer(T (&aData)[N])
        : data_(static_cast<cbufp_t>(aData)), size_(sizeof(T) * N)
    {
    }

    template <typename T = buf_t>
    T const *data() const noexcept
    {
        return static_cast<T const *>(data_);
    }

    template <typename T = dlen_t>
    T size() const noexcept
    {
        return static_cast<T>(size_);
    }

   private:
    cbufp_t data_ = nullptr;
    dlen_t size_ = 0;
};
}  // namespace ndt

#endif /* ndt_buffer_h */