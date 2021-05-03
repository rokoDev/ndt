#ifndef ndt_useful_base_types_h
#define ndt_useful_base_types_h

namespace ndt
{
struct NoCopyAble
{
    NoCopyAble() = default;
    ~NoCopyAble() = default;
    NoCopyAble(const NoCopyAble&) = delete;
    const NoCopyAble& operator=(const NoCopyAble&) = delete;
};

struct NoDefaultConstructible
{
    NoDefaultConstructible() = delete;
};

struct NoMoveAble
{
    NoMoveAble() = default;
    ~NoMoveAble() = default;
    NoMoveAble(NoMoveAble&&) = delete;
    NoMoveAble& operator=(NoMoveAble&&) = delete;
};

struct NoCopyMoveDefConstructible
    : public NoCopyAble
    , public NoMoveAble
    , public NoDefaultConstructible
{
};
}  // namespace ndt

#endif /* ndt_useful_base_types_h */