#include <fmt/core.h>

#include <algorithm>
#include <array>

#include "Address.h"
#include "NetException.h"
#include "Socket.h"
#include "UDP.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

using ::testing::_;
using ::testing::InSequence;
using ::testing::Return;

const int kValidSockId = 1;
const socklen_t kV4Size = sizeof(sockaddr_in);
const socklen_t kV6Size = sizeof(sockaddr_in6);

class MockDetails
{
   public:
    MOCK_METHOD(int, bind, (int, const struct sockaddr *, socklen_t));
    MOCK_METHOD(ssize_t, recvfrom,
                (int, void *, size_t, int, struct sockaddr *, socklen_t *));
    MOCK_METHOD(ssize_t, sendto,
                (int, const void *, size_t, int, const struct sockaddr *,
                 socklen_t));
    MOCK_METHOD(int, socket, (int, int, int));
    MOCK_METHOD(int, close, (int));

    void expectSocketFailed(const int family)
    {
        EXPECT_CALL(*this, socket(family, SOCK_DGRAM, IPPROTO_UDP))
            .WillOnce(Return(net::OPERATION_FAILED));
    }

    void expectSocketSucceded(const int family)
    {
        EXPECT_CALL(*this, socket(family, SOCK_DGRAM, IPPROTO_UDP))
            .WillOnce(Return(kValidSockId));
    }

    void expectBindFailed(const socklen_t sockaddrSize)
    {
        EXPECT_CALL(*this, bind(kValidSockId, _, sockaddrSize))
            .WillOnce(Return(net::OPERATION_FAILED));
    }

    void expectBindSucceded(const socklen_t sockaddrSize)
    {
        EXPECT_CALL(*this, bind(kValidSockId, _, sockaddrSize))
            .WillOnce(Return(0));
    }

    void expectCloseFailed()
    {
        EXPECT_CALL(*this, close(_)).WillOnce(Return(net::OPERATION_FAILED));
    }

    void expectCloseSucceded()
    {
        EXPECT_CALL(*this, close(_)).WillOnce(Return(0));
    }

    void expectSendToSucceded()
    {
        EXPECT_CALL(*this, sendto(_, _, _, _, _, _)).WillOnce(Return(1));
    }

    void expectSendToFailed()
    {
        EXPECT_CALL(*this, sendto(_, _, _, _, _, _))
            .WillOnce(Return(net::OPERATION_FAILED));
    }

    void expectRecvFromSucceded()
    {
        EXPECT_CALL(*this, recvfrom(_, _, _, _, _, _)).WillOnce(Return(1));
    }

    void expectRecvFromFailed()
    {
        EXPECT_CALL(*this, recvfrom(_, _, _, _, _, _))
            .WillOnce(Return(net::OPERATION_FAILED));
    }
};

class SocketTest : public ::testing::Test
{
   public:
    SocketTest() = default;
    SocketTest(const SocketTest &) = delete;
    SocketTest &operator=(const SocketTest &) = delete;
    SocketTest(SocketTest &&) = delete;
    SocketTest &operator=(SocketTest &&) = delete;

    static int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen)
    {
        return mDetails->bind(sockfd, addr, addrlen);
    }

    static ssize_t recvfrom(int sockfd, void *buf, size_t len, int flags,
                            struct sockaddr *src_addr, socklen_t *addrlen)
    {
        return mDetails->recvfrom(sockfd, buf, len, flags, src_addr, addrlen);
    }

    static ssize_t sendto(int sockfd, const void *buf, size_t len, int flags,
                          const struct sockaddr *dest_addr, socklen_t addrlen)
    {
        return mDetails->sendto(sockfd, buf, len, flags, dest_addr, addrlen);
    }

    static int socket(int socket_family, int socket_type, int protocol)
    {
        return mDetails->socket(socket_family, socket_type, protocol);
    }

    static int close(int fd) { return mDetails->close(fd); }

    static std::unique_ptr<MockDetails> mDetails;

   protected:
    static void SetUpTestSuite() { mDetails = std::make_unique<MockDetails>(); }

    static void TearDownTestSuite() { mDetails = nullptr; }
};

std::unique_ptr<MockDetails> SocketTest::mDetails;

TEST_F(SocketTest, ConstructorWithUDPv4flags)
{
    net::Socket<net::UDP, SocketTest> s(net::UDP::V4());
    ASSERT_EQ(s.isOpen(), false);
    ASSERT_EQ(s.flags().getFamily(), net::eAddressFamily::kIPv4);
    ASSERT_EQ(s.flags().getProtocol(), net::eIPProtocol::kUDP);
    ASSERT_EQ(s.flags().getSocketType(), net::eSocketType::kDgram);
}

TEST_F(SocketTest, ConstructorWithUDPv6flags)
{
    net::Socket<net::UDP, SocketTest> s(net::UDP::V6());
    ASSERT_EQ(s.isOpen(), false);
    ASSERT_EQ(s.flags().getFamily(), net::eAddressFamily::kIPv6);
    ASSERT_EQ(s.flags().getProtocol(), net::eIPProtocol::kUDP);
    ASSERT_EQ(s.flags().getSocketType(), net::eSocketType::kDgram);
}

TEST_F(SocketTest, SocketFuncReturnErrorInConstructorWithUDPv4flagsCall)
{
    mDetails->expectSocketFailed(AF_INET);

    using SocketT = net::Socket<net::UDP, SocketTest>;

    EXPECT_THROW(
        {
            try
            {
                SocketT s(net::UDP::V4(), 333);
            }
            catch (const net::exception::RuntimeError &re)
            {
                EXPECT_THAT(re.what(),
                            testing::StartsWith(net::exception::kSocketOpen));
                throw;
            }
        },
        net::exception::RuntimeError);
}

TEST_F(SocketTest, SocketFuncReturnErrorInConstructorWithUDPv6flagsCall)
{
    mDetails->expectSocketFailed(AF_INET6);

    using SocketT = net::Socket<net::UDP, SocketTest>;

    EXPECT_THROW(
        {
            try
            {
                SocketT s(net::UDP::V6(), 333);
            }
            catch (const net::exception::RuntimeError &re)
            {
                EXPECT_THAT(re.what(),
                            testing::StartsWith(net::exception::kSocketOpen));
                throw;
            }
        },
        net::exception::RuntimeError);
}

TEST_F(SocketTest, ConstructorWithUDPv4flagsAndPort)
{
    InSequence seq;
    mDetails->expectSocketSucceded(AF_INET);
    mDetails->expectBindSucceded(kV4Size);

    net::Socket<net::UDP, SocketTest> s(net::UDP::V4(), 333);

    ASSERT_EQ(s.isOpen(), true);
}

TEST_F(SocketTest, ConstructorWithUDPv4flagsAndPortNotThrow)
{
    InSequence seq;
    mDetails->expectSocketSucceded(AF_INET);
    mDetails->expectBindSucceded(kV4Size);

    const auto socketCreator = []() {
        net::Socket<net::UDP, SocketTest> s(net::UDP::V4(), 333);
    };

    EXPECT_NO_THROW(socketCreator());
}

TEST_F(SocketTest, ConstructorWithUDPv6flagsAndPort)
{
    InSequence seq;
    mDetails->expectSocketSucceded(AF_INET6);
    mDetails->expectBindSucceded(kV6Size);

    net::Socket<net::UDP, SocketTest> s(net::UDP::V6(), 333);

    ASSERT_EQ(s.isOpen(), true);
}

TEST_F(SocketTest, ConstructorWithUDPv6flagsAndPortNotThrow)
{
    InSequence seq;
    mDetails->expectSocketSucceded(AF_INET6);
    mDetails->expectBindSucceded(kV6Size);

    const auto socketCreator = []() {
        net::Socket<net::UDP, SocketTest> s(net::UDP::V6(), 333);
    };

    EXPECT_NO_THROW(socketCreator());
}

TEST_F(SocketTest, ConstructorWithUDPv4flagsAndPortBindFailed)
{
    InSequence seq;
    mDetails->expectSocketSucceded(AF_INET);
    mDetails->expectBindFailed(kV4Size);

    using SocketT = net::Socket<net::UDP, SocketTest>;

    EXPECT_THROW(
        {
            try
            {
                SocketT s(net::UDP::V4(), 333);
            }
            catch (const net::exception::RuntimeError &re)
            {
                EXPECT_THAT(re.what(),
                            testing::StartsWith(net::exception::kSocketBind));
                throw;
            }
        },
        net::exception::RuntimeError);
}

TEST_F(SocketTest, ConstructorWithUDPv6flagsAndPortBindFailed)
{
    InSequence seq;
    mDetails->expectSocketSucceded(AF_INET6);
    mDetails->expectBindFailed(kV6Size);

    using SocketT = net::Socket<net::UDP, SocketTest>;

    EXPECT_THROW(
        {
            try
            {
                SocketT s(net::UDP::V6(), 333);
            }
            catch (const net::exception::RuntimeError &re)
            {
                EXPECT_THAT(re.what(),
                            testing::StartsWith(net::exception::kSocketBind));
                throw;
            }
        },
        net::exception::RuntimeError);
}

TEST_F(SocketTest, CallOpenOnAlreadyOpenedSocketV4MustThrow)
{
    InSequence seq;
    mDetails->expectSocketSucceded(AF_INET);

    net::Socket<net::UDP, SocketTest> s(net::UDP::V4());
    s.open();

    EXPECT_THROW(
        {
            try
            {
                s.open();
            }
            catch (const net::exception::LogicError &re)
            {
                EXPECT_THAT(
                    re.what(),
                    testing::StartsWith(net::exception::kSocketAlreadyOpened));
                throw;
            }
        },
        net::exception::LogicError);
}

TEST_F(SocketTest, CallOpenOnAlreadyOpenedSocketV6MustThrow)
{
    InSequence seq;
    mDetails->expectSocketSucceded(AF_INET6);

    net::Socket<net::UDP, SocketTest> s(net::UDP::V6());
    s.open();

    EXPECT_THROW(
        {
            try
            {
                s.open();
            }
            catch (const net::exception::LogicError &re)
            {
                EXPECT_THAT(
                    re.what(),
                    testing::StartsWith(net::exception::kSocketAlreadyOpened));
                throw;
            }
        },
        net::exception::LogicError);
}

TEST_F(SocketTest, MoveConstructorV4ClosedSocket)
{
    net::UDP::Socket s1(net::UDP::V4());

    net::UDP::Socket s2(std::move(s1));

    ASSERT_EQ(s2.isOpen(), false);
    ASSERT_EQ(s2.flags().getFamily(), net::eAddressFamily::kIPv4);
    ASSERT_EQ(s2.flags().getProtocol(), net::eIPProtocol::kUDP);
    ASSERT_EQ(s2.flags().getSocketType(), net::eSocketType::kDgram);
}

TEST_F(SocketTest, MoveConstructorV6ClosedSocket)
{
    net::UDP::Socket s1(net::UDP::V6());

    net::UDP::Socket s2(std::move(s1));

    ASSERT_EQ(s2.isOpen(), false);
    ASSERT_EQ(s2.flags().getFamily(), net::eAddressFamily::kIPv6);
    ASSERT_EQ(s2.flags().getProtocol(), net::eIPProtocol::kUDP);
    ASSERT_EQ(s2.flags().getSocketType(), net::eSocketType::kDgram);
}

TEST_F(SocketTest, MoveConstructorV4OpenedSocket)
{
    InSequence seq;
    mDetails->expectSocketSucceded(AF_INET);

    using SocketT = net::Socket<net::UDP, SocketTest>;

    SocketT s1(net::UDP::V4());
    s1.open();

    ASSERT_EQ(s1.isOpen(), true);

    SocketT s2(std::move(s1));

    ASSERT_EQ(s1.isOpen(), false);
    ASSERT_EQ(s2.isOpen(), true);
}

TEST_F(SocketTest, MoveConstructorV6OpenedSocket)
{
    InSequence seq;
    mDetails->expectSocketSucceded(AF_INET6);

    using SocketT = net::Socket<net::UDP, SocketTest>;

    SocketT s1(net::UDP::V6());
    s1.open();

    ASSERT_EQ(s1.isOpen(), true);

    SocketT s2(std::move(s1));

    ASSERT_EQ(s1.isOpen(), false);
    ASSERT_EQ(s2.isOpen(), true);
}

TEST_F(SocketTest, MoveAssignmentV4ClosedToV6Opened)
{
    InSequence seq;
    mDetails->expectSocketSucceded(AF_INET6);

    using SocketT = net::Socket<net::UDP, SocketTest>;

    SocketT s1(net::UDP::V4());

    SocketT s2(net::UDP::V6());
    s2.open();

    s2 = std::move(s1);

    ASSERT_EQ(s2.isOpen(), false);
    ASSERT_EQ(s2.flags().getFamily(), net::eAddressFamily::kIPv4);
    ASSERT_EQ(s2.flags().getProtocol(), net::eIPProtocol::kUDP);
    ASSERT_EQ(s2.flags().getSocketType(), net::eSocketType::kDgram);

    ASSERT_EQ(s1.isOpen(), true);
    ASSERT_EQ(s1.flags().getFamily(), net::eAddressFamily::kIPv6);
    ASSERT_EQ(s1.flags().getProtocol(), net::eIPProtocol::kUDP);
    ASSERT_EQ(s1.flags().getSocketType(), net::eSocketType::kDgram);
}

TEST_F(SocketTest, BindNotOpenedV4MustThrowLogicError)
{
    net::Socket<net::UDP, SocketTest> s(net::UDP::V4());

    EXPECT_THROW(
        {
            try
            {
                s.bind(10);
            }
            catch (const net::exception::LogicError &le)
            {
                EXPECT_THAT(le.what(),
                            testing::StartsWith(
                                net::exception::kSocketMustBeOpenToBind));
                throw;
            }
        },
        net::exception::LogicError);
}

TEST_F(SocketTest, BindNotOpenedV6MustThrowLogicError)
{
    net::Socket<net::UDP, SocketTest> s(net::UDP::V6());

    EXPECT_THROW(
        {
            try
            {
                s.bind(10);
            }
            catch (const net::exception::LogicError &le)
            {
                EXPECT_THAT(le.what(),
                            testing::StartsWith(
                                net::exception::kSocketMustBeOpenToBind));
                throw;
            }
        },
        net::exception::LogicError);
}

TEST_F(SocketTest, BindOpenedV4MustNotThrow)
{
    InSequence seq;
    mDetails->expectSocketSucceded(AF_INET);
    mDetails->expectBindSucceded(kV4Size);

    net::Socket<net::UDP, SocketTest> s(net::UDP::V4());
    s.open();

    EXPECT_NO_THROW(s.bind(10));
}

TEST_F(SocketTest, BindOpenedV6MustNotThrow)
{
    InSequence seq;
    mDetails->expectSocketSucceded(AF_INET6);
    mDetails->expectBindSucceded(kV6Size);

    net::Socket<net::UDP, SocketTest> s(net::UDP::V6());
    s.open();

    EXPECT_NO_THROW(s.bind(10));
}

TEST_F(SocketTest, BindAlreadyBoundV4MustThrowRuntimeError)
{
    InSequence seq;
    mDetails->expectSocketSucceded(AF_INET);
    mDetails->expectBindSucceded(kV4Size);
    mDetails->expectBindFailed(kV4Size);

    net::Socket<net::UDP, SocketTest> s(net::UDP::V4(), 11);

    EXPECT_THROW(
        {
            try
            {
                s.bind(11);
            }
            catch (const net::exception::RuntimeError &re)
            {
                EXPECT_THAT(re.what(),
                            testing::StartsWith(net::exception::kSocketBind));
                throw;
            }
        },
        net::exception::RuntimeError);
}

TEST_F(SocketTest, BindAlreadyBoundV6MustThrowRuntimeError)
{
    InSequence seq;
    mDetails->expectSocketSucceded(AF_INET6);
    mDetails->expectBindSucceded(kV6Size);
    mDetails->expectBindFailed(kV6Size);

    net::Socket<net::UDP, SocketTest> s(net::UDP::V6(), 11);

    EXPECT_THROW(
        {
            try
            {
                s.bind(11);
            }
            catch (const net::exception::RuntimeError &re)
            {
                EXPECT_THAT(re.what(),
                            testing::StartsWith(net::exception::kSocketBind));
                throw;
            }
        },
        net::exception::RuntimeError);
}

TEST_F(SocketTest, CloseNotOpenedSocketV4MustNotThrow)
{
    net::Socket<net::UDP, SocketTest> s(net::UDP::V4());

    EXPECT_NO_THROW(s.close());
    ASSERT_EQ(s.isOpen(), false);
}

TEST_F(SocketTest, CloseNotOpenedSocketV6MustNotThrow)
{
    net::Socket<net::UDP, SocketTest> s(net::UDP::V6());

    EXPECT_NO_THROW(s.close());
    ASSERT_EQ(s.isOpen(), false);
}

TEST_F(SocketTest, SuccessfulCloseV4MustSetIsOpenedFlagToFalse)
{
    InSequence seq;
    mDetails->expectSocketSucceded(AF_INET);
    mDetails->expectBindSucceded(kV4Size);
    mDetails->expectCloseSucceded();

    net::Socket<net::UDP, SocketTest> s(net::UDP::V4(), 11);

    ASSERT_EQ(s.isOpen(), true);
    EXPECT_NO_THROW(s.close());
    ASSERT_EQ(s.isOpen(), false);
}

TEST_F(SocketTest, SuccessfulCloseV6MustSetIsOpenedFlagToFalse)
{
    InSequence seq;
    mDetails->expectSocketSucceded(AF_INET6);
    mDetails->expectBindSucceded(kV6Size);
    mDetails->expectCloseSucceded();

    net::Socket<net::UDP, SocketTest> s(net::UDP::V6(), 11);

    ASSERT_EQ(s.isOpen(), true);
    EXPECT_NO_THROW(s.close());
    ASSERT_EQ(s.isOpen(), false);
}

TEST_F(SocketTest, FailedCloseV4MustThrowRuntimeError)
{
    InSequence seq;
    mDetails->expectSocketSucceded(AF_INET);
    mDetails->expectBindSucceded(kV4Size);
    mDetails->expectCloseFailed();

    net::Socket<net::UDP, SocketTest> s(net::UDP::V4(), 11);

    ASSERT_EQ(s.isOpen(), true);

    EXPECT_THROW(
        {
            try
            {
                s.close();
            }
            catch (const net::exception::RuntimeError &re)
            {
                EXPECT_THAT(re.what(),
                            testing::StartsWith(net::exception::kSocketClose));
                throw;
            }
        },
        net::exception::RuntimeError);
}

TEST_F(SocketTest, FailedCloseV6MustThrowRuntimeError)
{
    InSequence seq;
    mDetails->expectSocketSucceded(AF_INET6);
    mDetails->expectBindSucceded(kV6Size);
    mDetails->expectCloseFailed();

    net::Socket<net::UDP, SocketTest> s(net::UDP::V6(), 11);

    ASSERT_EQ(s.isOpen(), true);

    EXPECT_THROW(
        {
            try
            {
                s.close();
            }
            catch (const net::exception::RuntimeError &re)
            {
                EXPECT_THAT(re.what(),
                            testing::StartsWith(net::exception::kSocketClose));
                throw;
            }
        },
        net::exception::RuntimeError);
}

TEST_F(SocketTest, FailedSendToMustThrowRuntimeError)
{
    InSequence seq;
    mDetails->expectSocketSucceded(AF_INET);
    mDetails->expectBindSucceded(kV4Size);
    mDetails->expectSendToFailed();

    net::Socket<net::UDP, SocketTest> s(net::UDP::V4(), 11);
    net::Address dst;
    const char *buf = nullptr;
    const std::size_t bufLen = 0;

    EXPECT_THROW(
        {
            try
            {
                s.sendTo(dst, buf, bufLen);
            }
            catch (const net::exception::RuntimeError &re)
            {
                EXPECT_THAT(re.what(),
                            testing::StartsWith(net::exception::kSocketSendTo));
                throw;
            }
        },
        net::exception::RuntimeError);
}

TEST_F(SocketTest, SuccessfulSendToMustReturnCountBytesSent)
{
    InSequence seq;
    mDetails->expectSocketSucceded(AF_INET);
    mDetails->expectBindSucceded(kV4Size);
    mDetails->expectSendToSucceded();

    net::Socket<net::UDP, SocketTest> s(net::UDP::V4(), 11);
    net::Address dst;
    const char *buf = nullptr;
    const std::size_t bufLen = 0;
    ASSERT_EQ(s.sendTo(dst, buf, bufLen), 1);
}

TEST_F(SocketTest, SuccessfulRecvFromMustReturnCountBytesRecieved)
{
    InSequence seq;
    mDetails->expectSocketSucceded(AF_INET);
    mDetails->expectBindSucceded(kV4Size);
    mDetails->expectRecvFromSucceded();

    net::Socket<net::UDP, SocketTest> s(net::UDP::V4(), 11);
    net::Address dst;
    char *buf = nullptr;
    const std::size_t bufLen = 0;
    ASSERT_EQ(s.recvFrom(buf, bufLen, dst), 1);
}

TEST_F(SocketTest, FailedRecvFromMustThrowRuntimeError)
{
    InSequence seq;
    mDetails->expectSocketSucceded(AF_INET);
    mDetails->expectBindSucceded(kV4Size);
    mDetails->expectRecvFromFailed();

    net::Socket<net::UDP, SocketTest> s(net::UDP::V4(), 11);
    net::Address dst;
    char *buf = nullptr;
    const std::size_t bufLen = 0;

    EXPECT_THROW(
        {
            try
            {
                s.recvFrom(buf, bufLen, dst);
            }
            catch (const net::exception::RuntimeError &re)
            {
                EXPECT_THAT(re.what(), testing::StartsWith(
                                           net::exception::kSocketRecvFrom));
                throw;
            }
        },
        net::exception::RuntimeError);
}