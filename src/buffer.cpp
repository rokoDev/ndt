#include "ndt/buffer.h"

#include <fmt/core.h>

namespace ndt
{
void Buffer::setSize(const std::size_t aSize) { size_ = aSize; }

template <>
float BufferReader::get<float, sizeof(float)>() const noexcept
{
    FloatInt tmpVal;
    tmpVal.uintVal = get<uint32_t, sizeof(uint32_t)>();
    return tmpVal.floatVal;
}

template <>
bool BufferReader::get<bool, sizeof(bool)>() const noexcept
{
    const bool result = (buffer_[byteIndex_] >> bitIndex_) & uint8_t{1};
    byteIndex_ += (bitIndex_ + 1) / 8;
    bitIndex_ = (bitIndex_ + 1) % 8;
    return result;
}

uint16_t BufferReader::byteIndex() const noexcept { return byteIndex_; }
uint8_t BufferReader::bitIndex() const noexcept { return bitIndex_; }

BufferWriter::~BufferWriter() = default;

BufferWriter::BufferWriter(Buffer aBuf) : buffer_(aBuf) {}

uint16_t BufferWriter::byteIndex() const noexcept { return byteIndex_; }
uint8_t BufferWriter::bitIndex() const noexcept { return bitIndex_; }

template <>
void BufferWriter::add<float, sizeof(float)>(const float aValue) noexcept
{
    FloatInt tmpVal{aValue};
    add<uint32_t, sizeof(uint32_t)>(tmpVal.uintVal);
}

template <>
void BufferWriter::add<bool, sizeof(bool)>(const bool aValue) noexcept
{
    if (aValue)
    {
        // set bit
        buffer_[byteIndex_] = buffer_[byteIndex_] | uint8_t(1 << bitIndex_);
    }
    else
    {
        // reset bit
        buffer_[byteIndex_] = buffer_[byteIndex_] & ~uint8_t(1 << bitIndex_);
    }
    byteIndex_ += (bitIndex_ + 1) / 8;
    bitIndex_ = (bitIndex_ + 1) % 8;
}

}  // namespace ndt