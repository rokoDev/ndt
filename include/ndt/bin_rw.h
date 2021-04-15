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
        byteIndex_ += (bitIndex_ + numBits) / kBitsInByte;
        bitIndex_ = (bitIndex_ + numBits) % kBitsInByte;
    }

    template <typename ResultT>
    constexpr ResultT filledMask(const uint8_t aNumBits) const noexcept
    {
        const ResultT leftAlignedMask =
            static_cast<ResultT>(static_cast<ResultT>(~ResultT(0))
                                 << (sizeof(ResultT) * kBitsInByte - aNumBits));
        const ResultT hostByteOrderMask = leftAlignedMask >> bitIndex_;
        const ResultT mask = utils::toNet<ResultT>(hostByteOrderMask);
        return mask;
    }

    mutable std::size_t byteIndex_ = 0;
    mutable uint8_t bitIndex_ = 0;
    static constexpr uint8_t kBitsInByte = 8;
};
}  // namespace details

class BinReader final : public details::BinBase<BinReader>
{
   public:
    constexpr explicit BinReader(CBuffer aBuf) noexcept : buffer_(aBuf) {}

    template <typename T>
    T get(const uint8_t aNumBits) const noexcept;

    template <typename T>
    T get() const noexcept
    {
        if constexpr (std::is_enum_v<T>)
        {
            return getEnum<T>();
        }
        else
        {
            if constexpr (std::is_signed_v<T>)
            {
                using UIntT = typename utils::UIntUnion<T>::UIntT;
                utils::UIntUnion<T> toT;
                toT.uintVal = get<UIntT>(sizeof(UIntT) * kBitsInByte);
                return toT.originalVal;
            }
            else
            {
                return get<T>(sizeof(T) * kBitsInByte);
            }
        }
    }

    template <>
    bool get<bool>() const noexcept
    {
        return get<uint8_t>(1);
    }

   private:
    template <typename E, typename U = std::enable_if_t<std::is_enum_v<E>>>
    E getEnum() const noexcept
    {
        using UIntT = typename utils::enum_properties<E>::SerializeT;
        return static_cast<E>(get<UIntT>(utils::enum_properties<E>::numBits));
    }

    CBuffer buffer_;
};

template <typename T>
T BinReader::get(const uint8_t aNumBits) const noexcept
{
    using ResultT = T;
    static_assert(std::is_unsigned_v<ResultT>, "T must be unsigned type");
    static_assert(std::is_integral_v<ResultT>, "T must be integral type");
    ResultT result;
    memcpy(&result, buffer_[byteIndex_], sizeof(ResultT));
    result &= filledMask<ResultT>(aNumBits);
    result = utils::toHost<ResultT>(result) << bitIndex_;
    result >>= sizeof(ResultT) * kBitsInByte - aNumBits;
    if (aNumBits + bitIndex_ > sizeof(ResultT) * kBitsInByte)
    {
        uint8_t lsbOffset =
            (1 + sizeof(ResultT)) * kBitsInByte - aNumBits - bitIndex_;
        uint8_t lsbValue = *buffer_[byteIndex_ + sizeof(ResultT)] >> lsbOffset;
        result |= lsbValue;
    }
    updateIndices(aNumBits);
    return result;
}

class BinWriter final : public details::BinBase<BinWriter>
{
   public:
    constexpr explicit BinWriter(Buffer aBuf) noexcept : buffer_(aBuf) {}

    template <typename T>
    void add(const T aValue, const uint8_t aNumBits) noexcept;

    template <typename T>
    void add(T aValue) noexcept
    {
        if constexpr (std::is_enum_v<T>)
        {
            addEnum(aValue);
        }
        else
        {
            if constexpr (std::is_signed_v<T>)
            {
                using UIntT = typename utils::UIntUnion<T>::UIntT;
                utils::UIntUnion<T> toUInt(aValue);
                add<UIntT>(toUInt.uintVal, sizeof(UIntT) * kBitsInByte);
            }
            else
            {
                add<T>(aValue, sizeof(T) * kBitsInByte);
            }
        }
    }

    template <>
    void add<bool>(const bool aValue) noexcept
    {
        add<uint8_t>(aValue, 1);
    }

   private:
    template <typename E, typename U = std::enable_if_t<std::is_enum_v<E>>>
    void addEnum(const E aEnumValue) noexcept
    {
        using UIntT = typename utils::enum_properties<E>::SerializeT;
        add<UIntT>(static_cast<UIntT>(aEnumValue),
                   utils::enum_properties<E>::numBits);
    }

    Buffer buffer_;
};

template <typename T>
void BinWriter::add(const T aValue, const uint8_t aNumBits) noexcept
{
    using ResultT = T;
    static_assert(std::is_unsigned_v<ResultT>, "T must be unsigned type");
    static_assert(std::is_integral_v<ResultT>, "T must be integral type");
    const ResultT leftAligned = aValue
                                << (sizeof(ResultT) * kBitsInByte - aNumBits);
    const ResultT targetValue = utils::toNet<ResultT>(leftAligned >> bitIndex_);
    ResultT dest;
    memcpy(&dest, buffer_[byteIndex_], sizeof(ResultT));
    dest &= ~filledMask<ResultT>(aNumBits);
    dest |= targetValue;
    memcpy(buffer_[byteIndex_], &dest, sizeof(ResultT));
    if (aNumBits + bitIndex_ > sizeof(ResultT) * kBitsInByte)
    {
        uint8_t lsbOffset =
            (1 + sizeof(ResultT)) * kBitsInByte - aNumBits - bitIndex_;
        uint8_t lsbValue =
            static_cast<uint8_t>(static_cast<uint8_t>(aValue) << lsbOffset);
        uint8_t lsbFilledMask = static_cast<uint8_t>(
            static_cast<uint8_t>(~uint8_t(0)) << lsbOffset);
        *buffer_[byteIndex_ + sizeof(ResultT)] &= ~lsbFilledMask;
        *buffer_[byteIndex_ + sizeof(ResultT)] |= lsbValue;
    }
    updateIndices(aNumBits);
}

}  // namespace ndt

#endif /* ndt_bin_rw_h */