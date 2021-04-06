#ifndef ndt_buffer_h
#define ndt_buffer_h

#include <cassert>
#include <cstddef>
#include <type_traits>

#include "common.h"

namespace ndt
{
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

    void setSize(const std::size_t aSize);

    template <size_t N, typename T>
    constexpr explicit Buffer(T (&aData)[N]) noexcept
        : data_(static_cast<bufp_t>(aData)), size_(sizeof(T) * N)
    {
    }

    template <typename T = buf_t>
    T *data() noexcept
    {
        return reinterpret_cast<T *>(data_);
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

class BufferReader
{
   public:
    constexpr explicit BufferReader(CBuffer aBuf) noexcept : buffer_(aBuf) {}

    template <typename T>
    std::remove_const_t<T> get() const noexcept
    {
        return get<std::remove_const_t<T>, sizeof(T)>();
    }

    uint16_t byteIndex() const noexcept;
    uint8_t bitIndex() const noexcept;

   private:
    template <typename T, std::size_t n>
    T get() const noexcept
    {
        if constexpr (std::is_integral_v<T>)
        {
            assert(byteIndex_ + sizeof(T) <= buffer_.size<uint32_t>());
            if constexpr (n == 1)
            {
                const T result = *reinterpret_cast<T const *>(
                    buffer_.data<char>() + byteIndex_);
                byteIndex_ += sizeof(T);
                return result;
            }
            else if constexpr (n == 2)
            {
                const T result = ntohs(*reinterpret_cast<T const *>(
                    buffer_.data<char>() + byteIndex_));
                byteIndex_ += sizeof(T);
                return result;
            }
            else if constexpr (n == 4)
            {
                const T result = ntohl(*reinterpret_cast<T const *>(
                    buffer_.data<char>() + byteIndex_));
                byteIndex_ += sizeof(T);
                return result;
            }
            else
            {
                static_assert((n == 1) || (n == 2) || (n == 4),
                              "error: unsupported integral type");
            }
        }
        else
        {
            static_assert(!std::is_integral_v<T>, "error: unsupported type");
        }
    }
    union FloatInt
    {
        float floatVal;
        uint32_t uintVal;
    };

    CBuffer buffer_;
    mutable uint16_t byteIndex_ = 0;
    mutable uint8_t bitIndex_ = 0;
};

class BufferWriter
{
   public:
    ~BufferWriter();
    BufferWriter(Buffer aBuf);

    template <typename T>
    void add(const T aValue) noexcept
    {
        add<std::remove_const_t<T>, sizeof(T)>(aValue);
    }

    uint16_t byteIndex() const noexcept;
    uint8_t bitIndex() const noexcept;

   private:
    template <typename T, std::size_t n>
    void add(const T aValue) noexcept
    {
        if constexpr (std::is_integral_v<T>)
        {
            assert(byteIndex_ + sizeof(T) <= buffer_.size<uint32_t>());
            if constexpr (n == 1)
            {
                *(buffer_.data<uint8_t>() + byteIndex_) = aValue;
            }
            else if constexpr (n == 2)
            {
                *reinterpret_cast<uint16_t *>(buffer_.data<char>() +
                                              byteIndex_) = htons(aValue);
            }
            else if constexpr (n == 4)
            {
                *reinterpret_cast<uint32_t *>(buffer_.data<char>() +
                                              byteIndex_) = htonl(aValue);
            }
            else
            {
                static_assert((n == 1) || (n == 2) || (n == 4),
                              "error: unsupported integral type");
            }
            byteIndex_ += sizeof(aValue);
        }
        else
        {
            static_assert(!std::is_integral_v<T>, "error: unsupported type");
        }
    }

    union FloatInt
    {
        float floatVal;
        uint32_t uintVal;
    };

    Buffer buffer_;
    mutable uint16_t byteIndex_ = 0;
    mutable uint8_t bitIndex_ = 0;
};
}  // namespace ndt

#endif /* ndt_buffer_h */