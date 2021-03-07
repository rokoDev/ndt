#ifndef ndt_nocopyable_h
#define ndt_nocopyable_h

namespace ndt
{
class Nocopyable
{
   protected:
    Nocopyable() = default;
    ~Nocopyable() = default;
    Nocopyable(const Nocopyable&) = delete;
    const Nocopyable& operator=(const Nocopyable&) = delete;
};
}  // namespace ndt

#endif /* ndt_nocopyable_h */