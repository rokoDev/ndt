#ifndef ndt_buffer_h
#define ndt_buffer_h

#include <cassert>
#include <cstddef>
#include <cstdint>
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

    template <typename T = uint8_t>
    T &operator[](std::size_t aIndex) noexcept
    {
        return *(
            reinterpret_cast<T *>(reinterpret_cast<uint8_t *>(data_) + aIndex));
    }

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

    template <typename T = uint8_t>
    const T &operator[](std::size_t aIndex) const noexcept
    {
        return *(reinterpret_cast<T const *>(
            reinterpret_cast<uint8_t const *>(data_) + aIndex));
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
        return get<std::decay_t<T>, sizeof(std::decay_t<T>)>();
    }

    uint16_t byteIndex() const noexcept;
    uint8_t bitIndex() const noexcept;

   private:
    template <typename T, std::size_t n>
    T get() const noexcept
    {
        T result{};
        if constexpr (std::is_integral_v<T>)
        {
            assert(byteIndex_ + sizeof(T) <= buffer_.size<uint32_t>());
            if constexpr (n == 1)
            {
                if (bitIndex_ == 0)
                {
                    result = *reinterpret_cast<T const *>(buffer_.data<char>() +
                                                          byteIndex_);
                }
                else
                {
                    result =
                        buffer_.operator[]<uint16_t>(byteIndex_) >> bitIndex_;
                }

                byteIndex_ += sizeof(T);
            }
            else if constexpr (n == 2)
            {
                if (bitIndex_ == 0)
                {
                    result = ntohs(*reinterpret_cast<T const *>(
                        buffer_.data<char>() + byteIndex_));
                }
                else if (byteIndex_ + sizeof(uint32_t) <=
                         buffer_.size<uint32_t>())
                {
                    const uint16_t value = static_cast<uint16_t>(
                        buffer_.operator[]<uint32_t>(byteIndex_) >> bitIndex_);
                    result = ntohs(value);
                }
                else
                {
                    const uint16_t msbOriginal =
                        buffer_.operator[]<uint16_t>(byteIndex_) >> bitIndex_;
                    const uint16_t lsbOriginal = static_cast<uint16_t>(
                        buffer_.operator[]<uint16_t>(byteIndex_ + 1)
                        << (8 - bitIndex_));
                    result = ntohs(msbOriginal | lsbOriginal);
                }

                byteIndex_ += sizeof(T);
            }
            else if constexpr (n == 4)
            {
                if (bitIndex_ == 0)
                {
                    result = ntohl(*reinterpret_cast<T const *>(
                        buffer_.data<char>() + byteIndex_));
                }
                else if (byteIndex_ + sizeof(uint64_t) <=
                         buffer_.size<uint64_t>())
                {
                    const uint32_t value = static_cast<uint32_t>(
                        buffer_.operator[]<uint64_t>(byteIndex_) >> bitIndex_);
                    result = ntohl(value);
                }
                else
                {
                    const uint32_t msbOriginal =
                        buffer_.operator[]<uint32_t>(byteIndex_) >> bitIndex_;
                    const uint32_t lsbOriginal =
                        buffer_.operator[]<uint32_t>(byteIndex_ + 1)
                        << (8 - bitIndex_);
                    result = ntohl(msbOriginal | lsbOriginal);
                }
                byteIndex_ += sizeof(T);
            }
            else
            {
                static_assert((n == 1) || (n == 2) || (n == 4),
                              "error: unsupported integral type");
            }
        }
        else
        {
            static_assert(std::is_integral_v<T>, "error: unsupported type");
        }
        return result;
    }

    template <>
    float get<float, sizeof(float)>() const noexcept;

    template <>
    bool get<bool, sizeof(bool)>() const noexcept;

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
        add<std::decay_t<T>, sizeof(std::decay_t<T>)>(aValue);
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
                if (bitIndex_ == 0)
                {
                    *(buffer_.data<uint8_t>() + byteIndex_) = aValue;
                }
                else
                {
                    const uint16_t target = aValue << bitIndex_;
                    const uint16_t clearedTarget = ~uint16_t(0xff << bitIndex_);
                    const uint16_t original =
                        buffer_.operator[]<uint16_t>(byteIndex_);
                    buffer_.operator[]<uint16_t>(byteIndex_) =
                        (clearedTarget & original) | target;
                }
            }
            else if constexpr (n == 2)
            {
                if (bitIndex_ == 0)
                {
                    *reinterpret_cast<uint16_t *>(buffer_.data<char>() +
                                                  byteIndex_) = htons(aValue);
                }
                else
                {
                    const uint16_t target = htons(aValue);
                    const uint8_t msbTarget = target << bitIndex_;
                    const uint8_t msbCleared = ~uint8_t(0xff << bitIndex_);
                    const uint8_t msbOriginal =
                        buffer_.operator[]<uint8_t>(byteIndex_);
                    buffer_.operator[]<uint8_t>(byteIndex_) =
                        (msbCleared & msbOriginal) | msbTarget;

                    const uint16_t lsbTarget = target >> (8 - bitIndex_);
                    const uint16_t lsbCleared =
                        ~uint16_t(0xffff >> (8 - bitIndex_));
                    const uint16_t lsbOriginal =
                        buffer_.operator[]<uint16_t>(byteIndex_ + 1);
                    buffer_.operator[]<uint16_t>(byteIndex_ + 1) =
                        (lsbCleared & lsbOriginal) | lsbTarget;
                }
            }
            else if constexpr (n == 4)
            {
                if (bitIndex_ == 0)
                {
                    *reinterpret_cast<uint32_t *>(buffer_.data<char>() +
                                                  byteIndex_) = htonl(aValue);
                }
                else
                {
                    const uint32_t target = htonl(aValue);
                    const uint8_t msbTarget =
                        static_cast<uint8_t>(target << bitIndex_);
                    const uint8_t msbCleared = ~uint8_t(0xff << bitIndex_);
                    const uint8_t msbOriginal =
                        buffer_.operator[]<uint8_t>(byteIndex_);
                    buffer_.operator[]<uint8_t>(byteIndex_) =
                        (msbCleared & msbOriginal) | msbTarget;

                    const uint32_t lsbTarget = target >> (8 - bitIndex_);
                    const uint32_t lsbCleared =
                        ~uint32_t(0xffffffff >> (8 - bitIndex_));
                    const uint32_t lsbOriginal =
                        buffer_.operator[]<uint32_t>(byteIndex_ + 1);
                    buffer_.operator[]<uint32_t>(byteIndex_ + 1) =
                        (lsbCleared & lsbOriginal) | lsbTarget;
                }
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
            static_assert(std::is_integral_v<T>, "error: unsupported type");
        }
    }

    template <>
    void add<float, sizeof(float)>(const float aValue) noexcept;

    template <>
    void add<bool, sizeof(bool)>(const bool aValue) noexcept;

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