#ifndef ndt_context_h
#define ndt_context_h

#include "executor_select.h"

#ifdef _WIN32
#include "platform/win/context_base.h"
#else
#include "platform/nix/context_base.h"
#endif

namespace ndt
{
template <typename SysWrapperT>
class Context final : public ContextBase<SysWrapperT>
{
   public:
    ~Context();
    Context();
    void run();
    void stop();
    ExecutorSelect<SysWrapperT> &executor() noexcept;

   private:
    bool isRunning_ = false;
    ExecutorSelect<SysWrapperT> executor_;
};

template <typename SysWrapperT>
Context<SysWrapperT>::~Context() = default;

template <typename SysWrapperT>
Context<SysWrapperT>::Context() = default;

template <typename SysWrapperT>
void Context<SysWrapperT>::run()
{
    isRunning_ = true;
    do
    {
        executor_();
    } while (isRunning_);
}

template <typename SysWrapperT>
void Context<SysWrapperT>::stop()
{
    isRunning_ = false;
}

template <typename SysWrapperT>
ExecutorSelect<SysWrapperT> &Context<SysWrapperT>::executor() noexcept
{
    return executor_;
}
}  // namespace ndt

#endif /* ndt_context_h */