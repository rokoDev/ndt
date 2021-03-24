#ifndef ndt_context_h
#define ndt_context_h

#include <array>
#include <atomic>
#include <functional>
#include <system_error>
#include <type_traits>
#include <variant>

#include "buffer.h"
#include "common.h"
#include "executor_select.h"
#include "nocopyable.h"
#include "system_wrappers.h"

namespace ndt
{
inline const char *kContextErrorCategoryCStr = "ndt_context_ec";
inline const std::string kContextWinSockDLLNotFoundDescr =
    "Could not find a usable WinSock DLL.";
inline const std::string kExecutorOverloadDescr =
    "No free space for adding new descriptors left.";

enum class eContextErrorCode
{
    kSuccess = 0,
    kWinSockDLLNotFound,
    kExecutorOverload
};

class ContextErrorCategory : public std::error_category
{
   public:
    virtual const char *name() const noexcept override final;
    virtual std::string message(int c) const override final;
    virtual std::error_condition default_error_condition(
        int c) const noexcept override final;
};

inline const ContextErrorCategory &getContextErrorCategory()
{
    static ContextErrorCategory c;
    return c;
}

inline std::error_code make_error_code(eContextErrorCode e)
{
    return {static_cast<int>(e), getContextErrorCategory()};
}

template <typename FlagsT, typename SFuncsT>
class Socket;

class ContextBase : private Nocopyable
{
   public:
    ~ContextBase();
    ContextBase() noexcept;
    static int instanceCount() noexcept;

   private:
    void startUp(std::error_code &aEc) noexcept;
    void cleanUp(std::error_code &aEc) noexcept;
    void logAndExit(std::error_code &aEc, const char *aMsg) noexcept;

    [[maybe_unused]] static inline std::atomic_int32_t instanceCount_ = 0;
};

class Context final : public ContextBase
{
   public:
    ~Context();
    Context();
    void run();
    void stop();
    ExecutorSelect &executor() noexcept;

   private:
    bool isRunning_ = false;
    ExecutorSelect executor_;
};
}  // namespace ndt

namespace std
{
// Tell the C++ 11 STL metaprogramming that enum ndt::eAddressErrorCode
// is registered with the standard error code system
template <>
struct is_error_code_enum<ndt::eContextErrorCode> : true_type
{
};
}  // namespace std

#endif /* ndt_context_h */