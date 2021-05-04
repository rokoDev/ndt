#include <fmt/core.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <chrono>

#include "ndt/address.h"
#include "ndt/context.h"
#include "ndt/event_handler_select.h"
#include "ndt/packet_handlers.h"
#include "ndt/udp.h"

enum class eClientPacket : uint8_t
{
    kJoin,
    kLeave,
    kInput,
    kRequestTime,
    Count,
    Error = Count
};

using namespace std::chrono_literals;

class Server : public ndt::HandlerSelect<ndt::UDP::Socket, Server, ndt::System>
{
    friend class ndt::HandlerSelect<ndt::UDP::Socket, Server, ndt::System>;
    friend class ndt::CheckMethod_readHandlerImpl<Server, void,
                                                  ndt::UDP::Socket &>;
    friend class ndt::CheckMethod_writeHandlerImpl<Server, void,
                                                   ndt::UDP::Socket &>;
    friend class ndt::CheckMethod_exceptionConditionHandlerImpl<
        Server, void, ndt::UDP::Socket &>;
    using microseconds = std::chrono::microseconds;

   public:
    ~Server()
    {
        std::error_code err;
        socket_.close(err);
        if (err)
        {
            handleError(err);
        }
    }

    Server(ndt::Context<ndt::System> &aContext)
        : HandlerSelect<ndt::UDP::Socket, Server, ndt::System>(aContext)
        , socket_(context_, ndt::UDP::V4(), port_)
        , handlers_(
              {{{{eClientPacket::kInput, &Server::handleInput},
                 {eClientPacket::kLeave, &Server::handleLeave},
                 {eClientPacket::kJoin, &Server::handleJoin},
                 {eClientPacket::kRequestTime, &Server::handleRequestTime},
                 {eClientPacket::Error, &Server::handleInvalidPacketType}}},
               *this})
    {
        context_.executor().setTimeout(
            {0, static_cast<decltype(timeval::tv_usec)>(timeStep_.count())});
        context_.executor().setTimeoutHandler([this]() { handleTimeout(); });

        context_.executor().setErrorHandler(
            [this](std::error_code aError) { handleError(aError); });
        socket_.handler(this);
    }

   private:
    void readHandlerImpl(ndt::UDP::Socket &s)
    {
        std::error_code err;
        ndt::Buffer buf(recvBuffer_);
        s.recvFrom(buf, sender_, err);
        if (!err)
        {
            ndt::BinReader reader((ndt::CBuffer(buf)));
            const auto packetType = reader.get<eClientPacket>();
            handlers_(packetType, reader);
        }
        else
        {
            handleError(err);
        }
    }

    void writeHandlerImpl(ndt::UDP::Socket &s)
    {
        std::error_code err;
        ndt::CBuffer buf(sendBuffer_);

        s.sendTo(sender_, buf, err);
        if (!err)
        {
        }
        else
        {
            handleError(err);
        }
    }

    void handleJoin([[maybe_unused]] ndt::BinReader &aReader) noexcept
    {
        fmt::print("eClientPacket::kJoin received\n");
    }

    void handleLeave([[maybe_unused]] ndt::BinReader &aReader) noexcept
    {
        fmt::print("eClientPacket::kLeave received\n");
    }

    void handleInput([[maybe_unused]] ndt::BinReader &aReader) noexcept
    {
        fmt::print("eClientPacket::kInput received\n");
    }

    void handleRequestTime([[maybe_unused]] ndt::BinReader &aReader) noexcept
    {
        fmt::print("eClientPacket::kRequestTime received\n");
    }

    void handleInvalidPacketType([
        [maybe_unused]] ndt::BinReader &aReader) noexcept
    {
        fmt::print("a packet with invalid type received\n");
    }

   private:
    void handleTimeout() { fmt::print("SELECT timeout\n"); }

    void handleError(std::error_code aError)
    {
        fmt::print("error: {}\nmsg: {}\n", aError.value(), aError.message());
        exit(aError.value());
    }

    microseconds timeStep_ = 200ms;
    static constexpr uint16_t port_ = 4123;
    ndt::UDP::Socket socket_;
    ndt::Address sender_;
    ndt::PacketHandlers<eClientPacket, Server> handlers_;
    char recvBuffer_[1024];
    char sendBuffer_[1024];
};

TEST(PacketHandler, LackOfSpaceWrite)
{
    ndt::Context<ndt::System> context;
    Server server(context);
    // context.run();
    ASSERT_EQ(0, 0);
}