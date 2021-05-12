#ifndef ndt_buffer_h
#define ndt_buffer_h

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <system_error>
#include <type_traits>

#include "common.h"

namespace ndt
{
class CBuffer;
class Buffer
{
   public:
    constexpr Buffer(void *aData, std::size_t aSize) noexcept
        : data_(static_cast<bufp_t>(aData)), size_(aSize)
    {
    }

    constexpr Buffer(char *aData, std::size_t aSize) noexcept
        : data_(static_cast<bufp_t>(aData)), size_(aSize)
    {
    }

    inline void setSize(const std::size_t aSize) { size_ = aSize; }

    inline uint8_t *operator[](std::size_t aIndex) noexcept
    {
        return static_cast<uint8_t *>(static_cast<void *>(data_)) + aIndex;
    }

    template <size_t N, typename T>
    constexpr explicit Buffer(T (&aData)[N]) noexcept
        : data_(static_cast<bufp_t>(aData)), size_(sizeof(T) * N)
    {
    }

    template <typename T = buf_t>
    T *data() noexcept
    {
        return static_cast<T *>(data_);
    }

    template <typename T = buf_t>
    T const *dataConst() const noexcept
    {
        return static_cast<T const *>(data_);
    }

    template <typename T = dlen_t>
    T size() const noexcept
    {
        return static_cast<T>(size_);
    }

    [[nodiscard]] std::error_code copyFrom(CBuffer &aBuf) noexcept;
    [[nodiscard]] std::error_code copyFrom(Buffer &aBuf) noexcept;

   private:
    bufp_t data_ = nullptr;
    dlen_t size_ = 0;
};

class CBuffer
{
   public:
    constexpr CBuffer(void const *aData, std::size_t aSize) noexcept
        : data_(static_cast<cbufp_t>(aData)), size_(aSize)
    {
    }
    constexpr CBuffer(char const *aData, std::size_t aSize) noexcept
        : data_(static_cast<cbufp_t>(aData)), size_(aSize)
    {
    }

    constexpr explicit CBuffer(const Buffer &aBuffer) noexcept
        : data_(aBuffer.dataConst()), size_(aBuffer.size())
    {
    }

    template <size_t N, typename T>
    constexpr explicit CBuffer(T (&aData)[N])
        : data_(static_cast<cbufp_t>(aData)), size_(sizeof(T) * N)
    {
    }

    uint8_t const *operator[](std::size_t aIndex) const noexcept
    {
        return static_cast<uint8_t const *>(static_cast<void const *>(data_)) +
               aIndex;
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