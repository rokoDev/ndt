#ifndef serialize_h
#define serialize_h

#include <boost/pfr.hpp>
#include <chrono>
#include <type_traits>

#include "bin_rw.h"
#include "tag.h"
#include "type_name.h"
#include "utils.h"

namespace pfr = boost::pfr;

namespace ndt
{
template <typename T>
using enable_if_enum_or_arithmetic_t =
    std::enable_if_t<std::is_enum_v<std::decay_t<T>> ||
                         std::is_arithmetic_v<std::decay_t<T>>,
                     T>;

template <typename T>
using enable_if_class_agregate_t =
    std::enable_if_t<std::is_aggregate_v<std::decay_t<T>> &&
                         std::is_class_v<std::decay_t<T>>,
                     T>;

template <typename T>
struct is_std_duration : std::false_type
{
};

template <typename Rep, typename Period>
struct is_std_duration<std::chrono::duration<Rep, Period>> : std::true_type
{
};

template <typename T>
inline constexpr bool is_std_duration_v =
    is_std_duration<std::decay_t<T>>::value;

template <typename T>
using enable_if_std_duration_t = std::enable_if_t<is_std_duration_v<T>, T>;

template <typename T>
struct is_parameterized_with_enum : std::false_type
{
};

template <template <auto> typename U, auto kEnumV>
struct is_parameterized_with_enum<U<kEnumV>>
    : std::conditional_t<std::is_enum_v<decltype(kEnumV)>, std::true_type,
                         std::false_type>
{
    using enum_type = decltype(kEnumV);
    static constexpr enum_type enum_value = kEnumV;
};

template <typename T>
inline constexpr bool is_parameterized_with_enum_v =
    is_parameterized_with_enum<std::decay_t<T>>::value;

template <typename T>
using enum_param_t =
    typename is_parameterized_with_enum<std::decay_t<T>>::enum_type;

template <typename T>
inline constexpr auto enum_param_v =
    is_parameterized_with_enum<std::decay_t<T>>::enum_value;

template <typename T>
[[nodiscard]] std::error_code serialize(
    BinWriter &aWriter, T &&aData,
    tag_t<enable_if_enum_or_arithmetic_t<T>>) noexcept
{
    return aWriter.add<std::decay_t<T>>(std::forward<T>(aData));
}

template <typename T>
[[nodiscard]] std::error_code deserialize(
    BinReader &aReader, T &&aData,
    tag_t<enable_if_enum_or_arithmetic_t<T>>) noexcept
{
    return aReader.get(std::forward<T>(aData));
}

template <typename T>
std::error_code serialize(BinWriter &aWriter, T &&aData,
                          tag_t<enable_if_std_duration_t<T>>) noexcept
{
    using TypeToSave =
        std::conditional_t<std::is_floating_point_v<decltype(aData.count())>,
                           double, int64_t>;
    const TypeToSave count = aData.count();
    return aWriter.add<TypeToSave>(count);
}

template <typename T>
std::error_code deserialize(BinReader &aReader, T &&aData,
                            tag_t<enable_if_std_duration_t<T>>) noexcept
{
    using DurationT = std::decay_t<T>;
    using TypeToRead =
        std::conditional_t<std::is_floating_point_v<decltype(aData.count())>,
                           double, int64_t>;
    std::error_code ec;
    aData = DurationT{aReader.get<TypeToRead>(ec)};
    return ec;
}

template <typename T, std::size_t... I>
std::error_code serializeStruct(BinWriter &aWriter, T &&aData,
                                std::index_sequence<I...>) noexcept
{
    std::error_code ec;
    (false || ... ||
     (ec = serialize(aWriter, pfr::get<I>(std::forward<T>(aData)))));
    return ec;
}

template <typename T, std::size_t... I>
std::error_code deserializeStruct(BinReader &aReader, T &&aData,
                                  std::index_sequence<I...>) noexcept
{
    std::error_code ec;
    (false || ... ||
     (ec = deserialize(aReader, pfr::get<I>(std::forward<T>(aData)))));
    return ec;
}

template <typename T>
std::error_code serialize(BinWriter &aWriter, T &&aData,
                          tag_t<enable_if_class_agregate_t<T>>) noexcept
{
    if constexpr (is_parameterized_with_enum_v<T>)
    {
        const auto ec = aWriter.add<enum_param_t<T>>(enum_param_v<T>);
        if (ec)
        {
            return ec;
        }
    }
    constexpr std::size_t kNumFields = pfr::tuple_size_v<std::decay_t<T>>;
    return serializeStruct(aWriter, std::forward<T>(aData),
                           std::make_index_sequence<kNumFields>{});
}

template <typename T>
std::error_code deserialize(BinReader &aReader, T &&aData,
                            tag_t<enable_if_class_agregate_t<T>>) noexcept
{
    constexpr std::size_t kNumFields = pfr::tuple_size_v<std::decay_t<T>>;
    return deserializeStruct(aReader, std::forward<T>(aData),
                             std::make_index_sequence<kNumFields>{});
}

template <typename T>
[[nodiscard]] std::error_code serialize(BinWriter &aWriter, T &&aData) noexcept
{
    return serialize(aWriter, std::forward<T>(aData), tag<T>);
}

template <typename T>
[[nodiscard]] std::error_code deserialize(BinReader &aReader,
                                          T &&aData) noexcept
{
    static_assert(std::is_lvalue_reference_v<T>, "T must be lvalue reference");
    static_assert(!std::is_const_v<std::remove_reference_t<T>>,
                  "T must not have const qualifier");
    return deserialize(aReader, std::forward<T>(aData), tag<T>);
}

template <typename T>
T deserialize(BinReader &aReader, std::error_code &aEc) noexcept
{
    T result{};
    aEc = deserialize(aReader, result);
    return result;
}

}  // namespace ndt

#endif /* serialize_h */