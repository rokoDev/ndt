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
    assert(false && aValue && "error: currently bool is unsupported");
}

}  // namespace ndt