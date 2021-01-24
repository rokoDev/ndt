#ifndef ndt_fast_pimpl_h
#define ndt_fast_pimpl_h

#include <type_traits>

template <typename T, std::size_t Size, std::size_t Alignment>
class FastPimpl
{
   public:
    ~FastPimpl() noexcept
    {
        validate<sizeof(T), alignof(T)>();
        get().~T();
    }

    template <typename... Args>
    explicit FastPimpl(Args &&... args) noexcept
    {
        new (&get()) T(std::forward<Args>(args)...);
    }

    FastPimpl(const FastPimpl &other) noexcept { new (&get()) T(other.get()); }

    FastPimpl(const T &other) noexcept { new (&get()) T(other); }

    FastPimpl(FastPimpl &&other) noexcept
    {
        new (&get()) T(std::move(other.get()));
    }

    FastPimpl(T &&other) noexcept { new (&get()) T(std::move(other)); }

    FastPimpl &operator=(const FastPimpl &other) noexcept
    {
        get() = other.get();
        return *this;
    }

    FastPimpl &operator=(const T &other) noexcept
    {
        get() = other;
        return *this;
    }

    FastPimpl &operator=(FastPimpl &&other) noexcept
    {
        std::swap(*this, other);
        // get() = std::move(other.get());
        return *this;
    }

    FastPimpl &operator=(T &&other) noexcept
    {
        get() = std::move(other);
        return *this;
    }

    T *operator->() noexcept { return &get(); }

    const T *operator->() const noexcept { return &get(); }

    template <std::size_t ActualSize, std::size_t ActualAlignment>
    static void validate() noexcept
    {
        static_assert(Size == ActualSize,
                      "Error: Size and sizeof(T) must be equal");
        static_assert(Alignment == ActualAlignment,
                      "Error: Alignment and alignof(T) must be equal");
    }

    T &get() noexcept { return reinterpret_cast<T &>(*this); }

    const T &get() const noexcept { return reinterpret_cast<const T &>(*this); }

   private:
    std::aligned_storage_t<Size, Alignment> _data;
};

#endif /* ndt_fast_pimpl_h */
