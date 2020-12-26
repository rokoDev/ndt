#include <fmt/core.h>
#include <algorithm>
#include <array>

#include "Address.h"
#include "NetException.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

TEST(AddressTests, DefaultConstructor)
{
    net::Address a;

    ASSERT_EQ(a.addressFamily(), net::eAddressFamily::kUnspec);
    ASSERT_EQ(a.addressFamilySys(), AF_UNSPEC);
    ASSERT_EQ(a.port(), 0);
    ASSERT_EQ(std::holds_alternative<std::monostate>(a.ip()), true);
    ASSERT_EQ(net::utils::memvcmp(a.nativeDataConst(), 0, a.capacity()), true);
}

TEST(AddressTests, CopyConstructorEmptySource)
{
    net::Address b;
    net::Address a(b);

    ASSERT_EQ(a.addressFamily(), net::eAddressFamily::kUnspec);
    ASSERT_EQ(a.addressFamilySys(), AF_UNSPEC);
    ASSERT_EQ(a.port(), 0);
    ASSERT_EQ(std::holds_alternative<std::monostate>(a.ip()), true);
    ASSERT_EQ(net::utils::memvcmp(a.nativeDataConst(), 0, a.capacity()), true);
    ASSERT_NE(a.nativeDataConst(), b.nativeDataConst());
    ASSERT_EQ(
        std::memcmp(a.nativeDataConst(), b.nativeDataConst(), a.capacity()), 0);
}

TEST(AddressTests, CopyConstructorFilledSourceIPv4)
{
    net::Address b(net::eAddressFamily::kIPv4, 25);
    net::Address a(b);

    ASSERT_EQ(a.addressFamily(), net::eAddressFamily::kIPv4);
    ASSERT_EQ(a.addressFamilySys(), AF_INET);
    ASSERT_EQ(a.port(), 25);
    ASSERT_EQ(std::holds_alternative<net::ipv4_t>(a.ip()), true);
    ASSERT_EQ(std::get<net::ipv4_t>(a.ip()), net::kIPv4Any);
    ASSERT_NE(a.nativeDataConst(), b.nativeDataConst());
    ASSERT_EQ(
        std::memcmp(a.nativeDataConst(), b.nativeDataConst(), a.capacity()), 0);
}

TEST(AddressTests, CopyConstructorFilledSourceIPv6)
{
    net::Address b(net::eAddressFamily::kIPv6, 1025);
    net::Address a(b);

    ASSERT_EQ(a.addressFamily(), net::eAddressFamily::kIPv6);
    ASSERT_EQ(a.addressFamilySys(), AF_INET6);
    ASSERT_EQ(a.port(), 1025);
    ASSERT_EQ(std::holds_alternative<net::ipv6_t>(a.ip()), true);
    ASSERT_EQ(std::get<net::ipv6_t>(a.ip()), net::kIPv6Any);
    ASSERT_NE(a.nativeDataConst(), b.nativeDataConst());
    ASSERT_EQ(
        std::memcmp(a.nativeDataConst(), b.nativeDataConst(), a.capacity()), 0);
}

TEST(AddressTests, CopyAssignmentEmptySource)
{
    net::Address a(net::eAddressFamily::kIPv6, 12345);
    net::Address b;

    a = b;

    ASSERT_EQ(a.addressFamily(), net::eAddressFamily::kUnspec);
    ASSERT_EQ(a.addressFamilySys(), AF_UNSPEC);
    ASSERT_EQ(a.port(), 0);
    ASSERT_EQ(std::holds_alternative<std::monostate>(a.ip()), true);
    ASSERT_EQ(net::utils::memvcmp(a.nativeDataConst(), 0, a.capacity()), true);
    ASSERT_NE(a.nativeDataConst(), b.nativeDataConst());
    ASSERT_EQ(
        std::memcmp(a.nativeDataConst(), b.nativeDataConst(), a.capacity()), 0);
    ASSERT_EQ(a, b);
}

TEST(AddressTests, CopyAssignmentIPv4Source)
{
    net::Address a(net::eAddressFamily::kIPv6, 12345);
    net::Address b(net::eAddressFamily::kIPv4, 123);

    a = b;

    ASSERT_EQ(a.addressFamily(), net::eAddressFamily::kIPv4);
    ASSERT_EQ(a.addressFamilySys(), AF_INET);
    ASSERT_EQ(a.port(), 123);
    ASSERT_EQ(std::holds_alternative<net::ipv4_t>(a.ip()), true);
    ASSERT_EQ(std::get<net::ipv4_t>(a.ip()), net::kIPv4Any);
    ASSERT_NE(a.nativeDataConst(), b.nativeDataConst());
    ASSERT_EQ(
        std::memcmp(a.nativeDataConst(), b.nativeDataConst(), a.capacity()), 0);
    ASSERT_EQ(a, b);
}

TEST(AddressTests, CopyAssignmentIPv6Source)
{
    net::Address a(net::eAddressFamily::kIPv4, 12345);
    net::Address b(net::eAddressFamily::kIPv6, 123);

    a = b;

    ASSERT_EQ(a.addressFamily(), net::eAddressFamily::kIPv6);
    ASSERT_EQ(a.addressFamilySys(), AF_INET6);
    ASSERT_EQ(a.port(), 123);
    ASSERT_EQ(std::holds_alternative<net::ipv6_t>(a.ip()), true);
    ASSERT_EQ(std::get<net::ipv6_t>(a.ip()), net::kIPv6Any);
    ASSERT_NE(a.nativeDataConst(), b.nativeDataConst());
    ASSERT_EQ(
        std::memcmp(a.nativeDataConst(), b.nativeDataConst(), a.capacity()), 0);
    ASSERT_EQ(a, b);
}

TEST(AddressTests, MoveConstructorEmptySource)
{
    net::Address b;
    net::Address a(std::move(b));

    ASSERT_EQ(a.addressFamily(), net::eAddressFamily::kUnspec);
    ASSERT_EQ(a.addressFamilySys(), AF_UNSPEC);
    ASSERT_EQ(a.port(), 0);
    ASSERT_EQ(std::holds_alternative<std::monostate>(a.ip()), true);
    ASSERT_EQ(net::utils::memvcmp(a.nativeDataConst(), 0, a.capacity()), true);
}

TEST(AddressTests, MoveAssignmentEmptySource)
{
    net::Address b;
    net::Address a(net::eAddressFamily::kIPv4, 12345);
    a = std::move(b);

    ASSERT_EQ(a.addressFamily(), net::eAddressFamily::kUnspec);
    ASSERT_EQ(a.addressFamilySys(), AF_UNSPEC);
    ASSERT_EQ(a.port(), 0);
    ASSERT_EQ(std::holds_alternative<std::monostate>(a.ip()), true);
    ASSERT_EQ(net::utils::memvcmp(a.nativeDataConst(), 0, a.capacity()), true);
}

TEST(AddressTests, MoveAssignmentIPv4Source)
{
    net::Address b(net::eAddressFamily::kIPv4, 12345);
    net::Address a(net::eAddressFamily::kIPv6, 345);
    a = std::move(b);

    ASSERT_EQ(a.addressFamily(), net::eAddressFamily::kIPv4);
    ASSERT_EQ(a.addressFamilySys(), AF_INET);
    ASSERT_EQ(a.port(), 12345);
    ASSERT_EQ(std::holds_alternative<net::ipv4_t>(a.ip()), true);
    ASSERT_EQ(std::get<net::ipv4_t>(a.ip()), net::kIPv4Any);
}

TEST(AddressTests, MoveAssignmentIPv6Source)
{
    net::Address b(net::eAddressFamily::kIPv6, 333);
    net::Address a(net::eAddressFamily::kIPv4, 12345);
    a = std::move(b);

    ASSERT_EQ(a.addressFamily(), net::eAddressFamily::kIPv6);
    ASSERT_EQ(a.addressFamilySys(), AF_INET6);
    ASSERT_EQ(a.port(), 333);
    ASSERT_EQ(std::holds_alternative<net::ipv6_t>(a.ip()), true);
    ASSERT_EQ(std::get<net::ipv6_t>(a.ip()), net::kIPv6Any);
}

TEST(AddressTests, MoveConstructorIPv4Source)
{
    net::Address b(net::eAddressFamily::kIPv4, 100);
    net::Address a(std::move(b));

    ASSERT_EQ(a.addressFamily(), net::eAddressFamily::kIPv4);
    ASSERT_EQ(a.addressFamilySys(), AF_INET);
    ASSERT_EQ(a.port(), 100);
    ASSERT_EQ(std::holds_alternative<net::ipv4_t>(a.ip()), true);
    ASSERT_EQ(std::get<net::ipv4_t>(a.ip()), net::kIPv4Any);
}

TEST(AddressTests, MoveConstructorIPv6Source)
{
    net::Address b(net::eAddressFamily::kIPv6, 100);
    net::Address a(std::move(b));

    ASSERT_EQ(a.addressFamily(), net::eAddressFamily::kIPv6);
    ASSERT_EQ(a.addressFamilySys(), AF_INET6);
    ASSERT_EQ(a.port(), 100);
    ASSERT_EQ(std::holds_alternative<net::ipv6_t>(a.ip()), true);
    ASSERT_EQ(std::get<net::ipv6_t>(a.ip()), net::kIPv6Any);
}

TEST(AddressTests, ConstructorFamilykUnspecThrowLogicError)
{
    EXPECT_THROW(
        {
            try
            {
                net::Address a(net::eAddressFamily::kUnspec);
            }
            catch (const net::exception::LogicError &le)
            {
                EXPECT_STREQ(
                    le.what(),
                    net::exception::kOnlyIPv4OrkIPv6FamilyAllowed.c_str());
                throw;
            }
        },
        net::exception::LogicError);
}

TEST(AddressTests, ConstructorFamilykIPv4NotThrow)
{
    const auto constructWithIPv4 = []() {
        net::Address a(net::eAddressFamily::kIPv4);
    };
    EXPECT_NO_THROW(constructWithIPv4());
}

TEST(AddressTests, ConstructorFamilykIPv6NotThrow)
{
    const auto constructWithIPv6 = []() {
        net::Address a(net::eAddressFamily::kIPv6);
    };
    EXPECT_NO_THROW(constructWithIPv6());
}

TEST(AddressTests, ConstructorSysFamilyInvalidThrowLogicError)
{
    EXPECT_THROW(
        {
            try
            {
                std::set<uint8_t> sysFamilies;
                for (const auto &[sysFamily, ignored]:
                     net::AddressFamilySystemToUser)
                {
                    sysFamilies.insert(sysFamily);
                }
                const uint8_t maxValue = *sysFamilies.rbegin();
                const uint8_t invalidFamily = maxValue + 1;
                net::Address a(invalidFamily);
            }
            catch (const net::exception::LogicError &le)
            {
                EXPECT_THAT(
                    le.what(),
                    testing::StartsWith(net::exception::kUnknownAddressFamily));
                throw;
            }
        },
        net::exception::LogicError);
}

TEST(AddressTests, ConstructorSysFamilyAFUNSPECThrowLogicError)
{
    EXPECT_THROW(
        {
            try
            {
                net::Address a(AF_UNSPEC);
            }
            catch (const net::exception::LogicError &le)
            {
                EXPECT_STREQ(
                    le.what(),
                    net::exception::kOnlyIPv4OrkIPv6FamilyAllowed.c_str());
                throw;
            }
        },
        net::exception::LogicError);
}

TEST(AddressTests, ConstructorSysFamilykIPv4NotThrow)
{
    const auto constructWithIPv4 = []() { net::Address a(AF_INET); };
    EXPECT_NO_THROW(constructWithIPv4());
}

TEST(AddressTests, ConstructorSysFamilykIPv6NotThrow)
{
    const auto constructWithIPv6 = []() { net::Address a(AF_INET6); };
    EXPECT_NO_THROW(constructWithIPv6());
}

TEST(AddressTests, ConstructorFamilykIPv4ValidInit)
{
    net::Address a(net::eAddressFamily::kIPv4);
    ASSERT_EQ(a.port(), 0);
    ASSERT_EQ(a.addressFamily(), net::eAddressFamily::kIPv4);
    ASSERT_EQ(a.addressFamilySys(), AF_INET);
    ASSERT_EQ(std::holds_alternative<net::ipv4_t>(a.ip()), true);
    ASSERT_EQ(std::get<net::ipv4_t>(a.ip()), net::kIPv4Any);
}

TEST(AddressTests, ConstructorFamilykIPv6ValidInit)
{
    net::Address a(net::eAddressFamily::kIPv6);
    ASSERT_EQ(a.port(), 0);
    ASSERT_EQ(a.addressFamily(), net::eAddressFamily::kIPv6);
    ASSERT_EQ(a.addressFamilySys(), AF_INET6);
    ASSERT_EQ(std::holds_alternative<net::ipv6_t>(a.ip()), true);
    ASSERT_EQ(std::get<net::ipv6_t>(a.ip()), net::kIPv6Any);
}

TEST(AddressTests, ConstructorSysFamilyAFINETValidInit)
{
    net::Address a(AF_INET);
    ASSERT_EQ(a.port(), 0);
    ASSERT_EQ(a.addressFamily(), net::eAddressFamily::kIPv4);
    ASSERT_EQ(a.addressFamilySys(), AF_INET);
    ASSERT_EQ(std::holds_alternative<net::ipv4_t>(a.ip()), true);
    ASSERT_EQ(std::get<net::ipv4_t>(a.ip()), net::kIPv4Any);
}

TEST(AddressTests, ConstructorSysFamilyAFINET6ValidInit)
{
    net::Address a(AF_INET6);
    ASSERT_EQ(a.port(), 0);
    ASSERT_EQ(a.addressFamily(), net::eAddressFamily::kIPv6);
    ASSERT_EQ(a.addressFamilySys(), AF_INET6);
    ASSERT_EQ(std::holds_alternative<net::ipv6_t>(a.ip()), true);
    ASSERT_EQ(std::get<net::ipv6_t>(a.ip()), net::kIPv6Any);
}

TEST(AddressTests, ConstructorFamilykUnspecPortThrowLogicError)
{
    EXPECT_THROW(
        {
            try
            {
                net::Address a(net::eAddressFamily::kUnspec, 111);
            }
            catch (const net::exception::LogicError &le)
            {
                EXPECT_STREQ(
                    le.what(),
                    net::exception::kOnlyIPv4OrkIPv6FamilyAllowed.c_str());
                throw;
            }
        },
        net::exception::LogicError);
}

TEST(AddressTests, ConstructorFamilykIPv4PortNotThrow)
{
    const auto constructWithIPv4 = []() {
        net::Address a(net::eAddressFamily::kIPv4, 333);
    };
    EXPECT_NO_THROW(constructWithIPv4());
}

TEST(AddressTests, ConstructorFamilykIPv6PortNotThrow)
{
    const auto constructWithIPv6 = []() {
        net::Address a(net::eAddressFamily::kIPv6, 333);
    };
    EXPECT_NO_THROW(constructWithIPv6());
}

TEST(AddressTests, ConstructorFamilykIPv4PortValidInit)
{
    net::Address a(net::eAddressFamily::kIPv4, 123);
    ASSERT_EQ(a.port(), 123);
    ASSERT_EQ(a.addressFamily(), net::eAddressFamily::kIPv4);
    ASSERT_EQ(a.addressFamilySys(), AF_INET);
    ASSERT_EQ(std::holds_alternative<net::ipv4_t>(a.ip()), true);
    ASSERT_EQ(std::get<net::ipv4_t>(a.ip()), net::kIPv4Any);
}

TEST(AddressTests, ConstructorFamilykIPv6PortValidInit)
{
    net::Address a(net::eAddressFamily::kIPv6, 123);
    ASSERT_EQ(a.port(), 123);
    ASSERT_EQ(a.addressFamily(), net::eAddressFamily::kIPv6);
    ASSERT_EQ(a.addressFamilySys(), AF_INET6);
    ASSERT_EQ(std::holds_alternative<net::ipv6_t>(a.ip()), true);
    ASSERT_EQ(std::get<net::ipv6_t>(a.ip()), net::kIPv6Any);
}

TEST(AddressTests, ConstructorSysFamilyAFUNSPECPortThrowLogicError)
{
    EXPECT_THROW(
        {
            try
            {
                net::Address a(AF_UNSPEC, 111);
            }
            catch (const net::exception::LogicError &le)
            {
                EXPECT_STREQ(
                    le.what(),
                    net::exception::kOnlyIPv4OrkIPv6FamilyAllowed.c_str());
                throw;
            }
        },
        net::exception::LogicError);
}

TEST(AddressTests, ConstructorSysFamilyInvalidPortThrowLogicError)
{
    EXPECT_THROW(
        {
            try
            {
                std::set<uint8_t> sysFamilies;
                for (const auto &[sysFamily, ignored]:
                     net::AddressFamilySystemToUser)
                {
                    sysFamilies.insert(sysFamily);
                }
                const uint8_t maxValue = *sysFamilies.rbegin();
                const uint8_t invalidFamily = maxValue + 1;
                net::Address a(invalidFamily, 111);
            }
            catch (const net::exception::LogicError &le)
            {
                EXPECT_THAT(
                    le.what(),
                    testing::StartsWith(net::exception::kUnknownAddressFamily));
                throw;
            }
        },
        net::exception::LogicError);
}

TEST(AddressTests, ConstructorSysFamilykIPv4PortNotThrow)
{
    const auto constructWithIPv4 = []() { net::Address a(AF_INET, 123); };
    EXPECT_NO_THROW(constructWithIPv4());
}

TEST(AddressTests, ConstructorSysFamilykIPv6PortNotThrow)
{
    const auto constructWithIPv6 = []() { net::Address a(AF_INET6, 123); };
    EXPECT_NO_THROW(constructWithIPv6());
}

TEST(AddressTests, ConstructorSysFamilykIPv4PortValidInit)
{
    net::Address a(AF_INET, 123);
    ASSERT_EQ(a.port(), 123);
    ASSERT_EQ(a.addressFamily(), net::eAddressFamily::kIPv4);
    ASSERT_EQ(a.addressFamilySys(), AF_INET);
    ASSERT_EQ(std::holds_alternative<net::ipv4_t>(a.ip()), true);
    ASSERT_EQ(std::get<net::ipv4_t>(a.ip()), net::kIPv4Any);
}

TEST(AddressTests, ConstructorSysFamilykIPv6PortValidInit)
{
    net::Address a(AF_INET6, 123);
    ASSERT_EQ(a.port(), 123);
    ASSERT_EQ(a.addressFamily(), net::eAddressFamily::kIPv6);
    ASSERT_EQ(a.addressFamilySys(), AF_INET6);
    ASSERT_EQ(std::holds_alternative<net::ipv6_t>(a.ip()), true);
    ASSERT_EQ(std::get<net::ipv6_t>(a.ip()), net::kIPv6Any);
}

TEST(AddressTests, ConstructorIPkIPv4PortValidInit)
{
    net::Address a(net::kIPv4Loopback, 123);
    ASSERT_EQ(a.port(), 123);
    ASSERT_EQ(a.addressFamily(), net::eAddressFamily::kIPv4);
    ASSERT_EQ(a.addressFamilySys(), AF_INET);
    ASSERT_EQ(std::holds_alternative<net::ipv4_t>(a.ip()), true);
    ASSERT_EQ(std::get<net::ipv4_t>(a.ip()), net::kIPv4Loopback);
}

TEST(AddressTests, ConstructorIPkIPv6PortValidInit)
{
    net::Address a(net::kIPv6Loopback, 123);
    ASSERT_EQ(a.port(), 123);
    ASSERT_EQ(a.addressFamily(), net::eAddressFamily::kIPv6);
    ASSERT_EQ(a.addressFamilySys(), AF_INET6);
    ASSERT_EQ(std::holds_alternative<net::ipv6_t>(a.ip()), true);
    ASSERT_EQ(std::get<net::ipv6_t>(a.ip()), net::kIPv6Loopback);
}

TEST(AddressTests, ConstructorSockaddrWithInvalidFamilyThrowLogicError)
{
    EXPECT_THROW(
        {
            try
            {
                std::set<uint8_t> sysFamilies;
                for (const auto &[sysFamily, ignored]:
                     net::AddressFamilySystemToUser)
                {
                    sysFamilies.insert(sysFamily);
                }
                const uint8_t maxValue = *sysFamilies.rbegin();
                const uint8_t invalidFamily = maxValue + 1;
                sockaddr sa;
                sa.sa_family = invalidFamily;
                net::Address a(sa);
            }
            catch (const net::exception::LogicError &le)
            {
                EXPECT_STREQ(
                    le.what(),
                    net::exception::kOnlyIPv4OrkIPv6FamilyAllowed.c_str());
                throw;
            }
        },
        net::exception::LogicError);
}

TEST(AddressTests, ConstructorSockaddrWithAFUNSPECFamilyThrowLogicError)
{
    EXPECT_THROW(
        {
            try
            {
                sockaddr sa;
                sa.sa_family = AF_UNSPEC;
                net::Address a(sa);
            }
            catch (const net::exception::LogicError &le)
            {
                EXPECT_STREQ(
                    le.what(),
                    net::exception::kOnlyIPv4OrkIPv6FamilyAllowed.c_str());
                throw;
            }
        },
        net::exception::LogicError);
}

TEST(AddressTests, ConstructorSockaddrWithAFINETFamilyNotThrow)
{
    const auto constructWithAF_INET = []() {
        sockaddr_in sa4;
        sa4.sin_family = AF_INET;
        const sockaddr *sa = reinterpret_cast<const sockaddr *>(&sa4);
        net::Address a(*sa);
    };
    EXPECT_NO_THROW(constructWithAF_INET());
}

TEST(AddressTests, ConstructorSockaddrWithAFINET6FamilyNotThrow)
{
    const auto constructWithAF_INET6 = []() {
        sockaddr_in6 sa6;
        sa6.sin6_family = AF_INET6;
        const sockaddr *sa = reinterpret_cast<const sockaddr *>(&sa6);
        net::Address a(*sa);
    };
    EXPECT_NO_THROW(constructWithAF_INET6());
}

TEST(AddressTests, ConstructorSockaddrWithAFINETFamilyValidInit)
{
    sockaddr_in sa4;
    std::memset(&sa4, 0, sizeof(sa4));
    sa4.sin_port = htons(123);
    sa4.sin_family = AF_INET;
    const sockaddr *sa = reinterpret_cast<const sockaddr *>(&sa4);
    net::Address a(*sa);

    ASSERT_EQ(a.port(), 123);
    ASSERT_EQ(a.addressFamily(), net::eAddressFamily::kIPv4);
    ASSERT_EQ(a.addressFamilySys(), AF_INET);
    ASSERT_EQ(std::holds_alternative<net::ipv4_t>(a.ip()), true);
    ASSERT_EQ(std::get<net::ipv4_t>(a.ip()), net::kIPv4Any);
}

TEST(AddressTests, ConstructorSockaddrWithAFINET6FamilyValidInit)
{
    sockaddr_in6 sa6;
    std::memset(&sa6, 0, sizeof(sa6));
    sa6.sin6_port = htons(321);
    sa6.sin6_family = AF_INET6;
    const sockaddr *sa = reinterpret_cast<const sockaddr *>(&sa6);
    net::Address a(*sa);

    ASSERT_EQ(a.port(), 321);
    ASSERT_EQ(a.addressFamily(), net::eAddressFamily::kIPv6);
    ASSERT_EQ(a.addressFamilySys(), AF_INET6);
    ASSERT_EQ(std::holds_alternative<net::ipv6_t>(a.ip()), true);
    ASSERT_EQ(std::get<net::ipv6_t>(a.ip()), net::kIPv6Any);
}

TEST(AddressTests, AddressFamilySetkUnspecThrowLogicError)
{
    EXPECT_THROW(
        {
            try
            {
                net::Address a;
                a.addressFamily(net::eAddressFamily::kUnspec);
            }
            catch (const net::exception::LogicError &le)
            {
                EXPECT_STREQ(
                    le.what(),
                    net::exception::kOnlyIPv4OrkIPv6FamilyAllowed.c_str());
                throw;
            }
        },
        net::exception::LogicError);
}

TEST(AddressTests, AddressFamilySetAFUNSPECThrowLogicError)
{
    EXPECT_THROW(
        {
            try
            {
                net::Address a;
                a.addressFamily(AF_UNSPEC);
            }
            catch (const net::exception::LogicError &le)
            {
                EXPECT_STREQ(
                    le.what(),
                    net::exception::kOnlyIPv4OrkIPv6FamilyAllowed.c_str());
                throw;
            }
        },
        net::exception::LogicError);
}

TEST(AddressTests, AddressFamilySetInvalidThrowLogicError)
{
    EXPECT_THROW(
        {
            try
            {
                net::Address a;
                std::set<uint8_t> sysFamilies;
                for (const auto &[sysFamily, ignored]:
                     net::AddressFamilySystemToUser)
                {
                    sysFamilies.insert(sysFamily);
                }
                const uint8_t maxValue = *sysFamilies.rbegin();
                const uint8_t invalidFamily = maxValue + 1;
                a.addressFamily(invalidFamily);
            }
            catch (const net::exception::LogicError &le)
            {
                EXPECT_THAT(
                    le.what(),
                    testing::StartsWith(net::exception::kUnknownAddressFamily));
                throw;
            }
        },
        net::exception::LogicError);
}

TEST(AddressTests, AddressFamilyGetSetCorrectness)
{
    net::Address a;

    a.addressFamily(net::eAddressFamily::kIPv4);
    ASSERT_EQ(a.port(), 0);
    ASSERT_EQ(a.addressFamily(), net::eAddressFamily::kIPv4);
    ASSERT_EQ(a.addressFamilySys(), AF_INET);
    ASSERT_EQ(std::holds_alternative<net::ipv4_t>(a.ip()), true);
    ASSERT_EQ(std::get<net::ipv4_t>(a.ip()), net::kIPv4Any);

    a.addressFamily(net::eAddressFamily::kIPv6);
    ASSERT_EQ(a.port(), 0);
    ASSERT_EQ(a.addressFamily(), net::eAddressFamily::kIPv6);
    ASSERT_EQ(a.addressFamilySys(), AF_INET6);
    ASSERT_EQ(std::holds_alternative<net::ipv6_t>(a.ip()), true);
    ASSERT_EQ(std::get<net::ipv6_t>(a.ip()), net::kIPv6Any);

    a.addressFamily(net::eAddressFamily::kIPv4);
    ASSERT_EQ(a.port(), 0);
    ASSERT_EQ(a.addressFamily(), net::eAddressFamily::kIPv4);
    ASSERT_EQ(a.addressFamilySys(), AF_INET);
    ASSERT_EQ(std::holds_alternative<net::ipv4_t>(a.ip()), true);
    ASSERT_EQ(std::get<net::ipv4_t>(a.ip()), net::kIPv4Any);

    a.addressFamily(AF_INET6);
    ASSERT_EQ(a.port(), 0);
    ASSERT_EQ(a.addressFamily(), net::eAddressFamily::kIPv6);
    ASSERT_EQ(a.addressFamilySys(), AF_INET6);
    ASSERT_EQ(std::holds_alternative<net::ipv6_t>(a.ip()), true);
    ASSERT_EQ(std::get<net::ipv6_t>(a.ip()), net::kIPv6Any);

    a.addressFamily(AF_INET);
    ASSERT_EQ(a.port(), 0);
    ASSERT_EQ(a.addressFamily(), net::eAddressFamily::kIPv4);
    ASSERT_EQ(a.addressFamilySys(), AF_INET);
    ASSERT_EQ(std::holds_alternative<net::ipv4_t>(a.ip()), true);
    ASSERT_EQ(std::get<net::ipv4_t>(a.ip()), net::kIPv4Any);
}

TEST(AddressTests, ipGetSetCorrectness)
{
    net::Address a;

    a.ip(net::kIPv4Loopback);
    ASSERT_EQ(a.port(), 0);
    ASSERT_EQ(a.addressFamily(), net::eAddressFamily::kIPv4);
    ASSERT_EQ(a.addressFamilySys(), AF_INET);
    ASSERT_EQ(std::holds_alternative<net::ipv4_t>(a.ip()), true);
    ASSERT_EQ(std::get<net::ipv4_t>(a.ip()), net::kIPv4Loopback);

    a.ip(net::kIPv6Loopback);
    ASSERT_EQ(a.port(), 0);
    ASSERT_EQ(a.addressFamily(), net::eAddressFamily::kIPv6);
    ASSERT_EQ(a.addressFamilySys(), AF_INET6);
    ASSERT_EQ(std::holds_alternative<net::ipv6_t>(a.ip()), true);
    ASSERT_EQ(std::get<net::ipv6_t>(a.ip()), net::kIPv6Loopback);

    a.ip(net::kIPv4Any);
    ASSERT_EQ(a.port(), 0);
    ASSERT_EQ(a.addressFamily(), net::eAddressFamily::kIPv4);
    ASSERT_EQ(a.addressFamilySys(), AF_INET);
    ASSERT_EQ(std::holds_alternative<net::ipv4_t>(a.ip()), true);
    ASSERT_EQ(std::get<net::ipv4_t>(a.ip()), net::kIPv4Any);
}

TEST(AddressTests, portGetSetCorrectness)
{
    net::Address a;

    a.port(423);
    ASSERT_EQ(a.port(), 423);
}

TEST(AddressTests, resetCorrectness)
{
    net::Address a(net::kIPv4Loopback, 23);
    a.reset();
    ASSERT_EQ(a.addressFamily(), net::eAddressFamily::kUnspec);
    ASSERT_EQ(a.addressFamilySys(), AF_UNSPEC);
    ASSERT_EQ(a.port(), 0);
    ASSERT_EQ(std::holds_alternative<std::monostate>(a.ip()), true);
    ASSERT_EQ(net::utils::memvcmp(a.nativeDataConst(), 0, a.capacity()), true);
}

TEST(AddressTests, capacityCorrectness)
{
    net::Address a;
    ASSERT_EQ(a.capacity(), sizeof(sockaddr_in6));

    net::Address b(net::eAddressFamily::kIPv4);
    ASSERT_EQ(b.capacity(), sizeof(sockaddr_in));

    net::Address c(net::eAddressFamily::kIPv4, 1111);
    ASSERT_EQ(c.capacity(), sizeof(sockaddr_in));

    net::Address d(net::eAddressFamily::kIPv6, 1111);
    ASSERT_EQ(d.capacity(), sizeof(sockaddr_in6));

    net::Address e(net::kIPv4Loopback, 1111);
    ASSERT_EQ(e.capacity(), sizeof(sockaddr_in));

    net::Address f(net::kIPv6Loopback, 1111);
    ASSERT_EQ(f.capacity(), sizeof(sockaddr_in6));
}

TEST(AddressTests, EqualityOperatorCorrectness)
{
    net::Address a;
    net::Address b;
    ASSERT_EQ(a, b);

    a.ip(net::kIPv6Loopback);
    b.ip(net::kIPv4Loopback);
    ASSERT_NE(a, b);

    a.ip(net::kIPv4Loopback);
    b.ip(net::kIPv4Loopback);
    ASSERT_EQ(a, b);

    a.port(503);
    ASSERT_NE(a, b);

    b.port(503);
    ASSERT_EQ(a, b);

    a.ip(net::kIPv6Loopback);
    a.addressFamily(AF_INET6);
    ASSERT_NE(a, b);
    ASSERT_EQ(std::get<net::ipv6_t>(a.ip()), net::kIPv6Loopback);
    ASSERT_EQ(a.port(), 503);
    ASSERT_EQ(a.addressFamily(), net::eAddressFamily::kIPv6);

    b.addressFamily(AF_INET6);
    ASSERT_NE(a, b);
    ASSERT_EQ(b.port(), 503);
    ASSERT_EQ(b.addressFamily(), net::eAddressFamily::kIPv6);

    b.ip(net::kIPv6Loopback);
    ASSERT_EQ(std::get<net::ipv6_t>(b.ip()), net::kIPv6Loopback);
    ASSERT_EQ(b.port(), 503);
    ASSERT_EQ(b.addressFamily(), net::eAddressFamily::kIPv6);
    ASSERT_EQ(a, b);
}

TEST(AddressTests, swapCorrectness)
{
    net::Address a(AF_INET6, 112);
    const auto aCopy = a;
    net::Address b(net::kIPv4Loopback, 345);
    const auto bCopy = b;
    swap(a, b);
    ASSERT_EQ(a, bCopy);
    ASSERT_EQ(b, aCopy);
}