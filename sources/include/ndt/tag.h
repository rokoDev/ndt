#ifndef ndt_tag_h
#define ndt_tag_h

namespace ndt
{
template <typename T>
struct tag_t
{
};

template <typename T>
constexpr tag_t<T> tag{};
}  // namespace ndt

#endif /* ndt_tag_h */