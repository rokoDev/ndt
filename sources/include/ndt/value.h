#ifndef ndt_value_h
#define ndt_value_h

#include "bin_rw.h"
#include "interval.h"
#include "tag.h"

namespace ndt
{
template <typename T, typename MinT, typename MaxT>
class Value;

template <typename T, T MinV, T MaxV>
class Value<T, Min<MinV>, Max<MaxV>>
{
   public:
    static constexpr Interval<T, Min<MinV>, Max<MaxV>>
        kInterval; /**< Interval<kMinValue, kMaxValue>: constrains the value */
    static constexpr uint8_t kNumBits =
        utils::bits_count(kInterval.length()); /**< kNumBits: minimum bits count
                                                  required for serialization. */
    using UIntT = typename utils::uint_from_nbits_t<
        kNumBits>; /**< UIntT: minimum unsigned integer type suitable for
                      storing kNumBits bits. */
    using type = T;

    Value() = default;
    Value(const Value&) = default;
    Value<T, Min<MinV>, Max<MaxV>>& operator=(
        const Value<T, Min<MinV>, Max<MaxV>>&) = default;
    Value(Value&&) = default;
    Value& operator=(Value&&) = default;
    ~Value() = default;

    constexpr T get() const noexcept { return value; }

    constexpr bool set(T&& aValue) noexcept
    {
        const bool inside_interval =
            kInterval.contains(std::forward<T>(aValue));
        if (inside_interval)
        {
            value = aValue;
        }
        return inside_interval;
    }

    friend bool operator==(const Value& aVal1, const Value& aVal2)
    {
        return aVal1.value == aVal2.value;
    }

    friend bool operator<(const Value& aVal1, const Value& aVal2)
    {
        return aVal1.value < aVal2.value;
    }

    friend bool operator!=(const Value& aVal1, const Value& aVal2)
    {
        return aVal1.value != aVal2.value;
    }

    friend bool operator>(const Value& aVal1, const Value& aVal2)
    {
        return aVal1.value > aVal2.value;
    }

    friend bool operator<=(const Value& aVal1, const Value& aVal2)
    {
        return aVal1.value <= aVal2.value;
    }

    friend bool operator>=(const Value& aVal1, const Value& aVal2)
    {
        return aVal1.value >= aVal2.value;
    }

   private:
    T value = kInterval.kMin; /**< value: always belong to  kInterval. */
};

template <typename T>
struct is_value : std::false_type
{
};

template <typename T, T MinV, T MaxV>
struct is_value<Value<T, Min<MinV>, Max<MaxV>>> : std::true_type
{
    static constexpr T min_value = MinV;
    static constexpr T max_value = MaxV;
};

template <typename T>
inline constexpr bool is_value_v = is_value<std::decay_t<T>>::value;

template <typename T>
using enable_if_value_t = std::enable_if_t<is_value_v<T>, T>;

template <typename T>
inline constexpr auto min_value = is_value<std::decay_t<T>>::min_value;

template <typename T>
inline constexpr auto max_value = is_value<std::decay_t<T>>::max_value;

template <typename T>
[[nodiscard]] std::error_code serialize(BinWriter& aWriter, T&& aData,
                                        tag_t<enable_if_value_t<T>>) noexcept
{
    using ValueT = std::decay_t<T>;
    const auto value = aData.get();
    std::error_code ec;
    if (ValueT::kInterval.contains(value))
    {
        const auto valueToSave =
            static_cast<typename ValueT::UIntT>(value - min_value<T>);
        ec = aWriter.add<typename ValueT::UIntT>(valueToSave, ValueT::kNumBits);
    }
    else
    {
        ec = std::make_error_code(std::errc::invalid_argument);
    }
    return ec;
}

template <typename T>
[[nodiscard]] std::error_code deserialize(BinReader& aReader, T&& aData,
                                          tag_t<enable_if_value_t<T>>) noexcept
{
    using ValueT = std::decay_t<T>;
    std::error_code ec;
    const auto result =
        aReader.get<typename ValueT::UIntT>(ValueT::kNumBits, ec);
    if (!ec)
    {
        const bool success = aData.set(min_value<T> + result);
        if (!success)
        {
            ec = std::make_error_code(std::errc::result_out_of_range);
        }
    }
    return ec;
}

}  // namespace ndt

#endif /* ndt_value_h */