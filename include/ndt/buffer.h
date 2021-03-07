#ifndef ndt_buffer_h
#define ndt_buffer_h

#include <cstddef>

namespace ndt
{
class buffer
{
   public:
    buffer(void* aData, std::size_t aSize) noexcept : data_(aData), size_(aSize)
    {
    }
    buffer(char* aData, std::size_t aSize) noexcept
        : data_(static_cast<void*>(aData)), size_(aSize)
    {
    }

    template <size_t N>
    explicit buffer(char (&aData)[N]) : data_(aData), size_(N)
    {
    }

    void* voidPtr() const noexcept { return data_; }
    char* charPtr() const noexcept { return static_cast<char*>(data_); }
    std::size_t size() const noexcept { return size_; }

   private:
    void* data_ = nullptr;
    std::size_t size_ = 0;
};

}  // namespace ndt

#endif /* ndt_buffer_h */