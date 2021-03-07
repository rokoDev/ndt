#include <fmt/core.h>
#include <algorithm>
#include <array>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "ndt/address.h"
#include "ndt/exception.h"
#include "test/sys_error_code_getter.h"

inline const char *kSystemCStr = "system";

class AddressTest : public ::testing::Test
{
   public:
    ndt::test::SysErrorCodeGetter sysErrorGetter;
    AddressTest() = default;
    AddressTest(const AddressTest &) = delete;
    AddressTest &operator=(const AddressTest &) = delete;
    AddressTest(AddressTest &&) = delete;
    AddressTest &operator=(AddressTest &&) = delete;
};

TEST_F(AddressTest, DefaultConstructor)
{
    ndt::Address a;

    ASSERT_EQ(a.addressFamily(), ndt::eAddressFamily::kUnspec);
    ASSERT_EQ(a.addressFamilySys(), AF_UNSPEC);
    ASSERT_EQ(a.port(), 0);
    ASSERT_EQ(std::holds_alternative<std::monostate>(a.ip()), true);
    ASSERT_EQ(ndt::utils::memvcmp(a.nativeDataConst(), 0, a.capacity()), true);
}

TEST_F(AddressTest, CopyConstructorEmptySource)
{
    ndt::Address b;
    ndt::Address a(b);

    ASSERT_EQ(a.addressFamily(), ndt::eAddressFamily::kUnspec);
    ASSERT_EQ(a.addressFamilySys(), AF_UNSPEC);
    ASSERT_EQ(a.port(), 0);
    ASSERT_EQ(std::holds_alternative<std::monostate>(a.ip()), true);
    ASSERT_EQ(ndt::utils::memvcmp(a.nativeDataConst(), 0, a.capacity()), true);
    ASSERT_NE(a.nativeDataConst(), b.nativeDataConst());
    ASSERT_EQ(
        std::memcmp(a.nativeDataConst(), b.nativeDataConst(), a.capacity()), 0);
}

TEST_F(AddressTest, CopyConstructorFilledSourceIPv4)
{
    ndt::Address b(ndt::eAddressFamily::kIPv4, 25);
    ndt::Address a(b);

    ASSERT_EQ(a.addressFamily(), ndt::eAddressFamily::kIPv4);
    ASSERT_EQ(a.addressFamilySys(), AF_INET);
    ASSERT_EQ(a.port(), 25);
    ASSERT_EQ(std::holds_alternative<ndt::ipv4_t>(a.ip()), true);
    ASSERT_EQ(std::get<ndt::ipv4_t>(a.ip()), ndt::kIPv4Any);
    ASSERT_NE(a.nativeDataConst(), b.nativeDataConst());
    ASSERT_EQ(
        std::memcmp(a.nativeDataConst(), b.nativeDataConst(), a.capacity()), 0);
}

TEST_F(AddressTest, CopyConstructorFilledSourceIPv6)
{
    ndt::Address b(ndt::eAddressFamily::kIPv6, 1025);
    ndt::Address a(b);

    ASSERT_EQ(a.addressFamily(), ndt::eAddressFamily::kIPv6);
    ASSERT_EQ(a.addressFamilySys(), AF_INET6);
    ASSERT_EQ(a.port(), 1025);
    ASSERT_EQ(std::holds_alternative<ndt::ipv6_t>(a.ip()), true);
    ASSERT_EQ(std::get<ndt::ipv6_t>(a.ip()), ndt::kIPv6Any);
    ASSERT_NE(a.nativeDataConst(), b.nativeDataConst());
    ASSERT_EQ(
        std::memcmp(a.nativeDataConst(), b.nativeDataConst(), a.capacity()), 0);
}

TEST_F(AddressTest, CopyAssignmentEmptySource)
{
    ndt::Address a(ndt::eAddressFamily::kIPv6, 12345);
    ndt::Address b;

    a = b;

    ASSERT_EQ(a.addressFamily(), ndt::eAddressFamily::kUnspec);
    ASSERT_EQ(a.addressFamilySys(), AF_UNSPEC);
    ASSERT_EQ(a.port(), 0);
    ASSERT_EQ(std::holds_alternative<std::monostate>(a.ip()), true);
    ASSERT_EQ(ndt::utils::memvcmp(a.nativeDataConst(), 0, a.capacity()), true);
    ASSERT_NE(a.nativeDataConst(), b.nativeDataConst());
    ASSERT_EQ(
        std::memcmp(a.nativeDataConst(), b.nativeDataConst(), a.capacity()), 0);
    ASSERT_EQ(a, b);
}

TEST_F(AddressTest, CopyAssignmentIPv4Source)
{
    ndt::Address a(ndt::eAddressFamily::kIPv6, 12345);
    ndt::Address b(ndt::eAddressFamily::kIPv4, 123);

    a = b;

    ASSERT_EQ(a.addressFamily(), ndt::eAddressFamily::kIPv4);
    ASSERT_EQ(a.addressFamilySys(), AF_INET);
    ASSERT_EQ(a.port(), 123);
    ASSERT_EQ(std::holds_alternative<ndt::ipv4_t>(a.ip()), true);
    ASSERT_EQ(std::get<ndt::ipv4_t>(a.ip()), ndt::kIPv4Any);
    ASSERT_NE(a.nativeDataConst(), b.nativeDataConst());
    ASSERT_EQ(
        std::memcmp(a.nativeDataConst(), b.nativeDataConst(), a.capacity()), 0);
    ASSERT_EQ(a, b);
}

TEST_F(AddressTest, CopyAssignmentIPv6Source)
{
    ndt::Address a(ndt::eAddressFamily::kIPv4, 12345);
    ndt::Address b(ndt::eAddressFamily::kIPv6, 123);

    a = b;

    ASSERT_EQ(a.addressFamily(), ndt::eAddressFamily::kIPv6);
    ASSERT_EQ(a.addressFamilySys(), AF_INET6);
    ASSERT_EQ(a.port(), 123);
    ASSERT_EQ(std::holds_alternative<ndt::ipv6_t>(a.ip()), true);
    ASSERT_EQ(std::get<ndt::ipv6_t>(a.ip()), ndt::kIPv6Any);
    ASSERT_NE(a.nativeDataConst(), b.nativeDataConst());
    ASSERT_EQ(
        std::memcmp(a.nativeDataConst(), b.nativeDataConst(), a.capacity()), 0);
    ASSERT_EQ(a, b);
}

TEST_F(AddressTest, MoveConstructorEmptySource)
{
    ndt::Address b;
    ndt::Address a(std::move(b));

    ASSERT_EQ(a.addressFamily(), ndt::eAddressFamily::kUnspec);
    ASSERT_EQ(a.addressFamilySys(), AF_UNSPEC);
    ASSERT_EQ(a.port(), 0);
    ASSERT_EQ(std::holds_alternative<std::monostate>(a.ip()), true);
    ASSERT_EQ(ndt::utils::memvcmp(a.nativeDataConst(), 0, a.capacity()), true);
}

TEST_F(AddressTest, MoveAssignmentEmptySource)
{
    ndt::Address b;
    ndt::Address a(ndt::eAddressFamily::kIPv4, 12345);
    a = std::move(b);

    ASSERT_EQ(a.addressFamily(), ndt::eAddressFamily::kUnspec);
    ASSERT_EQ(a.addressFamilySys(), AF_UNSPEC);
    ASSERT_EQ(a.port(), 0);
    ASSERT_EQ(std::holds_alternative<std::monostate>(a.ip()), true);
    ASSERT_EQ(ndt::utils::memvcmp(a.nativeDataConst(), 0, a.capacity()), true);
}

TEST_F(AddressTest, MoveAssignmentIPv4Source)
{
    ndt::Address b(ndt::eAddressFamily::kIPv4, 12345);
    ndt::Address a(ndt::eAddressFamily::kIPv6, 345);
    a = std::move(b);

    ASSERT_EQ(a.addressFamily(), ndt::eAddressFamily::kIPv4);
    ASSERT_EQ(a.addressFamilySys(), AF_INET);
    ASSERT_EQ(a.port(), 12345);
    ASSERT_EQ(std::holds_alternative<ndt::ipv4_t>(a.ip()), true);
    ASSERT_EQ(std::get<ndt::ipv4_t>(a.ip()), ndt::kIPv4Any);
}

TEST_F(AddressTest, MoveAssignmentIPv6Source)
{
    ndt::Address b(ndt::eAddressFamily::kIPv6, 333);
    ndt::Address a(ndt::eAddressFamily::kIPv4, 12345);
    a = std::move(b);

    ASSERT_EQ(a.addressFamily(), ndt::eAddressFamily::kIPv6);
    ASSERT_EQ(a.addressFamilySys(), AF_INET6);
    ASSERT_EQ(a.port(), 333);
    ASSERT_EQ(std::holds_alternative<ndt::ipv6_t>(a.ip()), true);
    ASSERT_EQ(std::get<ndt::ipv6_t>(a.ip()), ndt::kIPv6Any);
}

TEST_F(AddressTest, MoveConstructorIPv4Source)
{
    ndt::Address b(ndt::eAddressFamily::kIPv4, 100);
    ndt::Address a(std::move(b));

    ASSERT_EQ(a.addressFamily(), ndt::eAddressFamily::kIPv4);
    ASSERT_EQ(a.addressFamilySys(), AF_INET);
    ASSERT_EQ(a.port(), 100);
    ASSERT_EQ(std::holds_alternative<ndt::ipv4_t>(a.ip()), true);
    ASSERT_EQ(std::get<ndt::ipv4_t>(a.ip()), ndt::kIPv4Any);
}

TEST_F(AddressTest, MoveConstructorIPv6Source)
{
    ndt::Address b(ndt::eAddressFamily::kIPv6, 100);
    ndt::Address a(std::move(b));

    ASSERT_EQ(a.addressFamily(), ndt::eAddressFamily::kIPv6);
    ASSERT_EQ(a.addressFamilySys(), AF_INET6);
    ASSERT_EQ(a.port(), 100);
    ASSERT_EQ(std::holds_alternative<ndt::ipv6_t>(a.ip()), true);
    ASSERT_EQ(std::get<ndt::ipv6_t>(a.ip()), ndt::kIPv6Any);
}

TEST_F(AddressTest, ConstructorFamilykUnspecThrowLogicError)
{
    EXPECT_THROW(
        {
            try
            {
                ndt::Address a(ndt::eAddressFamily::kUnspec);
            }
            catch (const ndt::Error &e)
            {
                EXPECT_STREQ(e.what(), ndt::kInvalidAddressFamilyDescr.c_str());
                throw;
            }
        },
        ndt::Error);
}

TEST_F(AddressTest, ConstructorFamilykIPv4NotThrow)
{
    const auto constructWithIPv4 = []() {
        ndt::Address a(ndt::eAddressFamily::kIPv4);
    };
    EXPECT_NO_THROW(constructWithIPv4());
}

TEST_F(AddressTest, ConstructorFamilykIPv6NotThrow)
{
    const auto constructWithIPv6 = []() {
        ndt::Address a(ndt::eAddressFamily::kIPv6);
    };
    EXPECT_NO_THROW(constructWithIPv6());
}

TEST_F(AddressTest, ConstructorSysFamilyInvalidThrowLogicError)
{
    EXPECT_THROW(
        {
            try
            {
                std::set<uint8_t> sysFamilies;
                for (const auto &[sysFamily, ignored]:
                     ndt::AddressFamilySystemToUser)
                {
                    sysFamilies.insert(sysFamily);
                }
                const uint8_t maxValue = *sysFamilies.rbegin();
                const uint8_t invalidFamily = maxValue + 1;
                ndt::Address a(invalidFamily);
            }
            catch (const ndt::Error &e)
            {
                EXPECT_THAT(e.what(), testing::StartsWith(
                                          ndt::kAddressUnknownFamilyDescr));
                throw;
            }
        },
        ndt::Error);
}

TEST_F(AddressTest, ConstructorSysFamilyAFUNSPECThrowLogicError)
{
    EXPECT_THROW(
        {
            try
            {
                ndt::Address a(AF_UNSPEC);
            }
            catch (const ndt::Error &e)
            {
                EXPECT_STREQ(e.what(), ndt::kInvalidAddressFamilyDescr.c_str());
                throw;
            }
        },
        ndt::Error);
}

TEST_F(AddressTest, ConstructorSysFamilykIPv4NotThrow)
{
    const auto constructWithIPv4 = []() { ndt::Address a(AF_INET); };
    EXPECT_NO_THROW(constructWithIPv4());
}

TEST_F(AddressTest, ConstructorSysFamilykIPv6NotThrow)
{
    const auto constructWithIPv6 = []() { ndt::Address a(AF_INET6); };
    EXPECT_NO_THROW(constructWithIPv6());
}

TEST_F(AddressTest, ConstructorFamilykIPv4ValidInit)
{
    ndt::Address a(ndt::eAddressFamily::kIPv4);
    ASSERT_EQ(a.port(), 0);
    ASSERT_EQ(a.addressFamily(), ndt::eAddressFamily::kIPv4);
    ASSERT_EQ(a.addressFamilySys(), AF_INET);
    ASSERT_EQ(std::holds_alternative<ndt::ipv4_t>(a.ip()), true);
    ASSERT_EQ(std::get<ndt::ipv4_t>(a.ip()), ndt::kIPv4Any);
}

TEST_F(AddressTest, ConstructorFamilykIPv6ValidInit)
{
    ndt::Address a(ndt::eAddressFamily::kIPv6);
    ASSERT_EQ(a.port(), 0);
    ASSERT_EQ(a.addressFamily(), ndt::eAddressFamily::kIPv6);
    ASSERT_EQ(a.addressFamilySys(), AF_INET6);
    ASSERT_EQ(std::holds_alternative<ndt::ipv6_t>(a.ip()), true);
    ASSERT_EQ(std::get<ndt::ipv6_t>(a.ip()), ndt::kIPv6Any);
}

TEST_F(AddressTest, ConstructorSysFamilyAFINETValidInit)
{
    ndt::Address a(AF_INET);
    ASSERT_EQ(a.port(), 0);
    ASSERT_EQ(a.addressFamily(), ndt::eAddressFamily::kIPv4);
    ASSERT_EQ(a.addressFamilySys(), AF_INET);
    ASSERT_EQ(std::holds_alternative<ndt::ipv4_t>(a.ip()), true);
    ASSERT_EQ(std::get<ndt::ipv4_t>(a.ip()), ndt::kIPv4Any);
}

TEST_F(AddressTest, ConstructorSysFamilyAFINET6ValidInit)
{
    ndt::Address a(AF_INET6);
    ASSERT_EQ(a.port(), 0);
    ASSERT_EQ(a.addressFamily(), ndt::eAddressFamily::kIPv6);
    ASSERT_EQ(a.addressFamilySys(), AF_INET6);
    ASSERT_EQ(std::holds_alternative<ndt::ipv6_t>(a.ip()), true);
    ASSERT_EQ(std::get<ndt::ipv6_t>(a.ip()), ndt::kIPv6Any);
}

TEST_F(AddressTest, ConstructorFamilykUnspecPortThrowLogicError)
{
    EXPECT_THROW(
        {
            try
            {
                ndt::Address a(ndt::eAddressFamily::kUnspec, 111);
            }
            catch (const ndt::Error &e)
            {
                EXPECT_STREQ(e.what(), ndt::kInvalidAddressFamilyDescr.c_str());
                throw;
            }
        },
        ndt::Error);
}

TEST_F(AddressTest, ConstructorFamilykIPv4PortNotThrow)
{
    const auto constructWithIPv4 = []() {
        ndt::Address a(ndt::eAddressFamily::kIPv4, 333);
    };
    EXPECT_NO_THROW(constructWithIPv4());
}

TEST_F(AddressTest, ConstructorFamilykIPv6PortNotThrow)
{
    const auto constructWithIPv6 = []() {
        ndt::Address a(ndt::eAddressFamily::kIPv6, 333);
    };
    EXPECT_NO_THROW(constructWithIPv6());
}

TEST_F(AddressTest, ConstructorFamilykIPv4PortValidInit)
{
    ndt::Address a(ndt::eAddressFamily::kIPv4, 123);
    ASSERT_EQ(a.port(), 123);
    ASSERT_EQ(a.addressFamily(), ndt::eAddressFamily::kIPv4);
    ASSERT_EQ(a.addressFamilySys(), AF_INET);
    ASSERT_EQ(std::holds_alternative<ndt::ipv4_t>(a.ip()), true);
    ASSERT_EQ(std::get<ndt::ipv4_t>(a.ip()), ndt::kIPv4Any);
}

TEST_F(AddressTest, ConstructorFamilykIPv6PortValidInit)
{
    ndt::Address a(ndt::eAddressFamily::kIPv6, 123);
    ASSERT_EQ(a.port(), 123);
    ASSERT_EQ(a.addressFamily(), ndt::eAddressFamily::kIPv6);
    ASSERT_EQ(a.addressFamilySys(), AF_INET6);
    ASSERT_EQ(std::holds_alternative<ndt::ipv6_t>(a.ip()), true);
    ASSERT_EQ(std::get<ndt::ipv6_t>(a.ip()), ndt::kIPv6Any);
}

TEST_F(AddressTest, ConstructorSysFamilyAFUNSPECPortThrowLogicError)
{
    EXPECT_THROW(
        {
            try
            {
                ndt::Address a(AF_UNSPEC, 111);
            }
            catch (const ndt::Error &e)
            {
                EXPECT_STREQ(e.what(), ndt::kInvalidAddressFamilyDescr.c_str());
                throw;
            }
        },
        ndt::Error);
}

TEST_F(AddressTest, ConstructorSysFamilyInvalidPortThrowLogicError)
{
    EXPECT_THROW(
        {
            try
            {
                std::set<uint8_t> sysFamilies;
                for (const auto &[sysFamily, ignored]:
                     ndt::AddressFamilySystemToUser)
                {
                    sysFamilies.insert(sysFamily);
                }
                const uint8_t maxValue = *sysFamilies.rbegin();
                const uint8_t invalidFamily = maxValue + 1;
                ndt::Address a(invalidFamily, 111);
            }
            catch (const ndt::Error &e)
            {
                EXPECT_THAT(e.what(), testing::StartsWith(
                                          ndt::kAddressUnknownFamilyDescr));
                throw;
            }
        },
        ndt::Error);
}

TEST_F(AddressTest, ConstructorSysFamilykIPv4PortNotThrow)
{
    const auto constructWithIPv4 = []() { ndt::Address a(AF_INET, 123); };
    EXPECT_NO_THROW(constructWithIPv4());
}

TEST_F(AddressTest, ConstructorSysFamilykIPv6PortNotThrow)
{
    const auto constructWithIPv6 = []() { ndt::Address a(AF_INET6, 123); };
    EXPECT_NO_THROW(constructWithIPv6());
}

TEST_F(AddressTest, ConstructorSysFamilykIPv4PortValidInit)
{
    ndt::Address a(AF_INET, 123);
    ASSERT_EQ(a.port(), 123);
    ASSERT_EQ(a.addressFamily(), ndt::eAddressFamily::kIPv4);
    ASSERT_EQ(a.addressFamilySys(), AF_INET);
    ASSERT_EQ(std::holds_alternative<ndt::ipv4_t>(a.ip()), true);
    ASSERT_EQ(std::get<ndt::ipv4_t>(a.ip()), ndt::kIPv4Any);
}

TEST_F(AddressTest, ConstructorSysFamilykIPv6PortValidInit)
{
    ndt::Address a(AF_INET6, 123);
    ASSERT_EQ(a.port(), 123);
    ASSERT_EQ(a.addressFamily(), ndt::eAddressFamily::kIPv6);
    ASSERT_EQ(a.addressFamilySys(), AF_INET6);
    ASSERT_EQ(std::holds_alternative<ndt::ipv6_t>(a.ip()), true);
    ASSERT_EQ(std::get<ndt::ipv6_t>(a.ip()), ndt::kIPv6Any);
}

TEST_F(AddressTest, ConstructorIPkIPv4PortValidInit)
{
    ndt::Address a(ndt::kIPv4Loopback, 123);
    ASSERT_EQ(a.port(), 123);
    ASSERT_EQ(a.addressFamily(), ndt::eAddressFamily::kIPv4);
    ASSERT_EQ(a.addressFamilySys(), AF_INET);
    ASSERT_EQ(std::holds_alternative<ndt::ipv4_t>(a.ip()), true);
    ASSERT_EQ(std::get<ndt::ipv4_t>(a.ip()), ndt::kIPv4Loopback);
}

TEST_F(AddressTest, ConstructorIPkIPv6PortValidInit)
{
    ndt::Address a(ndt::kIPv6Loopback, 123);
    ASSERT_EQ(a.port(), 123);
    ASSERT_EQ(a.addressFamily(), ndt::eAddressFamily::kIPv6);
    ASSERT_EQ(a.addressFamilySys(), AF_INET6);
    ASSERT_EQ(std::holds_alternative<ndt::ipv6_t>(a.ip()), true);
    ASSERT_EQ(std::get<ndt::ipv6_t>(a.ip()), ndt::kIPv6Loopback);
}

TEST_F(AddressTest, ConstructorSockaddrWithInvalidFamilyThrowLogicError)
{
    EXPECT_THROW(
        {
            try
            {
                std::set<uint8_t> sysFamilies;
                for (const auto &[sysFamily, ignored]:
                     ndt::AddressFamilySystemToUser)
                {
                    sysFamilies.insert(sysFamily);
                }
                const uint8_t maxValue = *sysFamilies.rbegin();
                const uint8_t invalidFamily = maxValue + 1;
                sockaddr sa;
                sa.sa_family = invalidFamily;
                ndt::Address a(sa);
            }
            catch (const ndt::Error &e)
            {
                EXPECT_STREQ(e.what(), ndt::kAddressUnknownFamilyDescr.c_str());
                throw;
            }
        },
        ndt::Error);
}

TEST_F(AddressTest, ConstructorSockaddrWithAFUNSPECFamilyThrowLogicError)
{
    EXPECT_THROW(
        {
            try
            {
                sockaddr sa;
                sa.sa_family = AF_UNSPEC;
                ndt::Address a(sa);
            }
            catch (const ndt::Error &e)
            {
                EXPECT_STREQ(e.what(), ndt::kInvalidAddressFamilyDescr.c_str());
                throw;
            }
        },
        ndt::Error);
}

TEST_F(AddressTest, ConstructorSockaddrWithAFINETFamilyNotThrow)
{
    const auto constructWithAF_INET = []() {
        sockaddr_in sa4;
        sa4.sin_family = AF_INET;
        const sockaddr *sa = reinterpret_cast<const sockaddr *>(&sa4);
        ndt::Address a(*sa);
    };
    EXPECT_NO_THROW(constructWithAF_INET());
}

TEST_F(AddressTest, ConstructorSockaddrWithAFINET6FamilyNotThrow)
{
    const auto constructWithAF_INET6 = []() {
        sockaddr_in6 sa6;
        sa6.sin6_family = AF_INET6;
        const sockaddr *sa = reinterpret_cast<const sockaddr *>(&sa6);
        ndt::Address a(*sa);
    };
    EXPECT_NO_THROW(constructWithAF_INET6());
}

TEST_F(AddressTest, ConstructorSockaddrWithAFINETFamilyValidInit)
{
    sockaddr_in sa4;
    std::memset(&sa4, 0, sizeof(sa4));
    sa4.sin_port = htons(123);
    sa4.sin_family = AF_INET;
    const sockaddr *sa = reinterpret_cast<const sockaddr *>(&sa4);
    ndt::Address a(*sa);

    ASSERT_EQ(a.port(), 123);
    ASSERT_EQ(a.addressFamily(), ndt::eAddressFamily::kIPv4);
    ASSERT_EQ(a.addressFamilySys(), AF_INET);
    ASSERT_EQ(std::holds_alternative<ndt::ipv4_t>(a.ip()), true);
    ASSERT_EQ(std::get<ndt::ipv4_t>(a.ip()), ndt::kIPv4Any);
}

TEST_F(AddressTest, ConstructorSockaddrWithAFINET6FamilyValidInit)
{
    sockaddr_in6 sa6;
    std::memset(&sa6, 0, sizeof(sa6));
    sa6.sin6_port = htons(321);
    sa6.sin6_family = AF_INET6;
    const sockaddr *sa = reinterpret_cast<const sockaddr *>(&sa6);
    ndt::Address a(*sa);

    ASSERT_EQ(a.port(), 321);
    ASSERT_EQ(a.addressFamily(), ndt::eAddressFamily::kIPv6);
    ASSERT_EQ(a.addressFamilySys(), AF_INET6);
    ASSERT_EQ(std::holds_alternative<ndt::ipv6_t>(a.ip()), true);
    ASSERT_EQ(std::get<ndt::ipv6_t>(a.ip()), ndt::kIPv6Any);
}

TEST_F(AddressTest, AddressFamilySetkUnspecThrowLogicError)
{
    EXPECT_THROW(
        {
            try
            {
                ndt::Address a;
                a.addressFamily(ndt::eAddressFamily::kUnspec);
            }
            catch (const ndt::Error &e)
            {
                EXPECT_STREQ(e.what(), ndt::kInvalidAddressFamilyDescr.c_str());
                throw;
            }
        },
        ndt::Error);
}

TEST_F(AddressTest, AddressFamilySetAFUNSPECThrowLogicError)
{
    EXPECT_THROW(
        {
            try
            {
                ndt::Address a;
                a.addressFamily(AF_UNSPEC);
            }
            catch (const ndt::Error &e)
            {
                EXPECT_STREQ(e.what(), ndt::kInvalidAddressFamilyDescr.c_str());
                throw;
            }
        },
        ndt::Error);
}

TEST_F(AddressTest, AddressFamilySetInvalidThrowLogicError)
{
    EXPECT_THROW(
        {
            try
            {
                ndt::Address a;
                std::set<uint8_t> sysFamilies;
                for (const auto &[sysFamily, ignored]:
                     ndt::AddressFamilySystemToUser)
                {
                    sysFamilies.insert(sysFamily);
                }
                const uint8_t maxValue = *sysFamilies.rbegin();
                const uint8_t invalidFamily = maxValue + 1;
                a.addressFamily(invalidFamily);
            }
            catch (const ndt::Error &e)
            {
                EXPECT_THAT(e.what(), testing::StartsWith(
                                          ndt::kAddressUnknownFamilyDescr));
                throw;
            }
        },
        ndt::Error);
}

TEST_F(AddressTest, AddressFamilyGetSetCorrectness)
{
    ndt::Address a;

    a.addressFamily(ndt::eAddressFamily::kIPv4);
    ASSERT_EQ(a.port(), 0);
    ASSERT_EQ(a.addressFamily(), ndt::eAddressFamily::kIPv4);
    ASSERT_EQ(a.addressFamilySys(), AF_INET);
    ASSERT_EQ(std::holds_alternative<ndt::ipv4_t>(a.ip()), true);
    ASSERT_EQ(std::get<ndt::ipv4_t>(a.ip()), ndt::kIPv4Any);

    a.addressFamily(ndt::eAddressFamily::kIPv6);
    ASSERT_EQ(a.port(), 0);
    ASSERT_EQ(a.addressFamily(), ndt::eAddressFamily::kIPv6);
    ASSERT_EQ(a.addressFamilySys(), AF_INET6);
    ASSERT_EQ(std::holds_alternative<ndt::ipv6_t>(a.ip()), true);
    ASSERT_EQ(std::get<ndt::ipv6_t>(a.ip()), ndt::kIPv6Any);

    a.addressFamily(ndt::eAddressFamily::kIPv4);
    ASSERT_EQ(a.port(), 0);
    ASSERT_EQ(a.addressFamily(), ndt::eAddressFamily::kIPv4);
    ASSERT_EQ(a.addressFamilySys(), AF_INET);
    ASSERT_EQ(std::holds_alternative<ndt::ipv4_t>(a.ip()), true);
    ASSERT_EQ(std::get<ndt::ipv4_t>(a.ip()), ndt::kIPv4Any);

    a.addressFamily(AF_INET6);
    ASSERT_EQ(a.port(), 0);
    ASSERT_EQ(a.addressFamily(), ndt::eAddressFamily::kIPv6);
    ASSERT_EQ(a.addressFamilySys(), AF_INET6);
    ASSERT_EQ(std::holds_alternative<ndt::ipv6_t>(a.ip()), true);
    ASSERT_EQ(std::get<ndt::ipv6_t>(a.ip()), ndt::kIPv6Any);

    a.addressFamily(AF_INET);
    ASSERT_EQ(a.port(), 0);
    ASSERT_EQ(a.addressFamily(), ndt::eAddressFamily::kIPv4);
    ASSERT_EQ(a.addressFamilySys(), AF_INET);
    ASSERT_EQ(std::holds_alternative<ndt::ipv4_t>(a.ip()), true);
    ASSERT_EQ(std::get<ndt::ipv4_t>(a.ip()), ndt::kIPv4Any);
}

TEST_F(AddressTest, ipGetSetCorrectness)
{
    ndt::Address a;

    a.ip(ndt::kIPv4Loopback);
    ASSERT_EQ(a.port(), 0);
    ASSERT_EQ(a.addressFamily(), ndt::eAddressFamily::kIPv4);
    ASSERT_EQ(a.addressFamilySys(), AF_INET);
    ASSERT_EQ(std::holds_alternative<ndt::ipv4_t>(a.ip()), true);
    ASSERT_EQ(std::get<ndt::ipv4_t>(a.ip()), ndt::kIPv4Loopback);

    a.ip(ndt::kIPv6Loopback);
    ASSERT_EQ(a.port(), 0);
    ASSERT_EQ(a.addressFamily(), ndt::eAddressFamily::kIPv6);
    ASSERT_EQ(a.addressFamilySys(), AF_INET6);
    ASSERT_EQ(std::holds_alternative<ndt::ipv6_t>(a.ip()), true);
    ASSERT_EQ(std::get<ndt::ipv6_t>(a.ip()), ndt::kIPv6Loopback);

    a.ip(ndt::kIPv4Any);
    ASSERT_EQ(a.port(), 0);
    ASSERT_EQ(a.addressFamily(), ndt::eAddressFamily::kIPv4);
    ASSERT_EQ(a.addressFamilySys(), AF_INET);
    ASSERT_EQ(std::holds_alternative<ndt::ipv4_t>(a.ip()), true);
    ASSERT_EQ(std::get<ndt::ipv4_t>(a.ip()), ndt::kIPv4Any);
}

TEST_F(AddressTest, portGetSetCorrectness)
{
    ndt::Address a;

    a.port(423);
    ASSERT_EQ(a.port(), 423);
}

TEST_F(AddressTest, resetCorrectness)
{
    ndt::Address a(ndt::kIPv4Loopback, 23);
    a.reset();
    ASSERT_EQ(a.addressFamily(), ndt::eAddressFamily::kUnspec);
    ASSERT_EQ(a.addressFamilySys(), AF_UNSPEC);
    ASSERT_EQ(a.port(), 0);
    ASSERT_EQ(std::holds_alternative<std::monostate>(a.ip()), true);
    ASSERT_EQ(ndt::utils::memvcmp(a.nativeDataConst(), 0, a.capacity()), true);
}

TEST_F(AddressTest, capacityCorrectness)
{
    ndt::Address a;
    ASSERT_EQ(a.capacity(), sizeof(sockaddr_in6));

    ndt::Address b(ndt::eAddressFamily::kIPv4);
    ASSERT_EQ(b.capacity(), sizeof(sockaddr_in));

    ndt::Address c(ndt::eAddressFamily::kIPv4, 1111);
    ASSERT_EQ(c.capacity(), sizeof(sockaddr_in));

    ndt::Address d(ndt::eAddressFamily::kIPv6, 1111);
    ASSERT_EQ(d.capacity(), sizeof(sockaddr_in6));

    ndt::Address e(ndt::kIPv4Loopback, 1111);
    ASSERT_EQ(e.capacity(), sizeof(sockaddr_in));

    ndt::Address f(ndt::kIPv6Loopback, 1111);
    ASSERT_EQ(f.capacity(), sizeof(sockaddr_in6));
}

TEST_F(AddressTest, EqualityOperatorCorrectness)
{
    ndt::Address a;
    ndt::Address b;
    ASSERT_EQ(a, b);

    a.ip(ndt::kIPv6Loopback);
    b.ip(ndt::kIPv4Loopback);
    ASSERT_NE(a, b);

    a.ip(ndt::kIPv4Loopback);
    b.ip(ndt::kIPv4Loopback);
    ASSERT_EQ(a, b);

    a.port(503);
    ASSERT_NE(a, b);

    b.port(503);
    ASSERT_EQ(a, b);

    a.ip(ndt::kIPv6Loopback);
    a.addressFamily(AF_INET6);
    ASSERT_NE(a, b);
    ASSERT_EQ(std::get<ndt::ipv6_t>(a.ip()), ndt::kIPv6Loopback);
    ASSERT_EQ(a.port(), 503);
    ASSERT_EQ(a.addressFamily(), ndt::eAddressFamily::kIPv6);

    b.addressFamily(AF_INET6);
    ASSERT_NE(a, b);
    ASSERT_EQ(b.port(), 503);
    ASSERT_EQ(b.addressFamily(), ndt::eAddressFamily::kIPv6);

    b.ip(ndt::kIPv6Loopback);
    ASSERT_EQ(std::get<ndt::ipv6_t>(b.ip()), ndt::kIPv6Loopback);
    ASSERT_EQ(b.port(), 503);
    ASSERT_EQ(b.addressFamily(), ndt::eAddressFamily::kIPv6);
    ASSERT_EQ(a, b);
}

TEST_F(AddressTest, swapCorrectness)
{
    ndt::Address a(AF_INET6, 112);
    const auto aCopy = a;
    ndt::Address b(ndt::kIPv4Loopback, 345);
    const auto bCopy = b;
    swap(a, b);
    ASSERT_EQ(a, bCopy);
    ASSERT_EQ(b, aCopy);
}

TEST_F(AddressTest, IPv4ToStringNotThrow)
{
    char ipBuf[INET_ADDRSTRLEN];
    auto action = [&ipBuf]() {
        const ndt::Address a(AF_INET);
        a.ipStr(static_cast<ndt::buffer>(ipBuf));
    };

    EXPECT_NO_THROW(action());
    std::string_view ipStr(ipBuf);
    EXPECT_STREQ(ipStr.data(), "0.0.0.0");
}

TEST_F(AddressTest, IPv6ToStringNotThrow)
{
    char ipBuf[INET6_ADDRSTRLEN];
    auto action = [&ipBuf]() {
        const ndt::Address a(AF_INET6);
        a.ipStr(static_cast<ndt::buffer>(ipBuf));
    };

    EXPECT_NO_THROW(action());
    std::string_view ipStr(ipBuf);
    EXPECT_STREQ(ipStr.data(), "::");
}

TEST_F(AddressTest, IPkUnspecToStringThrow)
{
    EXPECT_THROW(
        {
            try
            {
                char ipBuf[INET_ADDRSTRLEN];
                const ndt::Address a;
                a.ipStr(static_cast<ndt::buffer>(ipBuf));
            }
            catch (const ndt::Error &e)
            {
                EXPECT_THAT(e.what(), testing::StartsWith(
                                          ndt::kInvalidAddressFamilyDescr));
                throw;
            }
        },
        ndt::Error);
}

TEST_F(AddressTest, ValidateAddressFamilyUserkUnspec)
{
    const ndt::eAddressFamily af = ndt::eAddressFamily::kUnspec;
    std::error_code ec;
    ndt::Address::validateAddressFamily(af, ec);

    EXPECT_STREQ(ec.category().name(), ndt::kAddressErrorCategoryCStr);
    EXPECT_STREQ(ec.category().message(ec.value()).c_str(),
                 ndt::kInvalidAddressFamilyDescr.c_str());
    ASSERT_EQ(ec.value(),
              static_cast<int>(ndt::eAddressErrorCode::kInvalidAddressFamily));
}

TEST_F(AddressTest, ValidateAddressFamilyUserkIPv4)
{
    const ndt::eAddressFamily af = ndt::eAddressFamily::kIPv4;
    std::error_code ec;
    ndt::Address::validateAddressFamily(af, ec);

    EXPECT_STREQ(ec.category().name(), kSystemCStr);
    ASSERT_EQ(ec.value(), 0);
}

TEST_F(AddressTest, ValidateAddressFamilyUserkIPv6)
{
    const ndt::eAddressFamily af = ndt::eAddressFamily::kIPv6;
    std::error_code ec;
    ndt::Address::validateAddressFamily(af, ec);

    EXPECT_STREQ(ec.category().name(), kSystemCStr);
    ASSERT_EQ(ec.value(), 0);
}

TEST_F(AddressTest, ValidateAddressFamilySystemUNSPEC)
{
    const uint8_t af = AF_UNSPEC;
    std::error_code ec;
    ndt::Address::validateAddressFamily(af, ec);

    EXPECT_STREQ(ec.category().name(), ndt::kAddressErrorCategoryCStr);
    EXPECT_STREQ(ec.category().message(ec.value()).c_str(),
                 ndt::kInvalidAddressFamilyDescr.c_str());
    ASSERT_EQ(ec.value(),
              static_cast<int>(ndt::eAddressErrorCode::kInvalidAddressFamily));
}

TEST_F(AddressTest, ValidateAddressFamilySystemInvalid)
{
    const uint8_t af = AF_APPLETALK;
    std::error_code ec;
    ndt::Address::validateAddressFamily(af, ec);

    EXPECT_STREQ(ec.category().name(), ndt::kAddressErrorCategoryCStr);
    EXPECT_STREQ(ec.category().message(ec.value()).c_str(),
                 ndt::kAddressUnknownFamilyDescr.c_str());
    ASSERT_EQ(
        ec.value(),
        static_cast<int>(ndt::eAddressErrorCode::kAddressUnknownFamilyDescr));
}

TEST_F(AddressTest, ValidateAddressFamilySystemINET)
{
    const uint8_t af = AF_INET;
    std::error_code ec;
    ndt::Address::validateAddressFamily(af, ec);

    EXPECT_STREQ(ec.category().name(), kSystemCStr);
    ASSERT_EQ(ec.value(), 0);
}

TEST_F(AddressTest, ValidateAddressFamilySystemINET6)
{
    const uint8_t af = AF_INET6;
    std::error_code ec;
    ndt::Address::validateAddressFamily(af, ec);

    EXPECT_STREQ(ec.category().name(), kSystemCStr);
    ASSERT_EQ(ec.value(), 0);
}

TEST_F(AddressTest, IpToStrNotThrow)
{
    ndt::Address a;
    char ipResult[INET6_ADDRSTRLEN] = {0};

    const auto action = [&a, &ipResult](const char *ipCStr) {
        a.ip(ipCStr);
        a.ipStr(static_cast<ndt::buffer>(ipResult));
    };

    struct TestDataT
    {
        const char ipCStr[64];
        const char expectedIP[64];
        uint8_t expectedAF;
    };

    constexpr std::size_t kTestCount = 9;
    constexpr std::array<TestDataT, kTestCount> kTestData = {
        TestDataT{"0.111.0.12", "0.111.0.12", AF_INET},
        TestDataT{"0.0.0.0", "0.0.0.0", AF_INET},
        TestDataT{"255.255.255.255", "255.255.255.255", AF_INET},
        TestDataT{"fe80:0000:0000:0000:a299:9bff:fe18:50d1",
                  "fe80::a299:9bff:fe18:50d1", AF_INET6},
        TestDataT{"ff02:0000:0000:0000:0000:0000:0000:0001", "ff02::1",
                  AF_INET6},
        TestDataT{"0000:0000:0000:0000:0000:0000:0000:0001", "::1", AF_INET6},
        TestDataT{"2001:0db8:aaaa:0001:0000:0000:0000:0200",
                  "2001:db8:aaaa:1::200", AF_INET6},
        TestDataT{"2001:0db8:0000:0000:abcd:0000:0000:1234",
                  "2001:db8::abcd:0:0:1234", AF_INET6},
        TestDataT{"2001:db8::abcd:0:0:1234", "2001:db8::abcd:0:0:1234",
                  AF_INET6},
    };

    for (std::size_t i = 0; i < kTestCount; ++i)
    {
        EXPECT_NO_THROW(action(kTestData[i].ipCStr));
        ASSERT_EQ(std::string_view(kTestData[i].expectedIP),
                  std::string_view(ipResult));
        ASSERT_EQ(a.addressFamilySys(), kTestData[i].expectedAF);
    }
}

TEST_F(AddressTest, UnspecIpToStrThrow)
{
    EXPECT_THROW(
        {
            try
            {
                ndt::Address a;
                char ipResult[INET6_ADDRSTRLEN] = {0};
                a.ipStr(static_cast<ndt::buffer>(ipResult));
            }
            catch (const ndt::Error &e)
            {
                EXPECT_THAT(e.what(), testing::StartsWith(
                                          ndt::kInvalidAddressFamilyDescr));
                throw;
            }
        },
        ndt::Error);
}