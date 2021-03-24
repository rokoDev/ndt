#include <fmt/core.h>
#include <algorithm>
#include <array>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "ndt/address.h"
#include "ndt/context.h"
#include "ndt/exception.h"
#include "ndt/socket.h"
#include "ndt/udp.h"
#include "test/sys_error_code_getter.h"

using ::testing::_;
using ::testing::InSequence;
using ::testing::Return;

constexpr ndt::sock_t kValidSockId{1};
constexpr int kBindSucceeded = 0;
constexpr int kCloseSucceeded = 0;
constexpr ndt::sdlen_t kRecvfromSucceeded = 1;
constexpr ndt::sdlen_t kSendtoSucceeded = 1;
constexpr ndt::salen_t kV4Size = sizeof(sockaddr_in);
constexpr ndt::salen_t kV6Size = sizeof(sockaddr_in6);

class MockDetails
{
   public:
    MOCK_METHOD(int, bind,
                (ndt::sock_t, const struct sockaddr *, ndt::salen_t));
    MOCK_METHOD(ndt::sdlen_t, recvfrom,
                (ndt::sock_t, ndt::bufp_t, ndt::dlen_t, int, struct sockaddr *,
                 ndt::salen_t *));
    MOCK_METHOD(ndt::sdlen_t, sendto,
                (ndt::sock_t, ndt::cbufp_t, ndt::dlen_t, int,
                 const struct sockaddr *, ndt::salen_t));
    MOCK_METHOD(ndt::sock_t, socket, (int, int, int));
    MOCK_METHOD(int, close, (ndt::sock_t));

    void expectSocketFailed(const int family)
    {
        EXPECT_CALL(*this, socket(family, SOCK_DGRAM, IPPROTO_UDP))
            .WillOnce(Return(ndt::kInvalidSocket));
    }

    void expectSocketSucceded(const int family)
    {
        EXPECT_CALL(*this, socket(family, SOCK_DGRAM, IPPROTO_UDP))
            .WillOnce(Return(kValidSockId));
    }

    void expectBindFailed(const ndt::salen_t sockaddrSize)
    {
        EXPECT_CALL(*this, bind(kValidSockId, _, sockaddrSize))
            .WillOnce(Return(ndt::kSocketError));
    }

    void expectBindSucceded(const ndt::salen_t sockaddrSize)
    {
        EXPECT_CALL(*this, bind(kValidSockId, _, sockaddrSize))
            .WillOnce(Return(kBindSucceeded));
    }

    void expectCloseFailed()
    {
        EXPECT_CALL(*this, close(_)).WillOnce(Return(ndt::kSocketError));
    }

    void expectCloseSucceded()
    {
        EXPECT_CALL(*this, close(_)).WillOnce(Return(kCloseSucceeded));
    }

    void expectSendToSucceded()
    {
        EXPECT_CALL(*this, sendto(_, _, _, _, _, _))
            .WillOnce(Return(kSendtoSucceeded));
    }

    void expectSendToFailed()
    {
        EXPECT_CALL(*this, sendto(_, _, _, _, _, _))
            .WillOnce(Return(ndt::kSocketError));
    }

    void expectRecvFromSucceded()
    {
        EXPECT_CALL(*this, recvfrom(_, _, _, _, _, _))
            .WillOnce(Return(kRecvfromSucceeded));
    }

    void expectRecvFromFailed()
    {
        EXPECT_CALL(*this, recvfrom(_, _, _, _, _, _))
            .WillOnce(Return(ndt::kSocketError));
    }
};

class SocketTest : public ::testing::Test
{
   public:
    ndt::test::SysErrorCodeGetter sysErrorGetter;
    ndt::Context ctx;
    SocketTest() = default;
    SocketTest(const SocketTest &) = delete;
    SocketTest &operator=(const SocketTest &) = delete;
    SocketTest(SocketTest &&) = delete;
    SocketTest &operator=(SocketTest &&) = delete;

    static int bind(ndt::sock_t sockfd, const struct sockaddr *addr,
                    ndt::salen_t addrlen)
    {
        return mDetails->bind(sockfd, addr, addrlen);
    }

    static ndt::sdlen_t recvfrom(ndt::sock_t sockfd, ndt::bufp_t buf,
                                 ndt::dlen_t len, int flags,
                                 struct sockaddr *src_addr,
                                 ndt::salen_t *addrlen)
    {
        return mDetails->recvfrom(sockfd, buf, len, flags, src_addr, addrlen);
    }

    static ndt::sdlen_t sendto(ndt::sock_t sockfd, ndt::cbufp_t buf,
                               ndt::dlen_t len, int flags,
                               const struct sockaddr *dest_addr,
                               ndt::salen_t addrlen)
    {
        return mDetails->sendto(sockfd, buf, len, flags, dest_addr, addrlen);
    }

    static ndt::sock_t socket(int socket_family, int socket_type, int protocol)
    {
        return mDetails->socket(socket_family, socket_type, protocol);
    }

    static int close(ndt::sock_t fd) { return mDetails->close(fd); }

    static std::unique_ptr<MockDetails> mDetails;

   protected:
    static void SetUpTestSuite() { mDetails = std::make_unique<MockDetails>(); }

    static void TearDownTestSuite() { mDetails = nullptr; }
};

std::unique_ptr<MockDetails> SocketTest::mDetails;

TEST_F(SocketTest, ConstructorWithUDPv4flags)
{
    ndt::Socket<ndt::UDP, SocketTest> s(ctx, ndt::UDP::V4());
    ASSERT_EQ(s.isOpen(), false);
    ASSERT_EQ(s.flags().getFamily(), ndt::eAddressFamily::kIPv4);
    ASSERT_EQ(s.flags().getProtocol(), ndt::eIPProtocol::kUDP);
    ASSERT_EQ(s.flags().getSocketType(), ndt::eSocketType::kDgram);
}

TEST_F(SocketTest, ConstructorWithUDPv6flags)
{
    ndt::Socket<ndt::UDP, SocketTest> s(ctx, ndt::UDP::V6());
    ASSERT_EQ(s.isOpen(), false);
    ASSERT_EQ(s.flags().getFamily(), ndt::eAddressFamily::kIPv6);
    ASSERT_EQ(s.flags().getProtocol(), ndt::eIPProtocol::kUDP);
    ASSERT_EQ(s.flags().getSocketType(), ndt::eSocketType::kDgram);
}

TEST_F(SocketTest, SocketFuncReturnErrorInConstructorWithUDPv4flagsCall)
{
    mDetails->expectSocketFailed(AF_INET);

    using SocketT = ndt::Socket<ndt::UDP, SocketTest>;

    EXPECT_THROW(
        {
            try
            {
                SocketT s(ctx, ndt::UDP::V4(), 333);
            }
            catch (const ndt::Error &e)
            {
                throw;
            }
        },
        ndt::Error);
}

TEST_F(SocketTest, SocketFuncReturnErrorInConstructorWithUDPv6flagsCall)
{
    mDetails->expectSocketFailed(AF_INET6);

    using SocketT = ndt::Socket<ndt::UDP, SocketTest>;

    EXPECT_THROW(
        {
            try
            {
                SocketT s(ctx, ndt::UDP::V6(), 333);
            }
            catch (const ndt::Error &e)
            {
                throw;
            }
        },
        ndt::Error);
}

TEST_F(SocketTest, ConstructorWithUDPv4flagsAndPort)
{
    InSequence seq;
    mDetails->expectSocketSucceded(AF_INET);
    mDetails->expectBindSucceded(kV4Size);
    mDetails->expectCloseSucceded();

    ndt::Socket<ndt::UDP, SocketTest> s(ctx, ndt::UDP::V4(), 333);

    ASSERT_EQ(s.isOpen(), true);
    s.close();
}

TEST_F(SocketTest, ConstructorWithUDPv4flagsAndPortNotThrow)
{
    InSequence seq;
    mDetails->expectSocketSucceded(AF_INET);
    mDetails->expectBindSucceded(kV4Size);
    mDetails->expectCloseSucceded();

    auto socketCreator = [&context = ctx]() {
        ndt::Socket<ndt::UDP, SocketTest> s(context, ndt::UDP::V4(), 333);
        s.close();
    };

    EXPECT_NO_THROW(socketCreator());
}

TEST_F(SocketTest, ConstructorWithUDPv6flagsAndPort)
{
    InSequence seq;
    mDetails->expectSocketSucceded(AF_INET6);
    mDetails->expectBindSucceded(kV6Size);
    mDetails->expectCloseSucceded();

    ndt::Socket<ndt::UDP, SocketTest> s(ctx, ndt::UDP::V6(), 333);

    ASSERT_EQ(s.isOpen(), true);
    s.close();
}

TEST_F(SocketTest, ConstructorWithUDPv6flagsAndPortNotThrow)
{
    InSequence seq;
    mDetails->expectSocketSucceded(AF_INET6);
    mDetails->expectBindSucceded(kV6Size);
    mDetails->expectCloseSucceded();

    auto socketCreator = [&context = ctx]() mutable {
        ndt::Socket<ndt::UDP, SocketTest> s(context, ndt::UDP::V6(), 333);
        s.close();
    };

    EXPECT_NO_THROW(socketCreator());
}

TEST_F(SocketTest, ConstructorWithUDPv4flagsAndPortBindFailed)
{
    InSequence seq;
    mDetails->expectSocketSucceded(AF_INET);
    mDetails->expectBindFailed(kV4Size);
    mDetails->expectCloseSucceded();

    using SocketT = ndt::Socket<ndt::UDP, SocketTest>;

    EXPECT_THROW(
        {
            try
            {
                SocketT s(ctx, ndt::UDP::V4(), 333);
            }
            catch (const ndt::Error &e)
            {
                throw;
            }
        },
        ndt::Error);
}

TEST_F(SocketTest, ConstructorWithUDPv6flagsAndPortBindFailed)
{
    InSequence seq;
    mDetails->expectSocketSucceded(AF_INET6);
    mDetails->expectBindFailed(kV6Size);
    mDetails->expectCloseSucceded();

    using SocketT = ndt::Socket<ndt::UDP, SocketTest>;

    EXPECT_THROW(
        {
            try
            {
                SocketT s(ctx, ndt::UDP::V6(), 333);
            }
            catch (const ndt::Error &e)
            {
                throw;
            }
        },
        ndt::Error);
}

TEST_F(SocketTest, CallOpenOnAlreadyOpenedSocketV4MustThrow)
{
    InSequence seq;
    mDetails->expectSocketSucceded(AF_INET);
    mDetails->expectSocketFailed(AF_INET);
    mDetails->expectCloseSucceded();

    ndt::Socket<ndt::UDP, SocketTest> s(ctx, ndt::UDP::V4());
    s.open();

    EXPECT_THROW(
        {
            try
            {
                s.open();
            }
            catch (const ndt::Error &e)
            {
                s.close();
                throw;
            }
        },
        ndt::Error);
}

TEST_F(SocketTest, CallOpenOnAlreadyOpenedSocketV6MustThrow)
{
    InSequence seq;
    mDetails->expectSocketSucceded(AF_INET6);
    mDetails->expectSocketFailed(AF_INET6);
    mDetails->expectCloseSucceded();

    ndt::Socket<ndt::UDP, SocketTest> s(ctx, ndt::UDP::V6());
    s.open();

    EXPECT_THROW(
        {
            try
            {
                s.open();
            }
            catch (const ndt::Error &e)
            {
                s.close();
                throw;
            }
        },
        ndt::Error);
}

TEST_F(SocketTest, MoveConstructorV4ClosedSocket)
{
    ndt::UDP::Socket s1(ctx, ndt::UDP::V4());

    ndt::UDP::Socket s2(std::move(s1));

    ASSERT_EQ(s2.isOpen(), false);
    ASSERT_EQ(s2.flags().getFamily(), ndt::eAddressFamily::kIPv4);
    ASSERT_EQ(s2.flags().getProtocol(), ndt::eIPProtocol::kUDP);
    ASSERT_EQ(s2.flags().getSocketType(), ndt::eSocketType::kDgram);
}

TEST_F(SocketTest, MoveConstructorV6ClosedSocket)
{
    ndt::UDP::Socket s1(ctx, ndt::UDP::V6());

    ndt::UDP::Socket s2(std::move(s1));

    ASSERT_EQ(s2.isOpen(), false);
    ASSERT_EQ(s2.flags().getFamily(), ndt::eAddressFamily::kIPv6);
    ASSERT_EQ(s2.flags().getProtocol(), ndt::eIPProtocol::kUDP);
    ASSERT_EQ(s2.flags().getSocketType(), ndt::eSocketType::kDgram);
}

TEST_F(SocketTest, MoveConstructorV4OpenedSocket)
{
    InSequence seq;
    mDetails->expectSocketSucceded(AF_INET);
    mDetails->expectCloseSucceded();

    using SocketT = ndt::Socket<ndt::UDP, SocketTest>;

    SocketT s1(ctx, ndt::UDP::V4());
    s1.open();

    ASSERT_EQ(s1.isOpen(), true);

    SocketT s2(std::move(s1));

    ASSERT_EQ(s1.isOpen(), false);
    ASSERT_EQ(s2.isOpen(), true);
    s2.close();
}

TEST_F(SocketTest, MoveConstructorV6OpenedSocket)
{
    InSequence seq;
    mDetails->expectSocketSucceded(AF_INET6);
    mDetails->expectCloseSucceded();

    using SocketT = ndt::Socket<ndt::UDP, SocketTest>;

    SocketT s1(ctx, ndt::UDP::V6());
    s1.open();

    ASSERT_EQ(s1.isOpen(), true);

    SocketT s2(std::move(s1));

    ASSERT_EQ(s1.isOpen(), false);
    ASSERT_EQ(s2.isOpen(), true);
    s2.close();
}

TEST_F(SocketTest, MoveAssignmentV4OpenedToV6Closed)
{
    InSequence seq;
    mDetails->expectSocketSucceded(AF_INET);
    mDetails->expectCloseSucceded();

    using SocketT = ndt::Socket<ndt::UDP, SocketTest>;

    SocketT s1(ctx, ndt::UDP::V4());
    s1.open();

    SocketT s2(ctx, ndt::UDP::V6());

    s2 = std::move(s1);

    ASSERT_EQ(s1.isOpen(), false);
    ASSERT_EQ(s1.flags().getFamily(), ndt::eAddressFamily::kIPv6);
    ASSERT_EQ(s1.flags().getProtocol(), ndt::eIPProtocol::kUDP);
    ASSERT_EQ(s1.flags().getSocketType(), ndt::eSocketType::kDgram);

    ASSERT_EQ(s2.isOpen(), true);
    ASSERT_EQ(s2.flags().getFamily(), ndt::eAddressFamily::kIPv4);
    ASSERT_EQ(s2.flags().getProtocol(), ndt::eIPProtocol::kUDP);
    ASSERT_EQ(s2.flags().getSocketType(), ndt::eSocketType::kDgram);
    s2.close();
}

TEST_F(SocketTest, BindNotOpenedV4MustThrowError)
{
    ndt::UDP::Socket s(ctx, ndt::UDP::V4());

    EXPECT_THROW(
        {
            try
            {
                s.bind(10);
            }
            catch (const ndt::Error &e)
            {
                throw;
            }
        },
        ndt::Error);
}

TEST_F(SocketTest, BindNotOpenedV6MustThrowError)
{
    ndt::UDP::Socket s(ctx, ndt::UDP::V6());

    EXPECT_THROW(
        {
            try
            {
                s.bind(10);
            }
            catch (const ndt::Error &e)
            {
                throw;
            }
        },
        ndt::Error);
}

TEST_F(SocketTest, BindOpenedV4MustNotThrow)
{
    InSequence seq;
    mDetails->expectSocketSucceded(AF_INET);
    mDetails->expectBindSucceded(kV4Size);
    mDetails->expectCloseSucceded();

    ndt::Socket<ndt::UDP, SocketTest> s(ctx, ndt::UDP::V4());
    s.open();

    EXPECT_NO_THROW(s.bind(10));
    s.close();
}

TEST_F(SocketTest, BindOpenedV6MustNotThrow)
{
    InSequence seq;
    mDetails->expectSocketSucceded(AF_INET6);
    mDetails->expectBindSucceded(kV6Size);
    mDetails->expectCloseSucceded();

    ndt::Socket<ndt::UDP, SocketTest> s(ctx, ndt::UDP::V6());
    s.open();

    EXPECT_NO_THROW(s.bind(10));
    s.close();
}

TEST_F(SocketTest, BindAlreadyBoundV4MustThrowError)
{
    InSequence seq;
    mDetails->expectSocketSucceded(AF_INET);
    mDetails->expectBindSucceded(kV4Size);
    mDetails->expectBindFailed(kV4Size);
    mDetails->expectCloseSucceded();

    ndt::Socket<ndt::UDP, SocketTest> s(ctx, ndt::UDP::V4(), 11);

    EXPECT_THROW(
        {
            try
            {
                s.bind(11);
            }
            catch (const ndt::Error &e)
            {
                throw;
            }
        },
        ndt::Error);
    s.close();
}

TEST_F(SocketTest, BindAlreadyBoundV6MustThrowError)
{
    InSequence seq;
    mDetails->expectSocketSucceded(AF_INET6);
    mDetails->expectBindSucceded(kV6Size);
    mDetails->expectBindFailed(kV6Size);
    mDetails->expectCloseSucceded();

    ndt::Socket<ndt::UDP, SocketTest> s(ctx, ndt::UDP::V6(), 11);

    EXPECT_THROW(
        {
            try
            {
                s.bind(11);
            }
            catch (const ndt::Error &e)
            {
                throw;
            }
        },
        ndt::Error);
    s.close();
}

TEST_F(SocketTest, CloseNotOpenedSocketV4MustNotThrow)
{
    ndt::Socket<ndt::UDP, SocketTest> s(ctx, ndt::UDP::V4());

    EXPECT_NO_THROW(s.close());
    ASSERT_EQ(s.isOpen(), false);
}

TEST_F(SocketTest, CloseNotOpenedSocketV6MustNotThrow)
{
    ndt::Socket<ndt::UDP, SocketTest> s(ctx, ndt::UDP::V6());

    EXPECT_NO_THROW(s.close());
    ASSERT_EQ(s.isOpen(), false);
}

TEST_F(SocketTest, SuccessfulCloseV4MustSetIsOpenedFlagToFalse)
{
    InSequence seq;
    mDetails->expectSocketSucceded(AF_INET);
    mDetails->expectBindSucceded(kV4Size);
    mDetails->expectCloseSucceded();

    ndt::Socket<ndt::UDP, SocketTest> s(ctx, ndt::UDP::V4(), 11);

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

    ndt::Socket<ndt::UDP, SocketTest> s(ctx, ndt::UDP::V6(), 11);

    ASSERT_EQ(s.isOpen(), true);
    EXPECT_NO_THROW(s.close());
    ASSERT_EQ(s.isOpen(), false);
}

TEST_F(SocketTest, FailedCloseV4MustThrowError)
{
    InSequence seq;
    mDetails->expectSocketSucceded(AF_INET);
    mDetails->expectBindSucceded(kV4Size);
    mDetails->expectCloseFailed();
    mDetails->expectCloseSucceded();

    ndt::Socket<ndt::UDP, SocketTest> s(ctx, ndt::UDP::V4(), 11);

    ASSERT_EQ(s.isOpen(), true);

    EXPECT_THROW(
        {
            try
            {
                s.close();
            }
            catch (const ndt::Error &e)
            {
                throw;
            }
        },
        ndt::Error);
    s.close();
}

TEST_F(SocketTest, FailedCloseV6MustThrowError)
{
    InSequence seq;
    mDetails->expectSocketSucceded(AF_INET6);
    mDetails->expectBindSucceded(kV6Size);
    mDetails->expectCloseFailed();
    mDetails->expectCloseSucceded();

    ndt::Socket<ndt::UDP, SocketTest> s(ctx, ndt::UDP::V6(), 11);

    ASSERT_EQ(s.isOpen(), true);

    EXPECT_THROW(
        {
            try
            {
                s.close();
            }
            catch (const ndt::Error &e)
            {
                throw;
            }
        },
        ndt::Error);
    s.close();
}

TEST_F(SocketTest, FailedSendToMustThrowError)
{
    InSequence seq;
    mDetails->expectSocketSucceded(AF_INET);
    mDetails->expectBindSucceded(kV4Size);
    mDetails->expectSendToFailed();
    mDetails->expectCloseSucceded();

    ndt::Socket<ndt::UDP, SocketTest> s(ctx, ndt::UDP::V4(), 11);
    ndt::Address dst;

    EXPECT_THROW(
        {
            try
            {
                s.sendTo(dst, {static_cast<ndt::cbufp_t>(nullptr), 0});
            }
            catch (const ndt::Error &e)
            {
                throw;
            }
        },
        ndt::Error);
    s.close();
}

TEST_F(SocketTest, SuccessfulSendToMustReturnCountBytesSent)
{
    InSequence seq;
    mDetails->expectSocketSucceded(AF_INET);
    mDetails->expectBindSucceded(kV4Size);
    mDetails->expectSendToSucceded();
    mDetails->expectCloseSucceded();

    ndt::Socket<ndt::UDP, SocketTest> s(ctx, ndt::UDP::V4(), 11);
    ndt::Address dst;
    ASSERT_EQ(s.sendTo(dst, {static_cast<ndt::cbufp_t>(nullptr), 0}),
              kSendtoSucceeded);
    s.close();
}

TEST_F(SocketTest, SuccessfulRecvFromMustReturnCountBytesRecieved)
{
    InSequence seq;
    mDetails->expectSocketSucceded(AF_INET);
    mDetails->expectBindSucceded(kV4Size);
    mDetails->expectRecvFromSucceded();
    mDetails->expectCloseSucceded();

    ndt::Socket<ndt::UDP, SocketTest> s(ctx, ndt::UDP::V4(), 11);
    ndt::Address dst;
    ndt::Buffer buf({static_cast<ndt::bufp_t>(nullptr), 0});
    ASSERT_EQ(s.recvFrom(buf, dst), kRecvfromSucceeded);
    s.close();
}

TEST_F(SocketTest, FailedRecvFromMustThrowError)
{
    InSequence seq;
    mDetails->expectSocketSucceded(AF_INET);
    mDetails->expectBindSucceded(kV4Size);
    mDetails->expectRecvFromFailed();
    mDetails->expectCloseSucceded();

    ndt::Socket<ndt::UDP, SocketTest> s(ctx, ndt::UDP::V4(), 11);
    ndt::Address dst;

    EXPECT_THROW(
        {
            try
            {
                ndt::Buffer buf({static_cast<ndt::bufp_t>(nullptr), 0});
                s.recvFrom(buf, dst);
            }
            catch (const ndt::Error &e)
            {
                throw;
            }
        },
        ndt::Error);
    s.close();
}

TEST(SocketTests, SetNonBlockingMode)
{
    ndt::Context ctx;
    ndt::UDP::Socket s(ctx, ndt::UDP::V4(), 111);
    ASSERT_EQ(s.nonBlocking(), false);
    auto action = [](ndt::UDP::Socket &sRef) {
        sRef.nonBlocking(true);
        ASSERT_EQ(sRef.nonBlocking(), true);
        sRef.nonBlocking(false);
        ASSERT_EQ(sRef.nonBlocking(), false);
    };
    EXPECT_NO_THROW(action(s));
    s.close();
}

TEST(SocketTests, MoveConstructorWithNonBlockingMode)
{
    ndt::Context ctx;
    ndt::UDP::Socket s(ctx, ndt::UDP::V6(), 111);
    s.nonBlocking(true);
    ASSERT_EQ(s.nonBlocking(), true);

    auto movedSocket(std::move(s));
    ASSERT_EQ(s.nonBlocking(), false);
    ASSERT_EQ(movedSocket.nonBlocking(), true);
    movedSocket.close();
}

TEST(SocketTests, MoveAssignmentWithNonBlockingMode)
{
    ndt::Context ctx;
    ndt::UDP::Socket s(ctx, ndt::UDP::V6(), 112);
    s.nonBlocking(true);
    ASSERT_EQ(s.nonBlocking(), true);

    auto movedSocket = std::move(s);
    ASSERT_EQ(s.nonBlocking(), false);
    ASSERT_EQ(movedSocket.nonBlocking(), true);
    movedSocket.close();
}