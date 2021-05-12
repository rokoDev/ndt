#ifndef ndt_interval_h
#define ndt_interval_h

#include <type_traits>

namespace ndt
{
template <auto Value>
struct Min
{
};

template <auto Value>
struct Max
{
};

enum class eIntervalLocation : uint8_t
{
    kInside = 0b001,  //  value location is inside of the interval
    kAbove = 0b010,   //  value location is above the interval
    kBelow = 0b100    //  value location is below the interval
};

template <typename T, typename MinT, typename MaxT>
struct Interval;

template <typename T, T MinV, T MaxV>
struct Interval<T, Min<MinV>, Max<MaxV>>
{
    static_assert(MinV <= MaxV, "interval cannot be empty");
    static constexpr T kMin = MinV;
    static constexpr T kMax = MaxV;

    constexpr eIntervalLocation location(T aValue) const noexcept
    {
        eIntervalLocation result = eIntervalLocation::kInside;
        if (aValue > kMax)
        {
            result = eIntervalLocation::kAbove;
        }
        if (aValue < kMin)
        {
            result = eIntervalLocation::kBelow;
        }
        return result;
    }

    constexpr bool contains(T aValue) const noexcept
    {
        return location(aValue) == eIntervalLocation::kInside;
    }

    static constexpr auto length() noexcept
    {
        if constexpr ((MinV < 0) && (MaxV >= 0))
        {
            static_assert(
                MinV >
                    static_cast<T>(-1) * (std::numeric_limits<T>::max() - MaxV),
                "type T is insufficient to represent interval length");
        }
        return MaxV - MinV + 1;
    }
};
}  // namespace ndt

#endif /* ndt_interval_h */