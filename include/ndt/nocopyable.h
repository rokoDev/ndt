#ifndef ndt_nocopyable_h
#define ndt_nocopyable_h

namespace ndt
{
struct NoCopyAble
{
    NoCopyAble() = default;
    ~NoCopyAble() = default;
    NoCopyAble(const NoCopyAble&) = delete;
    const NoCopyAble& operator=(const NoCopyAble&) = delete;
};
}  // namespace ndt

#endif /* ndt_nocopyable_h */