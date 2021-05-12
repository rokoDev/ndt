#include <fmt/core.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <chrono>
#include <thread>

//#include "ndt/address.h"
//#include "ndt/context.h"
//#include "ndt/event_handler_select.h"
//#include "ndt/packet_handlers.h"
//#include "ndt/udp.h"
//#include "packet_info.h"
//
// void printErrorCode(const std::error_code &aError)
//{
//    fmt::print("error: {}\nreason: {}\n", aError.value(), aError.message());
//}
//
// void printErrorCodeAndExit(const std::error_code &aError)
//{
//    printErrorCode(aError);
//    exit(aError.value());
//}
//
// void printSysError(const int aValue)
//{
//    const auto err = std::error_code(aValue, std::system_category());
//    printErrorCode(err);
//}
//
// void printSysErrorAndExit(const int aValue)
//{
//    printSysError(aValue);
//    exit(aValue);
//}
//
// using namespace std::chrono_literals;
//
// using ::testing::_;
// using ::testing::InSequence;
// using ::testing::Return;
//
// constexpr ndt::sock_t kValidSockId{1};
// constexpr int kBindSucceeded = 0;
// constexpr int kCloseSucceeded = 0;
// constexpr ndt::sdlen_t kRecvfromSucceeded = 1;
// constexpr ndt::sdlen_t kSendtoSucceeded = 1;
// constexpr ndt::salen_t kV4Size = sizeof(sockaddr_in);
// constexpr ndt::salen_t kV6Size = sizeof(sockaddr_in6);
//
// using PacketInfo =
//    ndt::Packet<std::chrono::microseconds, ndt::Address, ndt::Buffer>;
//
// template <std::size_t kBufSize>
// class NetSimulator
//{
//   public:
//    class PacketInfo
//    {
//       public:
//        PacketInfo() : delay_(0), sender_(), content_(rawData_) {}
//
//        [[nodiscard]] inline const std::chrono::microseconds &delay()
//            const noexcept
//        {
//            return delay_;
//        }
//
//        inline void delay(ndt::BinReader &aReader) noexcept
//        {
//            delay_ = std::chrono::microseconds(aReader.get<int32_t>());
//        }
//
//        [[nodiscard]] inline const ndt::Address &sender() const noexcept
//        {
//            return sender_;
//        }
//
//        [[nodiscard]] inline std::error_code sender(
//            ndt::BinReader &aReader) noexcept
//        {
//            return sender_.deserialize(aReader);
//        }
//
//        [[nodiscard]] inline ndt::CBuffer content() const noexcept
//        {
//            return ndt::CBuffer(content_);
//        }
//
//        inline void content(ndt::BinReader &aReader) noexcept
//        {
//            aReader.get(content_);
//        }
//
//        std::error_code deserialize(ndt::BinReader &aReader) noexcept
//        {
//            std::error_code retVal;
//            delay(aReader);
//            retVal = sender(aReader);
//            content(aReader);
//            return retVal;
//        }
//
//       private:
//        char rawData_[2048];
//        std::chrono::microseconds delay_;
//        ndt::Address sender_;
//        ndt::Buffer content_;
//    };
//
//    constexpr NetSimulator()
//        : buf_(rawBuf_), writer_(buf_), reader_(ndt::CBuffer(buf_))
//    {
//    }
//
//    template <typename PacketT>
//    std::error_code addPacket(const std::chrono::microseconds &aDelay,
//                              const ndt::Address &aAddress,
//                              const PacketT &aPacket) noexcept
//    {
//        std::error_code retVal;
//        delay(aDelay);
//        retVal = aAddress.serialize(writer_);
//        if (retVal)
//        {
//            return retVal;
//        }
//        const uint16_t packetSize = aPacket.minBitSize();
//        writer_.add<uint16_t>(packetSize);
//        const auto err = aPacket.serialize(writer_);
//        if (err)
//        {
//            printErrorCodeAndExit(err);
//        }
//        return retVal;
//    }
//
//    template <typename PacketT>
//    std::error_code getPacket(std::chrono::microseconds &aDelay,
//                              ndt::Address &aAddress, PacketT &aPacket)
//                              noexcept
//    {
//        std::error_code retVal;
//        aDelay = delay();
//        retVal = aAddress.deserialize(reader_);
//        if (retVal)
//        {
//            return retVal;
//        }
//        reader_.get<uint16_t>();
//        const auto err = aPacket.deserialize(reader_);
//        if (err)
//        {
//            printErrorCodeAndExit(err);
//        }
//        return retVal;
//    }
//
//    const PacketInfo &deserializeNextPacket() noexcept
//    {
//        recvInfo_.deserialize(reader_);
//        return recvInfo_;
//    }
//
//    const PacketInfo &recvInfo() const noexcept { return recvInfo_; }
//
//    const PacketInfo &sendInfo() const noexcept { return sendInfo_; }
//
//    [[nodiscard]] std::chrono::microseconds delay() noexcept
//    {
//        return std::chrono::microseconds(reader_.get<int32_t>());
//    }
//
//    void delay(const std::chrono::microseconds &aDelay) noexcept
//    {
//        writer_.add<int32_t>(aDelay.count());
//    }
//
//   private:
//    char rawBuf_[kBufSize];
//    ndt::Buffer buf_;
//    ndt::BinWriter writer_;
//    ndt::BinReader reader_;
//
//    PacketInfo recvInfo_;
//    PacketInfo sendInfo_;
//};
//
// class MockDetails
//{
//   public:
//    MOCK_METHOD(int, bind,
//                (ndt::sock_t, const struct sockaddr *, ndt::salen_t));
//    MOCK_METHOD(ndt::sdlen_t, recvfrom,
//                (ndt::sock_t, ndt::bufp_t, ndt::dlen_t, int, struct sockaddr
//                *,
//                 ndt::salen_t *));
//    MOCK_METHOD(ndt::sdlen_t, sendto,
//                (ndt::sock_t, ndt::cbufp_t, ndt::dlen_t, int,
//                 const struct sockaddr *, ndt::salen_t));
//    MOCK_METHOD(ndt::sock_t, socket, (int, int, int));
//    MOCK_METHOD(int, close, (ndt::sock_t));
//
//    void expectSocketFailed(const int family)
//    {
//        EXPECT_CALL(*this, socket(family, SOCK_DGRAM, IPPROTO_UDP))
//            .WillOnce(Return(ndt::kInvalidSocket));
//    }
//
//    void expectSocketSucceded(const int family)
//    {
//        EXPECT_CALL(*this, socket(family, SOCK_DGRAM, IPPROTO_UDP))
//            .WillOnce(Return(kValidSockId));
//    }
//
//    void expectBindFailed(const ndt::salen_t sockaddrSize)
//    {
//        EXPECT_CALL(*this, bind(kValidSockId, _, sockaddrSize))
//            .WillOnce(Return(ndt::kSocketError));
//    }
//
//    void expectBindSucceded(const ndt::salen_t sockaddrSize)
//    {
//        EXPECT_CALL(*this, bind(kValidSockId, _, sockaddrSize))
//            .WillOnce(Return(kBindSucceeded));
//    }
//
//    void expectCloseFailed()
//    {
//        EXPECT_CALL(*this, close(_)).WillOnce(Return(ndt::kSocketError));
//    }
//
//    void expectCloseSucceded()
//    {
//        EXPECT_CALL(*this, close(_)).WillOnce(Return(kCloseSucceeded));
//    }
//
//    void expectSendToSucceded()
//    {
//        EXPECT_CALL(*this, sendto(_, _, _, _, _, _))
//            .WillOnce(Return(kSendtoSucceeded));
//    }
//
//    void expectSendToFailed()
//    {
//        EXPECT_CALL(*this, sendto(_, _, _, _, _, _))
//            .WillOnce(Return(ndt::kSocketError));
//    }
//
//    void expectRecvFromSucceded()
//    {
//        EXPECT_CALL(*this, recvfrom(_, _, _, _, _, _))
//            .WillOnce(Return(kRecvfromSucceeded));
//    }
//
//    void expectRecvFromFailed()
//    {
//        EXPECT_CALL(*this, recvfrom(_, _, _, _, _, _))
//            .WillOnce(Return(ndt::kSocketError));
//    }
//
//    NetSimulator<2048> netSim;
//    ndt::SteadyTP startTP;
//    ndt::SteadyTP recvTP;
//    ndt::SteadyTP sendTP;
//    std::chrono::microseconds elapsed;
//};
//
// class SysMockup : public ::testing::Test
//{
//   public:
//    SysMockup() = default;
//    SysMockup(const SysMockup &) = delete;
//    SysMockup &operator=(const SysMockup &) = delete;
//    SysMockup(SysMockup &&) = delete;
//    SysMockup &operator=(SysMockup &&) = delete;
//
//    inline static int lastErrorCode() { return 10; }
//
//    static int bind(ndt::sock_t sockfd, const struct sockaddr *addr,
//                    ndt::salen_t addrlen)
//    {
//        return mDetails->bind(sockfd, addr, addrlen);
//    }
//
//    static ndt::sdlen_t recvfrom(ndt::sock_t sockfd, ndt::bufp_t buf,
//                                 ndt::dlen_t len, int flags,
//                                 struct sockaddr *src_addr,
//                                 ndt::salen_t *addrlen)
//    {
//        mDetails->recvTP = std::chrono::steady_clock::now();
//        mDetails->elapsed =
//            std::chrono::duration_cast<std::chrono::microseconds>(
//                mDetails->recvTP - mDetails->startTP);
//
//        const auto &packetInfo = mDetails->netSim.recvInfo();
//
//        // copy address
//        const ndt::Address &sender = packetInfo.sender();
//        std::memcpy(src_addr, sender.nativeDataConst(), sender.capacity());
//        *addrlen = sender.capacity();
//
//        // copy packet content
//        ndt::Buffer buffer(buf, len);
//        auto packetContent = packetInfo.content();
//        const auto err = buffer.copyFrom(packetContent);
//        if (err)
//        {
//            printErrorCodeAndExit(err);
//        }
//
//        return buffer.size<ndt::sdlen_t>();
//    }
//
//    static ndt::sdlen_t sendto(ndt::sock_t sockfd, ndt::cbufp_t buf,
//                               ndt::dlen_t len, int flags,
//                               const struct sockaddr *dest_addr,
//                               ndt::salen_t addrlen)
//    {
//        mDetails->sendTP = std::chrono::steady_clock::now();
//        return mDetails->sendto(sockfd, buf, len, flags, dest_addr, addrlen);
//    }
//
//    static ndt::sock_t socket(int socket_family, int socket_type, int
//    protocol)
//    {
//        return mDetails->socket(socket_family, socket_type, protocol);
//    }
//
//    static int close(ndt::sock_t fd) { return mDetails->close(fd); }
//
//    [[nodiscard]] static int select(int nfds, fd_set *readfds, fd_set
//    *writefds,
//                                    fd_set *exceptfds,
//                                    struct timeval *timeout) noexcept
//    {
//        const auto &packetInfo = mDetails->netSim.deserializeNextPacket();
//        mDetails->startTP = std::chrono::steady_clock::now();
//        std::this_thread::sleep_for(packetInfo.delay());
//        if (FD_ISSET(kValidSockId, readfds))
//        {
//            FD_ZERO(readfds);
//            FD_ZERO(writefds);
//            FD_ZERO(exceptfds);
//            FD_SET(kValidSockId, readfds);
//        }
//        return 1;
//    }
//
//#if _WIN32
//    static int ioctlsocket(ndt::sock_t s, long cmd, u_long *argp) noexcept
//    {
//        return ndt::System::ioctlsocket(s, cmd, argp);
//    }
//
//    static int WSAStartup(WORD wVersionRequired, LPWSADATA lpWSAData) noexcept
//    {
//        return ndt::System::WSAStartup(wVersionRequired, lpWSAData);
//    }
//
//    static int WSACleanup() noexcept { return ndt::System::WSACleanup(); }
//#endif
//
//    static std::unique_ptr<MockDetails> mDetails;
//
//   protected:
//    static void SetUpTestSuite() { mDetails = std::make_unique<MockDetails>();
//    } static void TearDownTestSuite() { mDetails = nullptr; }
//};
//
// std::unique_ptr<MockDetails> SysMockup::mDetails;
//
// using SocketT = ndt::Socket<ndt::UDP, SysMockup>;
// class Server : public ndt::HandlerSelect<SocketT, Server, SysMockup>
//{
//    friend class ndt::HandlerSelect<SocketT, Server, SysMockup>;
//    friend class ndt::CheckMethod_readHandlerImpl<Server, void, SocketT &>;
//    friend class ndt::CheckMethod_writeHandlerImpl<Server, void, SocketT &>;
//    friend class ndt::CheckMethod_exceptionConditionHandlerImpl<Server, void,
//                                                                SocketT &>;
//    using microseconds = std::chrono::microseconds;
//
//   public:
//    ~Server()
//    {
//        std::error_code err;
//        socket_.close(err);
//        if (err)
//        {
//            handleError(err);
//        }
//    }
//
//    Server(ndt::Context<SysMockup> &aContext)
//        : HandlerSelect<SocketT, Server, SysMockup>(aContext)
//        , socket_(context_, ndt::UDP::V4(), port_)
//        , handlers_({{{{client::ePacketType::kInput, &Server::handleInput},
//                       {client::ePacketType::kLeave, &Server::handleLeave},
//                       {client::ePacketType::kJoin, &Server::handleJoin},
//                       {client::ePacketType::kRequestTime,
//                        &Server::handleRequestTime},
//                       {client::ePacketType::Error,
//                        &Server::handleInvalidPacketType}}},
//                     *this})
//    {
//        context_.executor().setTimeout(
//            {0, static_cast<decltype(timeval::tv_usec)>(timeStep_.count())});
//        context_.executor().setTimeoutHandler([this]() { handleTimeout(); });
//
//        context_.executor().setErrorHandler(
//            [this](std::error_code aError) { handleError(aError); });
//        socket_.handler(this);
//    }
//
//    Server(ndt::Context<SysMockup> &aContext,
//           ndt::PacketHandlers<client::ePacketType, Server> &aHandlers);
//
//   private:
//    void readHandlerImpl(SocketT &s)
//    {
//        std::error_code err;
//        ndt::Buffer buf(recvBuffer_);
//        s.recvFrom(buf, sender_, err);
//        if (!err)
//        {
//            ndt::BinReader reader((ndt::CBuffer(buf)));
//            const auto packetType = reader.get<client::ePacketType>();
//            handlers_(packetType, reader);
//        }
//        else
//        {
//            handleError(err);
//        }
//    }
//
//    void writeHandlerImpl(SocketT &s)
//    {
//        std::error_code err;
//        ndt::CBuffer buf(sendBuffer_);
//
//        s.sendTo(sender_, buf, err);
//        if (!err)
//        {
//        }
//        else
//        {
//            handleError(err);
//        }
//    }
//
//    void handleJoin([[maybe_unused]] ndt::BinReader &aReader) noexcept
//    {
//        fmt::print("eClientPacket::kJoin received\n");
//        using namespace client;
//        JoinPacket packet =
//        PacketFactory<ePacketType::kJoin>::create(aReader);
//    }
//
//    void handleLeave([[maybe_unused]] ndt::BinReader &aReader) noexcept
//    {
//        fmt::print("eClientPacket::kLeave received\n");
//    }
//
//    void handleInput([[maybe_unused]] ndt::BinReader &aReader) noexcept
//    {
//        fmt::print("eClientPacket::kInput received\n");
//    }
//
//    void handleRequestTime([[maybe_unused]] ndt::BinReader &aReader) noexcept
//    {
//        fmt::print("eClientPacket::kRequestTime received\n");
//    }
//
//    void handleInvalidPacketType([
//        [maybe_unused]] ndt::BinReader &aReader) noexcept
//    {
//        fmt::print("a packet with invalid type received\n");
//    }
//
//   private:
//    void handleTimeout() { fmt::print("SELECT timeout\n"); }
//
//    void handleError(std::error_code aError) { printErrorCodeAndExit(aError);
//    }
//
//    microseconds tickElapsed_ = 0ms;
//    microseconds timeStep_ = 200ms;
//    static constexpr uint16_t port_ = 4123;
//    SocketT socket_;
//    ndt::Address sender_;
//    ndt::PacketHandlers<client::ePacketType, Server> handlers_;
//    char recvBuffer_[1024];
//    char sendBuffer_[1024];
//};
//
// TEST_F(SysMockup, SimulateJoinReceived)
//{
//    using namespace std::chrono;
//
//    client::JoinPacket somePacket(
//        ndt::TypeId<client::ePacketType>(client::ePacketType::kJoin),
//        {ndt::PacketId(1234)});
//    ndt::Address sender(ndt::kIPv4Loopback, 1234);
//    mDetails->netSim.addPacket(50ms, sender, somePacket);
//
//    ndt::Context<SysMockup> ctx;
//    Server server(ctx);
//
//    for (int i = 0; i < 1; ++i)
//    {
//        ctx.executor()();
//
//        // assert results
//    }
//}
//
// TEST(NetSimulatorTest, RWJoin)
//{
//    using namespace std::chrono;
//    client::JoinPacket somePacket(
//        ndt::TypeId<client::ePacketType>(client::ePacketType::kJoin),
//        {ndt::PacketId(1234)});
//
//    NetSimulator<512> netSim;
//    ndt::Address sender(ndt::kIPv4Loopback, 1234);
//    netSim.addPacket(50ms, sender, somePacket);
//
//    microseconds delay(0);
//    client::JoinPacket unpackedPacket;
//    ndt::Address unpackedSender;
//    netSim.getPacket(delay, unpackedSender, unpackedPacket);
//
//    ASSERT_EQ(somePacket.typeId(), unpackedPacket.typeId());
//    ASSERT_EQ(somePacket.packetId(), unpackedPacket.packetId());
//    ASSERT_EQ(delay, 50ms);
//    ASSERT_EQ(sender, unpackedSender);
//}
//
// TEST(NetSimulatorTest, RWLeave)
//{
//    using namespace std::chrono;
//    client::LeavePacket somePacket(
//        ndt::TypeId<client::ePacketType>(client::ePacketType::kLeave),
//        {{ndt::PacketId(1234)}, {ndt::UserId(5)}});
//
//    NetSimulator<512> netSim;
//    ndt::Address sender(ndt::kIPv4Loopback, 1234);
//    netSim.addPacket(50ms, sender, somePacket);
//
//    microseconds delay(0);
//    client::LeavePacket unpackedPacket;
//    ndt::Address unpackedSender;
//    netSim.getPacket(delay, unpackedSender, unpackedPacket);
//
//    ASSERT_EQ(somePacket.typeId(), unpackedPacket.typeId());
//    ASSERT_EQ(somePacket.packetId(), unpackedPacket.packetId());
//    ASSERT_EQ(somePacket.userId(), unpackedPacket.userId());
//    ASSERT_EQ(delay, 50ms);
//    ASSERT_EQ(sender, unpackedSender);
//}
//
// TEST(NetSimulatorTest, RWInput)
//{
//    using namespace std::chrono;
//    client::InputPacket somePacket(
//        ndt::TypeId<client::ePacketType>(client::ePacketType::kInput),
//        {{ndt::PacketId(1234)}, {ndt::UserId(5)}, {client::UserInput(231)}});
//
//    NetSimulator<512> netSim;
//    ndt::Address sender(ndt::kIPv4Loopback, 1234);
//    netSim.addPacket(50ms, sender, somePacket);
//
//    microseconds delay(0);
//    client::InputPacket unpackedPacket;
//    ndt::Address unpackedSender;
//    netSim.getPacket(delay, unpackedSender, unpackedPacket);
//
//    ASSERT_EQ(somePacket.typeId(), unpackedPacket.typeId());
//    ASSERT_EQ(somePacket.packetId(), unpackedPacket.packetId());
//    ASSERT_EQ(somePacket.userId(), unpackedPacket.userId());
//    ASSERT_EQ(somePacket.actionKey(), unpackedPacket.actionKey());
//    ASSERT_EQ(delay, 50ms);
//    ASSERT_EQ(sender, unpackedSender);
//}
//
// TEST(NetSimulatorTest, RWRequestTime)
//{
//    using namespace std::chrono;
//    client::RequestTimePacket somePacket(
//        ndt::TypeId<client::ePacketType>(client::ePacketType::kInput),
//        {{ndt::PacketId(1234)}, {ndt::UserId(5)}});
//
//    NetSimulator<512> netSim;
//    ndt::Address sender(ndt::kIPv4Loopback, 1234);
//    netSim.addPacket(55ms, sender, somePacket);
//
//    microseconds delay(0);
//    client::RequestTimePacket unpackedPacket;
//    ndt::Address unpackedSender;
//    netSim.getPacket(delay, unpackedSender, unpackedPacket);
//
//    ASSERT_EQ(somePacket.typeId(), unpackedPacket.typeId());
//    ASSERT_EQ(somePacket.packetId(), unpackedPacket.packetId());
//    ASSERT_EQ(somePacket.userId(), unpackedPacket.userId());
//    ASSERT_EQ(delay, 55ms);
//    ASSERT_EQ(sender, unpackedSender);
//}
//
// TEST(NetSimulatorTest, RWMixAll)
//{
//    using namespace std::chrono;
//    NetSimulator<1024> netSim;
//    ndt::Address sender(ndt::kIPv4Loopback, 1234);
//    ndt::Address unpackedSender;
//    {
//        client::JoinPacket somePacket(
//            ndt::TypeId<client::ePacketType>(client::ePacketType::kJoin),
//            {ndt::PacketId(1234)});
//
//        netSim.addPacket(50ms, sender, somePacket);
//
//        microseconds delay(0);
//        client::JoinPacket unpackedPacket;
//        netSim.getPacket(delay, unpackedSender, unpackedPacket);
//
//        ASSERT_EQ(somePacket.typeId(), unpackedPacket.typeId());
//        ASSERT_EQ(somePacket.packetId(), unpackedPacket.packetId());
//        ASSERT_EQ(delay, 50ms);
//        ASSERT_EQ(sender, unpackedSender);
//    }
//
//    {
//        client::LeavePacket somePacket(
//            ndt::TypeId<client::ePacketType>(client::ePacketType::kLeave),
//            {{ndt::PacketId(1234)}, {ndt::UserId(5)}});
//
//        netSim.addPacket(50ms, sender, somePacket);
//
//        microseconds delay(0);
//        client::LeavePacket unpackedPacket;
//        netSim.getPacket(delay, unpackedSender, unpackedPacket);
//
//        ASSERT_EQ(somePacket.typeId(), unpackedPacket.typeId());
//        ASSERT_EQ(somePacket.packetId(), unpackedPacket.packetId());
//        ASSERT_EQ(somePacket.userId(), unpackedPacket.userId());
//        ASSERT_EQ(delay, 50ms);
//        ASSERT_EQ(sender, unpackedSender);
//    }
//
//    {
//        client::InputPacket somePacket(
//            ndt::TypeId<client::ePacketType>(client::ePacketType::kInput),
//            {{ndt::PacketId(1234)},
//             {ndt::UserId(5)},
//             {client::UserInput(231)}});
//
//        netSim.addPacket(50ms, sender, somePacket);
//
//        microseconds delay(0);
//        client::InputPacket unpackedPacket;
//        netSim.getPacket(delay, unpackedSender, unpackedPacket);
//
//        ASSERT_EQ(somePacket.typeId(), unpackedPacket.typeId());
//        ASSERT_EQ(somePacket.packetId(), unpackedPacket.packetId());
//        ASSERT_EQ(somePacket.userId(), unpackedPacket.userId());
//        ASSERT_EQ(somePacket.actionKey(), unpackedPacket.actionKey());
//        ASSERT_EQ(delay, 50ms);
//        ASSERT_EQ(sender, unpackedSender);
//    }
//
//    {
//        client::RequestTimePacket somePacket(
//            ndt::TypeId<client::ePacketType>(client::ePacketType::kInput),
//            {{ndt::PacketId(1234)}, {ndt::UserId(5)}});
//
//        netSim.addPacket(55ms, sender, somePacket);
//
//        microseconds delay(0);
//        client::RequestTimePacket unpackedPacket;
//        netSim.getPacket(delay, unpackedSender, unpackedPacket);
//
//        ASSERT_EQ(somePacket.typeId(), unpackedPacket.typeId());
//        ASSERT_EQ(somePacket.packetId(), unpackedPacket.packetId());
//        ASSERT_EQ(somePacket.userId(), unpackedPacket.userId());
//        ASSERT_EQ(delay, 55ms);
//        ASSERT_EQ(sender, unpackedSender);
//    }
//}