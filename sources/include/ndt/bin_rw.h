#ifndef ndt_bin_rw_h
#define ndt_bin_rw_h

#include <algorithm>
#include <cstring>
#include <type_traits>

#include "buffer.h"
#include "endian.h"
#include "utils.h"

namespace ndt
{
namespace details
{
template <typename T>
class BinBase
{
   public:
    inline std::size_t byteIndex() const noexcept { return byteIndex_; }
    inline uint8_t bitIndex() const noexcept { return bitIndex_; }
    inline std::size_t size() const noexcept
    {
        return byteIndex_ + ((bitIndex_ != 0) ? 1 : 0);
    }
    inline std::size_t bitSize() const noexcept
    {
        return byteIndex_ * 8 + bitIndex_;
    }
    inline std::size_t bitCapacity() const noexcept
    {
        return (static_cast<T const *>(this))->bitCapacityImpl();
    }
    inline std::size_t bitsLeft() const noexcept
    {
        return bitCapacity() - bitSize();
    }
    inline void reset() const noexcept
    {
        byteIndex_ = 0;
        bitIndex_ = 0;
    }
    inline void alignByte() const noexcept
    {
        if (bitIndex_ != 0)
        {
            bitIndex_ = 0;
            ++byteIndex_;
        }
    }

   protected:
    ~BinBase() = default;
    BinBase() = default;
    BinBase(const BinBase &) = default;
    BinBase &operator=(const BinBase &) = default;
    BinBase(BinBase &&) noexcept = default;
    BinBase &operator=(BinBase &&) noexcept = default;

    using IndexesT = std::tuple<std::size_t, uint8_t>;

    IndexesT extractIndexes() const noexcept { return {byteIndex_, bitIndex_}; }

    void applyIndexes(const IndexesT &aIndexes) const noexcept
    {
        byteIndex_ = std::get<0>(aIndexes);
        bitIndex_ = std::get<1>(aIndexes);
    }

    void updateIndices(const uint8_t numBits) const noexcept
    {
        byteIndex_ += (bitIndex_ + numBits) / kBitsInByte;
        bitIndex_ = (bitIndex_ + numBits) % kBitsInByte;
    }

    template <typename UIntT>
    constexpr UIntT filledMask(const uint8_t aNumBits) const noexcept
    {
        const UIntT leftAlignedMask =
            static_cast<UIntT>(static_cast<UIntT>(~UIntT(0))
                               << (utils::num_bits<UIntT>() - aNumBits));
        const UIntT hostByteOrderMask = leftAlignedMask >> bitIndex_;
        const UIntT mask = toNet<UIntT>(hostByteOrderMask);
        return mask;
    }

    template <typename SizeT>
    std::error_code checkIsAlignedSizeAvailable(SizeT aSize) const noexcept
    {
        static_assert(std::is_integral_v<SizeT>, "SizeT must be integral type");
        if (bitsLeft() / kBitsInByte >= aSize)
        {
            if (aSize > 0)
            {
                return std::error_code();
            }
            else
            {
                // buffer size must be greater than 0
                return std::make_error_code(std::errc::invalid_argument);
            }
        }
        else
        {
            // not enough space to add new data
            return std::make_error_code(std::errc::no_buffer_space);
        }
    }

    template <typename SizeT>
    std::error_code checkAlignedBufArgs(void const *const aBuf,
                                        SizeT aSize) const noexcept
    {
        if (!aBuf)
        {
            // invalid data pointer
            return std::make_error_code(std::errc::bad_address);
        }
        return checkIsAlignedSizeAvailable(aSize);
    }

    mutable std::size_t byteIndex_ = 0;
    mutable uint8_t bitIndex_ = 0;
    static constexpr uint8_t kBitsInByte = 8;
};
}  // namespace details

class BinReader final : public details::BinBase<BinReader>
{
    friend class details::BinBase<BinReader>;

   public:
    constexpr explicit BinReader(CBuffer aBuf) noexcept : buffer_(aBuf) {}

    template <typename T>
    std::error_code get(T &aValue, const uint8_t aNumBits) const noexcept;

    template <typename T>
    T get(const uint8_t aNumBits, std::error_code &aEc) const noexcept
    {
        T result;
        aEc = get(result, aNumBits);
        return result;
    }

    template <typename T>
    T get(std::error_code &aEc) const noexcept
    {
        T result;
        aEc = get(result);
        return result;
    }

    template <typename T>
    std::error_code get(T &aValue) const noexcept
    {
        static_assert(std::is_enum_v<T> || std::is_arithmetic_v<T>,
                      "T must be enum or arithmetic");
        if constexpr (std::is_enum_v<T>)
        {
            return getEnum(aValue);
        }
        else
        {
            using UIntT =
                typename utils::uint_from_nbits_t<utils::num_bits<T>()>;
            UIntT result;
            const auto ec = get<UIntT>(result, utils::num_bits<UIntT>());
            if (!ec)
            {
                aValue = utils::bit_cast<T>(result);
            }
            return ec;
        }
    }

    template <>
    std::error_code get<bool>(bool &aValue) const noexcept
    {
        uint8_t result;
        const auto ec = get(result, 1);
        if (!ec)
        {
            aValue = result;
        }
        return ec;
    }

    std::error_code get(void *aBuf, const std::size_t aSize) const noexcept
    {
        const auto ec = checkAlignedBufArgs(aBuf, aSize);
        if (!ec)
        {
            alignByte();
            std::memcpy(aBuf, buffer_.data<char>() + byteIndex_, aSize);
            byteIndex_ += aSize;
        }
        return ec;
    }

    std::error_code get(Buffer &aBuffer) const noexcept
    {
        const auto kOriginIndexes = extractIndexes();
        alignByte();
        std::error_code ec;
        const auto kStoredBufSize = get<uint16_t>(ec);
        if (!ec)
        {
            if (kStoredBufSize <= aBuffer.size())
            {
                ec = get(aBuffer.data<void>(), kStoredBufSize);
            }
            else
            {
                // aBuffer size is not enough to store saved buffer
                ec = std::make_error_code(std::errc::no_buffer_space);
            }
        }
        if (!ec)
        {
            aBuffer.setSize(kStoredBufSize);
        }
        else
        {
            applyIndexes(kOriginIndexes);
        }
        return ec;
    }

   private:
    template <typename E, typename U = std::enable_if_t<std::is_enum_v<E>>>
    std::error_code getEnum(E &aValue) const noexcept
    {
        static_assert(std::is_unsigned_v<std::underlying_type_t<E>>,
                      "E must have unsigned underlying type");
        static_assert(
            utils::to_underlying(E::Error) == utils::to_underlying(E::Count),
            "E::Count must be equal to E::Error");
        using UIntT = typename utils::enum_properties<E>::SerializeT;
        UIntT result;
        const auto ec = get<UIntT>(result, utils::enum_properties<E>::numBits);
        if (!ec)
        {
            result = std::min(result, static_cast<UIntT>(E::Count));
            aValue = static_cast<E>(result);
        }
        return ec;
    }

    std::size_t bitCapacityImpl() const noexcept
    {
        return buffer_.size() * kBitsInByte;
    }

    CBuffer buffer_;
};

template <typename T>
std::error_code BinReader::get(T &aValue, const uint8_t aNumBits) const noexcept
{
    static_assert(std::is_unsigned_v<T>, "T must be unsigned type");
    static_assert(std::is_integral_v<T>, "T must be integral type");
    if (aNumBits <= bitsLeft())
    {
        std::memcpy(&aValue, buffer_[byteIndex_], sizeof(T));
        aValue &= filledMask<T>(aNumBits);
        aValue = static_cast<T>(toHost<T>(aValue) << bitIndex_);
        aValue >>= utils::num_bits<T>() - aNumBits;
        if (aNumBits + bitIndex_ > utils::num_bits<T>())
        {
            uint8_t lsbOffset =
                (1 + sizeof(T)) * kBitsInByte - aNumBits - bitIndex_;
            uint8_t lsbValue = *buffer_[byteIndex_ + sizeof(T)] >> lsbOffset;
            aValue |= lsbValue;
        }
        updateIndices(aNumBits);
        return std::error_code();
    }
    else
    {
        return std::make_error_code(std::errc::no_message_available);
    }
}

class BinWriter final : public details::BinBase<BinWriter>
{
    friend class details::BinBase<BinWriter>;

   public:
    constexpr explicit BinWriter(Buffer aBuf) noexcept : buffer_(aBuf) {}

    template <typename T>
    std::error_code add(const T aValue, const uint8_t aNumBits) noexcept;

    template <typename T>
    std::error_code add(T aValue) noexcept
    {
        static_assert(std::is_enum_v<T> || std::is_arithmetic_v<T>,
                      "T must be enum or arithmetic");
        if constexpr (std::is_enum_v<T>)
        {
            return addEnum(aValue);
        }
        else
        {
            using UIntT =
                typename utils::uint_from_nbits_t<utils::num_bits<T>()>;
            return add<UIntT>(utils::bit_cast<UIntT>(aValue),
                              utils::num_bits<UIntT>());
        }
    }

    template <>
    std::error_code add<bool>(const bool aValue) noexcept
    {
        return add<uint8_t>(aValue, 1);
    }

    std::error_code add(void const *aValue, const std::size_t aSize) noexcept
    {
        const auto ec = checkAlignedBufArgs(aValue, aSize);
        if (!ec)
        {
            alignByte();
            std::memcpy(buffer_.data<char>() + byteIndex_, aValue, aSize);
            byteIndex_ += aSize;
        }
        return ec;
    }

    std::error_code add(const CBuffer &aBuffer) noexcept
    {
        const auto kOriginIndexes = extractIndexes();
        alignByte();
        std::error_code ec =
            add<uint16_t>(static_cast<uint16_t>(aBuffer.size()));
        if (!ec)
        {
            ec = add(aBuffer.data<void>(), aBuffer.size());
        }
        if (ec)
        {
            applyIndexes(kOriginIndexes);
        }
        return ec;
    }

   private:
    template <typename E, typename U = std::enable_if_t<std::is_enum_v<E>>>
    std::error_code addEnum(const E aEnumValue) noexcept
    {
        static_assert(std::is_unsigned_v<std::underlying_type_t<E>>,
                      "E must have unsigned underlying type");
        using UIntT = typename utils::enum_properties<E>::SerializeT;
        return add<UIntT>(static_cast<UIntT>(aEnumValue),
                          utils::enum_properties<E>::numBits);
    }

    std::size_t bitCapacityImpl() const noexcept
    {
        return buffer_.size() * kBitsInByte;
    }

    Buffer buffer_;
};

template <typename T>
std::error_code BinWriter::add(const T aValue, const uint8_t aNumBits) noexcept
{
    static_assert(std::is_unsigned_v<T>, "T must be unsigned type");
    static_assert(std::is_integral_v<T>, "T must be integral type");
    if (bitsLeft() >= aNumBits)
    {
        const T leftAligned =
            static_cast<T>(aValue << (utils::num_bits<T>() - aNumBits));
        const T targetValue = toNet<T>(leftAligned >> bitIndex_);
        T dest;
        std::memcpy(&dest, buffer_[byteIndex_], sizeof(T));
        dest &= ~filledMask<T>(aNumBits);
        dest |= targetValue;
        std::memcpy(buffer_[byteIndex_], &dest, sizeof(T));
        if (aNumBits + bitIndex_ > utils::num_bits<T>())
        {
            uint8_t lsbOffset =
                (1 + sizeof(T)) * kBitsInByte - aNumBits - bitIndex_;
            uint8_t lsbValue =
                static_cast<uint8_t>(static_cast<uint8_t>(aValue) << lsbOffset);
            uint8_t lsbFilledMask = static_cast<uint8_t>(
                static_cast<uint8_t>(~uint8_t(0)) << lsbOffset);
            *buffer_[byteIndex_ + sizeof(T)] &= ~lsbFilledMask;
            *buffer_[byteIndex_ + sizeof(T)] |= lsbValue;
        }
        updateIndices(aNumBits);
        return std::error_code();
    }
    else
    {
        return std::make_error_code(std::errc::no_buffer_space);
    }
}

}  // namespace ndt

#endif /* ndt_bin_rw_h */