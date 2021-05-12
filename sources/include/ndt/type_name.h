#ifndef ndt_type_name_h
#define ndt_type_name_h

#include <string_view>

namespace ndt
{
template <typename T>
constexpr std::string_view type_name();

template <>
constexpr std::string_view type_name<float>()
{
    return "float";
}

namespace details
{
using type_name_prober = float;

template <typename T>
constexpr std::string_view clattered_name()
{
#ifdef __clang__
    return __PRETTY_FUNCTION__;
#elif defined(_MSC_VER)
    return __FUNCSIG__;
#elif defined(__GNUC__)
    return __PRETTY_FUNCTION__;
#else
#error "Compiler unrecognized"
#endif
}

constexpr std::size_t wrapped_type_name_prefix_length()
{
    return clattered_name<type_name_prober>().find(
        type_name<type_name_prober>());
}

constexpr std::size_t wrapped_type_name_suffix_length()
{
    return clattered_name<type_name_prober>().length() -
           wrapped_type_name_prefix_length() -
           type_name<type_name_prober>().length();
}

}  // namespace details

template <typename T>
constexpr std::string_view type_name()
{
    constexpr auto wrapped_name = details::clattered_name<T>();
    constexpr auto prefix_length = details::wrapped_type_name_prefix_length();
    constexpr auto suffix_length = details::wrapped_type_name_suffix_length();
    constexpr auto type_name_length =
        wrapped_name.length() - prefix_length - suffix_length;
    return wrapped_name.substr(prefix_length, type_name_length);
}
}  // namespace ndt

#endif /* ndt_type_name_h */