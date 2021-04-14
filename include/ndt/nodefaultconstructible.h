#ifndef nodefaultconstructible_h
#define nodefaultconstructible_h

namespace ndt
{
struct NoDefaultConstructible
{
    NoDefaultConstructible() = delete;
};
}  // namespace ndt

#endif /* nodefaultconstructible_h */