#include <fmt/core.h>
#include <algorithm>
#include <array>

#include "ndt/address.h"
#include "ndt/exception.h"
#include "ndt/socket.h"
#include "ndt/udp.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

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
                (ndt::sock_t, ndt::buf_t, ndt::dlen_t, int, struct sockaddr *,
                 ndt::salen_t *));
    MOCK_METHOD(ndt::sdlen_t, sendto,
                (ndt::sock_t, ndt::cbuf_t, ndt::dlen_t, int,
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

    static ndt::sdlen_t recvfrom(ndt::sock_t sockfd, ndt::buf_t buf,
                                 ndt::dlen_t len, int flags,
                                 struct sockaddr *src_addr,
                                 ndt::salen_t *addrlen)
    {
        return mDetails->recvfrom(sockfd, buf, len, flags, src_addr, addrlen);
    }

    static ndt::sdlen_t sendto(ndt::sock_t sockfd, ndt::cbuf_t buf,
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
    ndt::Socket<ndt::UDP, SocketTest> s(ndt::UDP::V4());
    ASSERT_EQ(s.isOpen(), false);
    ASSERT_EQ(s.flags().getFamily(), ndt::eAddressFamily::kIPv4);
    ASSERT_EQ(s.flags().getProtocol(), ndt::eIPProtocol::kUDP);
    ASSERT_EQ(s.flags().getSocketType(), ndt::eSocketType::kDgram);
}

TEST_F(SocketTest, ConstructorWithUDPv6flags)
{
    ndt::Socket<ndt::UDP, SocketTest> s(ndt::UDP::V6());
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
                SocketT s(ndt::UDP::V4(), 333);
            }
            catch (const ndt::Error &re)
            {
                EXPECT_THAT(re.what(),
                            testing::StartsWith(ndt::exception::kSocketOpen));
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
                SocketT s(ndt::UDP::V6(), 333);
            }
            catch (const ndt::Error &re)
            {
                EXPECT_THAT(re.what(),
                            testing::StartsWith(ndt::exception::kSocketOpen));
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

    ndt::Socket<ndt::UDP, SocketTest> s(ndt::UDP::V4(), 333);

    ASSERT_EQ(s.isOpen(), true);
}

TEST_F(SocketTest, ConstructorWithUDPv4flagsAndPortNotThrow)
{
    InSequence seq;
    mDetails->expectSocketSucceded(AF_INET);
    mDetails->expectBindSucceded(kV4Size);

    const auto socketCreator = []() {
        ndt::Socket<ndt::UDP, SocketTest> s(ndt::UDP::V4(), 333);
    };

    EXPECT_NO_THROW(socketCreator());
}

TEST_F(SocketTest, ConstructorWithUDPv6flagsAndPort)
{
    InSequence seq;
    mDetails->expectSocketSucceded(AF_INET6);
    mDetails->expectBindSucceded(kV6Size);

    ndt::Socket<ndt::UDP, SocketTest> s(ndt::UDP::V6(), 333);

    ASSERT_EQ(s.isOpen(), true);
}

TEST_F(SocketTest, ConstructorWithUDPv6flagsAndPortNotThrow)
{
    InSequence seq;
    mDetails->expectSocketSucceded(AF_INET6);
    mDetails->expectBindSucceded(kV6Size);

    const auto socketCreator = []() {
        ndt::Socket<ndt::UDP, SocketTest> s(ndt::UDP::V6(), 333);
    };

    EXPECT_NO_THROW(socketCreator());
}

TEST_F(SocketTest, ConstructorWithUDPv4flagsAndPortBindFailed)
{
    InSequence seq;
    mDetails->expectSocketSucceded(AF_INET);
    mDetails->expectBindFailed(kV4Size);

    using SocketT = ndt::Socket<ndt::UDP, SocketTest>;

    EXPECT_THROW(
        {
            try
            {
                SocketT s(ndt::UDP::V4(), 333);
            }
            catch (const ndt::Error &re)
            {
                EXPECT_THAT(re.what(),
                            testing::StartsWith(ndt::exception::kSocketBind));
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

    using SocketT = ndt::Socket<ndt::UDP, SocketTest>;

    EXPECT_THROW(
        {
            try
            {
                SocketT s(ndt::UDP::V6(), 333);
            }
            catch (const ndt::Error &re)
            {
                EXPECT_THAT(re.what(),
                            testing::StartsWith(ndt::exception::kSocketBind));
                throw;
            }
        },
        ndt::Error);
}

TEST_F(SocketTest, CallOpenOnAlreadyOpenedSocketV4MustThrow)
{
    InSequence seq;
    mDetails->expectSocketSucceded(AF_INET);

    ndt::Socket<ndt::UDP, SocketTest> s(ndt::UDP::V4());
    s.open();

    EXPECT_THROW(
        {
            try
            {
                s.open();
            }
            catch (const ndt::Error &re)
            {
                EXPECT_THAT(
                    re.what(),
                    testing::StartsWith(ndt::exception::kSocketAlreadyOpened));
                throw;
            }
        },
        ndt::Error);
}

TEST_F(SocketTest, CallOpenOnAlreadyOpenedSocketV6MustThrow)
{
    InSequence seq;
    mDetails->expectSocketSucceded(AF_INET6);

    ndt::Socket<ndt::UDP, SocketTest> s(ndt::UDP::V6());
    s.open();

    EXPECT_THROW(
        {
            try
            {
                s.open();
            }
            catch (const ndt::Error &re)
            {
                EXPECT_THAT(
                    re.what(),
                    testing::StartsWith(ndt::exception::kSocketAlreadyOpened));
                throw;
            }
        },
        ndt::Error);
}

TEST_F(SocketTest, MoveConstructorV4ClosedSocket)
{
    ndt::UDP::Socket s1(ndt::UDP::V4());

    ndt::UDP::Socket s2(std::move(s1));

    ASSERT_EQ(s2.isOpen(), false);
    ASSERT_EQ(s2.flags().getFamily(), ndt::eAddressFamily::kIPv4);
    ASSERT_EQ(s2.flags().getProtocol(), ndt::eIPProtocol::kUDP);
    ASSERT_EQ(s2.flags().getSocketType(), ndt::eSocketType::kDgram);
}

TEST_F(SocketTest, MoveConstructorV6ClosedSocket)
{
    ndt::UDP::Socket s1(ndt::UDP::V6());

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

    using SocketT = ndt::Socket<ndt::UDP, SocketTest>;

    SocketT s1(ndt::UDP::V4());
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

    using SocketT = ndt::Socket<ndt::UDP, SocketTest>;

    SocketT s1(ndt::UDP::V6());
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

    using SocketT = ndt::Socket<ndt::UDP, SocketTest>;

    SocketT s1(ndt::UDP::V4());

    SocketT s2(ndt::UDP::V6());
    s2.open();

    s2 = std::move(s1);

    ASSERT_EQ(s2.isOpen(), false);
    ASSERT_EQ(s2.flags().getFamily(), ndt::eAddressFamily::kIPv4);
    ASSERT_EQ(s2.flags().getProtocol(), ndt::eIPProtocol::kUDP);
    ASSERT_EQ(s2.flags().getSocketType(), ndt::eSocketType::kDgram);

    ASSERT_EQ(s1.isOpen(), true);
    ASSERT_EQ(s1.flags().getFamily(), ndt::eAddressFamily::kIPv6);
    ASSERT_EQ(s1.flags().getProtocol(), ndt::eIPProtocol::kUDP);
    ASSERT_EQ(s1.flags().getSocketType(), ndt::eSocketType::kDgram);
}

TEST_F(SocketTest, BindNotOpenedV4MustThrowError)
{
    ndt::Socket<ndt::UDP, SocketTest> s(ndt::UDP::V4());

    EXPECT_THROW(
        {
            try
            {
                s.bind(10);
            }
            catch (const ndt::Error &le)
            {
                EXPECT_THAT(le.what(),
                            testing::StartsWith(
                                ndt::exception::kSocketMustBeOpenToBind));
                throw;
            }
        },
        ndt::Error);
}

TEST_F(SocketTest, BindNotOpenedV6MustThrowError)
{
    ndt::Socket<ndt::UDP, SocketTest> s(ndt::UDP::V6());

    EXPECT_THROW(
        {
            try
            {
                s.bind(10);
            }
            catch (const ndt::Error &le)
            {
                EXPECT_THAT(le.what(),
                            testing::StartsWith(
                                ndt::exception::kSocketMustBeOpenToBind));
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

    ndt::Socket<ndt::UDP, SocketTest> s(ndt::UDP::V4());
    s.open();

    EXPECT_NO_THROW(s.bind(10));
}

TEST_F(SocketTest, BindOpenedV6MustNotThrow)
{
    InSequence seq;
    mDetails->expectSocketSucceded(AF_INET6);
    mDetails->expectBindSucceded(kV6Size);

    ndt::Socket<ndt::UDP, SocketTest> s(ndt::UDP::V6());
    s.open();

    EXPECT_NO_THROW(s.bind(10));
}

TEST_F(SocketTest, BindAlreadyBoundV4MustThrowError)
{
    InSequence seq;
    mDetails->expectSocketSucceded(AF_INET);
    mDetails->expectBindSucceded(kV4Size);
    mDetails->expectBindFailed(kV4Size);

    ndt::Socket<ndt::UDP, SocketTest> s(ndt::UDP::V4(), 11);

    EXPECT_THROW(
        {
            try
            {
                s.bind(11);
            }
            catch (const ndt::Error &re)
            {
                EXPECT_THAT(re.what(),
                            testing::StartsWith(ndt::exception::kSocketBind));
                throw;
            }
        },
        ndt::Error);
}

TEST_F(SocketTest, BindAlreadyBoundV6MustThrowError)
{
    InSequence seq;
    mDetails->expectSocketSucceded(AF_INET6);
    mDetails->expectBindSucceded(kV6Size);
    mDetails->expectBindFailed(kV6Size);

    ndt::Socket<ndt::UDP, SocketTest> s(ndt::UDP::V6(), 11);

    EXPECT_THROW(
        {
            try
            {
                s.bind(11);
            }
            catch (const ndt::Error &re)
            {
                EXPECT_THAT(re.what(),
                            testing::StartsWith(ndt::exception::kSocketBind));
                throw;
            }
        },
        ndt::Error);
}

TEST_F(SocketTest, CloseNotOpenedSocketV4MustNotThrow)
{
    ndt::Socket<ndt::UDP, SocketTest> s(ndt::UDP::V4());

    EXPECT_NO_THROW(s.close());
    ASSERT_EQ(s.isOpen(), false);
}

TEST_F(SocketTest, CloseNotOpenedSocketV6MustNotThrow)
{
    ndt::Socket<ndt::UDP, SocketTest> s(ndt::UDP::V6());

    EXPECT_NO_THROW(s.close());
    ASSERT_EQ(s.isOpen(), false);
}

TEST_F(SocketTest, SuccessfulCloseV4MustSetIsOpenedFlagToFalse)
{
    InSequence seq;
    mDetails->expectSocketSucceded(AF_INET);
    mDetails->expectBindSucceded(kV4Size);
    mDetails->expectCloseSucceded();

    ndt::Socket<ndt::UDP, SocketTest> s(ndt::UDP::V4(), 11);

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

    ndt::Socket<ndt::UDP, SocketTest> s(ndt::UDP::V6(), 11);

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

    ndt::Socket<ndt::UDP, SocketTest> s(ndt::UDP::V4(), 11);

    ASSERT_EQ(s.isOpen(), true);

    EXPECT_THROW(
        {
            try
            {
                s.close();
            }
            catch (const ndt::Error &re)
            {
                EXPECT_THAT(re.what(),
                            testing::StartsWith(ndt::exception::kSocketClose));
                throw;
            }
        },
        ndt::Error);
}

TEST_F(SocketTest, FailedCloseV6MustThrowError)
{
    InSequence seq;
    mDetails->expectSocketSucceded(AF_INET6);
    mDetails->expectBindSucceded(kV6Size);
    mDetails->expectCloseFailed();

    ndt::Socket<ndt::UDP, SocketTest> s(ndt::UDP::V6(), 11);

    ASSERT_EQ(s.isOpen(), true);

    EXPECT_THROW(
        {
            try
            {
                s.close();
            }
            catch (const ndt::Error &re)
            {
                EXPECT_THAT(re.what(),
                            testing::StartsWith(ndt::exception::kSocketClose));
                throw;
            }
        },
        ndt::Error);
}

TEST_F(SocketTest, FailedSendToMustThrowError)
{
    InSequence seq;
    mDetails->expectSocketSucceded(AF_INET);
    mDetails->expectBindSucceded(kV4Size);
    mDetails->expectSendToFailed();

    ndt::Socket<ndt::UDP, SocketTest> s(ndt::UDP::V4(), 11);
    ndt::Address dst;
    ndt::cbuf_t buf = nullptr;
    const ndt::dlen_t bufLen = 0;

    EXPECT_THROW(
        {
            try
            {
                s.sendTo(dst, buf, bufLen);
            }
            catch (const ndt::Error &re)
            {
                EXPECT_THAT(re.what(),
                            testing::StartsWith(ndt::exception::kSocketSendTo));
                throw;
            }
        },
        ndt::Error);
}

TEST_F(SocketTest, SuccessfulSendToMustReturnCountBytesSent)
{
    InSequence seq;
    mDetails->expectSocketSucceded(AF_INET);
    mDetails->expectBindSucceded(kV4Size);
    mDetails->expectSendToSucceded();

    ndt::Socket<ndt::UDP, SocketTest> s(ndt::UDP::V4(), 11);
    ndt::Address dst;
    ndt::cbuf_t buf = nullptr;
    const ndt::dlen_t bufLen = 0;
    ASSERT_EQ(s.sendTo(dst, buf, bufLen), kSendtoSucceeded);
}

TEST_F(SocketTest, SuccessfulRecvFromMustReturnCountBytesRecieved)
{
    InSequence seq;
    mDetails->expectSocketSucceded(AF_INET);
    mDetails->expectBindSucceded(kV4Size);
    mDetails->expectRecvFromSucceded();

    ndt::Socket<ndt::UDP, SocketTest> s(ndt::UDP::V4(), 11);
    ndt::Address dst;
    ndt::buf_t buf = nullptr;
    const ndt::dlen_t bufLen = 0;
    ASSERT_EQ(s.recvFrom(buf, bufLen, dst), kRecvfromSucceeded);
}

TEST_F(SocketTest, FailedRecvFromMustThrowError)
{
    InSequence seq;
    mDetails->expectSocketSucceded(AF_INET);
    mDetails->expectBindSucceded(kV4Size);
    mDetails->expectRecvFromFailed();

    ndt::Socket<ndt::UDP, SocketTest> s(ndt::UDP::V4(), 11);
    ndt::Address dst;
    ndt::buf_t buf = nullptr;
    const ndt::dlen_t bufLen = 0;

    EXPECT_THROW(
        {
            try
            {
                s.recvFrom(buf, bufLen, dst);
            }
            catch (const ndt::Error &re)
            {
                EXPECT_THAT(re.what(), testing::StartsWith(
                                           ndt::exception::kSocketRecvFrom));
                throw;
            }
        },
        ndt::Error);
}