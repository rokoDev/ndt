#ifndef ndt_buffer_h
#define ndt_buffer_h

#include <cassert>
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

    uint32_t readInt32() const noexcept
    {
        assert(byteIndex_ + sizeof(uint32_t) <= size_);
        const uint32_t result = ntohl(*reinterpret_cast<uint32_t const *>(
            dataConst<char>() + byteIndex_));
        byteIndex_ += sizeof(uint32_t);
        return result;
    }

    void saveInt32(const uint32_t aValue) noexcept
    {
        assert(byteIndex_ + sizeof(uint32_t) <= size_);
        *reinterpret_cast<uint32_t *>(data<char>() + byteIndex_) =
            htonl(aValue);
        byteIndex_ += sizeof(aValue);
    }

    uint16_t readInt16() const noexcept
    {
        assert(byteIndex_ + sizeof(uint16_t) <= size_);
        const uint16_t result = ntohs(*reinterpret_cast<uint16_t const *>(
            dataConst<char>() + byteIndex_));
        byteIndex_ += sizeof(uint16_t);
        return result;
    }

    void saveInt16(const uint16_t aValue) noexcept
    {
        assert(byteIndex_ + sizeof(uint16_t) <= size_);
        *reinterpret_cast<uint16_t *>(data<char>() + byteIndex_) =
            htons(aValue);
        byteIndex_ += sizeof(aValue);
    }

    uint8_t readInt8() const noexcept
    {
        assert(byteIndex_ + sizeof(uint8_t) <= size_);
        const uint8_t result =
            *reinterpret_cast<uint8_t const *>(dataConst<char>() + byteIndex_);
        byteIndex_ += sizeof(uint8_t);
        return result;
    }

    void saveInt8(const uint8_t aValue) noexcept
    {
        assert(byteIndex_ + sizeof(uint8_t) <= size_);
        *(data<uint8_t>() + byteIndex_) = aValue;
        byteIndex_ += sizeof(aValue);
    }

    float readFloat() const noexcept
    {
        FloatInt tmpVal;
        tmpVal.uintVal = readInt32();
        return tmpVal.floatVal;
    }

    void saveFloat(const float aValue) noexcept
    {
        FloatInt tmpVal{aValue};
        saveInt32(tmpVal.uintVal);
    }

    void resetIndex() noexcept { byteIndex_ = 0; }
    uint16_t byteIndex() const noexcept { return byteIndex_; }

   private:
    union FloatInt
    {
        float floatVal;
        uint32_t uintVal;
    };

    bufp_t data_ = nullptr;
    dlen_t size_ = 0;
    mutable uint16_t byteIndex_ = 0;
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

    explicit CBuffer(const Buffer &aBuffer) noexcept
        : data_(aBuffer.dataConst()), size_(aBuffer.size())
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