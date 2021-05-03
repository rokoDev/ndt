#ifndef ndt_packet_handlers_h
#define ndt_packet_handlers_h

#include <array>
#include <functional>
#include <type_traits>
#include <utility>

#include "bin_rw.h"
#include "utils.h"

namespace ndt
{
template <typename EnumT, typename HanderT,
          std::size_t kArraySize = utils::to_underlying(EnumT::Count) + 1>
class PacketHandlers
{
   public:
    using MethodPtr = void (HanderT::*)(BinReader &) noexcept;
    constexpr PacketHandlers(PacketHandlers &&) noexcept = default;
    constexpr PacketHandlers(
        std::array<std::pair<EnumT, MethodPtr>, kArraySize> aHandlersMap,
        HanderT &aCallingContext) noexcept
        : callingContext_(aCallingContext)
        , handlers_(createHandlersArray(std::move(aHandlersMap)))
    {
        static_assert(std::is_nothrow_invocable_r_v<void, MethodPtr, HanderT *,
                                                    BinReader &>,
                      "invalid parameters");
    }
    void operator()(const EnumT aValue, BinReader &aReader) const noexcept
    {
        std::invoke(handlers_[utils::to_underlying(aValue)], callingContext_,
                    aReader);
    }

   private:
    constexpr auto createHandlersArray(
        std::array<std::pair<EnumT, MethodPtr>, kArraySize>
            aHandlersMap) noexcept
    {
        static_assert(std::is_enum_v<EnumT>, "EnumT must be enum");
        static_assert(utils::to_underlying(EnumT::Count) ==
                          utils::to_underlying(EnumT::Error),
                      "(EnumT::Count == EnumT::Error) must be true");
        std::array<MethodPtr, kArraySize> result{};
        for (auto &item: aHandlersMap)
        {
            result[utils::to_underlying(item.first)] = item.second;
        }
        return result;
    };

    HanderT &callingContext_;
    std::array<MethodPtr, kArraySize> handlers_;
};
}  // namespace ndt

#endif /* ndt_packet_handlers_h */