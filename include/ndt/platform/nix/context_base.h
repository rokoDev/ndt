#ifndef ndt_context_base_h
#define ndt_context_base_h

#include <atomic>

#include "../../nocopyable.h"

namespace ndt
{
template <typename SysWrapperT>
class ContextBase : private NoCopyAble
{
   public:
    ~ContextBase();
    ContextBase() noexcept;
    static int instanceCount() noexcept;

   private:
    static inline std::atomic_int32_t instanceCount_ = 0;
};

template <typename SysWrapperT>
ContextBase<SysWrapperT>::~ContextBase()
{
    if (instanceCount_.fetch_sub(1, std::memory_order_relaxed) == 1)
    {
        return;
    }
}

template <typename SysWrapperT>
ContextBase<SysWrapperT>::ContextBase() noexcept
{
    if (instanceCount_.fetch_add(1, std::memory_order_relaxed))
    {
        return;
    }
}

template <typename SysWrapperT>
int ContextBase<SysWrapperT>::instanceCount() noexcept
{
    return std::atomic_load_explicit(&instanceCount_,
                                     std::memory_order_relaxed);
}

}  // namespace ndt

#endif /* ndt_context_base_h */