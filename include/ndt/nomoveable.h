#ifndef ndt_nomoveable_h
#define ndt_nomoveable_h

namespace ndt
{
struct NoMoveAble
{
    NoMoveAble() = default;
    ~NoMoveAble() = default;
    NoMoveAble(NoMoveAble&&) = delete;
    NoMoveAble& operator=(NoMoveAble&&) = delete;
};
}  // namespace ndt

#endif /* ndt_nomoveable_h */