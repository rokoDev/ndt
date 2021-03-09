#ifndef ndt_context_h
#define ndt_context_h

#include <atomic>
#include <system_error>

#include "ndt/nocopyable.h"

namespace ndt
{
inline const char *kContextErrorCategoryCStr = "ndt_context_ec";
inline const std::string kContextWinSockDLLNotFoundDescr =
    "Could not find a usable WinSock DLL.";

enum class eContextErrorCode
{
    kSuccess = 0,
    kWinSockDLLNotFound
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

class Context : private Nocopyable
{
   public:
    ~Context();
    Context();
    Context(std::error_code &aEc);
    static int instanceCount() noexcept
    {
        return std::atomic_load_explicit(&instanceCount_,
                                         std::memory_order_relaxed);
    }

   private:
    void startUp(std::error_code &aEc) noexcept;
    void cleanUp(std::error_code &aEc) noexcept;

    [[maybe_unused]] static inline std::atomic_int32_t instanceCount_ = 0;
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