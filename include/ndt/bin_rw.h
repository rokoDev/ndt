#ifndef ndt_bin_rw_h
#define ndt_bin_rw_h

#include "buffer.h"
#include "utils.h"

namespace ndt
{
namespace details
{
template <typename T>
class BinBase
{
   public:
    std::size_t byteIndex() const noexcept { return byteIndex_; }
    uint8_t bitIndex() const noexcept { return bitIndex_; }

   protected:
    ~BinBase() = default;
    BinBase() = default;
    BinBase(const BinBase &) = default;
    BinBase &operator=(const BinBase &) = default;
    BinBase(BinBase &&) noexcept = default;
    BinBase &operator=(BinBase &&) noexcept = default;

    void updateIndices(const uint8_t numBits) const noexcept
    {
        byteIndex_ += (bitIndex_ + numBits) / 8;
        bitIndex_ = (bitIndex_ + numBits) % 8;
    }

    union FloatInt
    {
        float floatVal;
        uint32_t uintVal;
    };
    union DoubleInt
    {
        double doubleVal;
        uint64_t uintVal;
    };

    mutable std::size_t byteIndex_ = 0;
    mutable uint8_t bitIndex_ = 0;
    static constexpr uint8_t kBitsInByte = 8;
};
}  // namespace details

class BinReader final : public details::BinBase<BinReader>
{
   public:
    constexpr explicit BinReader(CBuffer aBuf) noexcept : buffer_(aBuf) {}

    uint8_t getBits(const uint8_t aNumBits /*aNumBits < 8*/)
    {
        return aNumBits;
    }

    template <typename T>
    auto get(const uint8_t aNumBits) const noexcept
    {
        using ResultT = std::decay_t<T>;
        ResultT result;
        static_assert(std::is_integral_v<ResultT>, "T must be integral type");
        memcpy(&result, buffer_[byteIndex_], sizeof(ResultT));
        const ResultT filledMask = utils::toNet<ResultT>(
            static_cast<ResultT>((
                (~ResultT(0)) << (sizeof(ResultT) * kBitsInByte - aNumBits))) >>
            bitIndex_);
        result &= filledMask;
        result = utils::toHost<ResultT>(result) << bitIndex_;
        result >>= sizeof(ResultT) * kBitsInByte - aNumBits;
        if (aNumBits + bitIndex_ > sizeof(ResultT) * kBitsInByte)
        {
            uint8_t lsbOffset =
                (1 + sizeof(ResultT)) * kBitsInByte - aNumBits - bitIndex_;
            uint8_t lsbValue =
                *buffer_[byteIndex_ + sizeof(ResultT)] >> lsbOffset;
            result |= lsbValue;
        }
        updateIndices(aNumBits);
        return result;
    }

    template <typename T>
    auto get() const noexcept
    {
        using ResultT = std::decay_t<T>;
        return get<ResultT>(sizeof(ResultT) * 8);
    }

    template <>
    auto get<bool>() const noexcept
    {
        return get<uint8_t>(1);
    }

    template <>
    auto get<float>() const noexcept
    {
        FloatInt tmpVal;
        tmpVal.uintVal = get<uint32_t>();
        return tmpVal.floatVal;
    }

    template <>
    auto get<double>() const noexcept
    {
        DoubleInt tmpVal;
        tmpVal.uintVal = get<uint64_t>();
        return tmpVal.doubleVal;
    }

   private:
    CBuffer buffer_;
};

class BinWriter final : public details::BinBase<BinWriter>
{
   public:
    constexpr explicit BinWriter(Buffer aBuf) noexcept : buffer_(aBuf) {}

    template <typename T>
    void add(const std::decay_t<T> aValue, const uint8_t aNumBits) noexcept
    {
        using ResultT = std::decay_t<T>;
        static_assert(std::is_integral_v<ResultT>, "T must be integral type");
        const ResultT leftAligned =
            aValue << (sizeof(ResultT) * kBitsInByte - aNumBits);
        const ResultT targetValue =
            utils::toNet<ResultT>(leftAligned >> bitIndex_);
        const ResultT filledMask = utils::toNet<ResultT>(
            static_cast<ResultT>((
                (~ResultT(0)) << (sizeof(ResultT) * kBitsInByte - aNumBits))) >>
            bitIndex_);
        ResultT dest;
        memcpy(&dest, buffer_[byteIndex_], sizeof(ResultT));
        dest &= ~filledMask;
        dest |= targetValue;
        memcpy(buffer_[byteIndex_], &dest, sizeof(ResultT));
        if (aNumBits + bitIndex_ > sizeof(ResultT) * kBitsInByte)
        {
            uint8_t lsbOffset =
                (1 + sizeof(ResultT)) * kBitsInByte - aNumBits - bitIndex_;
            uint8_t lsbValue = static_cast<uint8_t>(aValue) << lsbOffset;
            uint8_t lsbFilledMask = 0xff << lsbOffset;
            *buffer_[byteIndex_ + sizeof(ResultT)] &= ~lsbFilledMask;
            *buffer_[byteIndex_ + sizeof(ResultT)] |= lsbValue;
        }
        updateIndices(aNumBits);
    }

    template <typename T>
    void add(const T aValue) noexcept
    {
        using ResultT = std::decay_t<T>;
        add<ResultT>(aValue, sizeof(ResultT) * 8);
    }

    template <>
    void add<bool>(const bool aValue) noexcept
    {
        add<uint8_t>(aValue, 1);
    }

    template <>
    void add<float>(const float aValue) noexcept
    {
        FloatInt tmpVal{aValue};
        add<uint32_t>(tmpVal.uintVal);
    }

    template <>
    void add<double>(const double aValue) noexcept
    {
        DoubleInt tmpVal{aValue};
        add<uint64_t>(tmpVal.uintVal);
    }

   private:
    Buffer buffer_;
};
}  // namespace ndt

#endif /* ndt_bin_rw_h */