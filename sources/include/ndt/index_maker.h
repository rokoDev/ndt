#ifndef ndt_index_maker_h
#define ndt_index_maker_h

#include <limits>
#include <type_traits>

namespace ndt
{
template <typename T, T maxIndex = std::numeric_limits<T>::max()>
class IndexMaker
{
   public:
    T next() noexcept
    {
        static_assert(std::is_unsigned_v<T>,
                      "T must be unsigned integral type");
        static_assert(maxIndex > 0, "maxIndex must be great than zero");
        T result = nextIndex_;
        ++nextIndex_;
        if constexpr (maxIndex < std::numeric_limits<T>::max())
        {
            nextIndex_ %= maxIndex + 1;
        }
        return result;
    }

    constexpr T max() const noexcept { return maxIndex; }

   private:
    T nextIndex_ = 0;
};
}  // namespace ndt

#endif /* ndt_index_maker_h */