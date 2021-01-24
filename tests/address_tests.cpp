#include <fmt/core.h>
#include <algorithm>
#include <array>

#include "ndt/address.h"
#include "ndt/exception.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

TEST(AddressTests, DefaultConstructor)
{
    ndt::Address a;

    ASSERT_EQ(a.addressFamily(), ndt::eAddressFamily::kUnspec);
    ASSERT_EQ(a.addressFamilySys(), AF_UNSPEC);
    ASSERT_EQ(a.port(), 0);
    ASSERT_EQ(std::holds_alternative<std::monostate>(a.ip()), true);
    ASSERT_EQ(ndt::utils::memvcmp(a.nativeDataConst(), 0, a.capacity()), true);
}

TEST(AddressTests, CopyConstructorEmptySource)
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

TEST(AddressTests, CopyConstructorFilledSourceIPv4)
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

TEST(AddressTests, CopyConstructorFilledSourceIPv6)
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

TEST(AddressTests, CopyAssignmentEmptySource)
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

TEST(AddressTests, CopyAssignmentIPv4Source)
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

TEST(AddressTests, CopyAssignmentIPv6Source)
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

TEST(AddressTests, MoveConstructorEmptySource)
{
    ndt::Address b;
    ndt::Address a(std::move(b));

    ASSERT_EQ(a.addressFamily(), ndt::eAddressFamily::kUnspec);
    ASSERT_EQ(a.addressFamilySys(), AF_UNSPEC);
    ASSERT_EQ(a.port(), 0);
    ASSERT_EQ(std::holds_alternative<std::monostate>(a.ip()), true);
    ASSERT_EQ(ndt::utils::memvcmp(a.nativeDataConst(), 0, a.capacity()), true);
}

TEST(AddressTests, MoveAssignmentEmptySource)
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

TEST(AddressTests, MoveAssignmentIPv4Source)
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

TEST(AddressTests, MoveAssignmentIPv6Source)
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

TEST(AddressTests, MoveConstructorIPv4Source)
{
    ndt::Address b(ndt::eAddressFamily::kIPv4, 100);
    ndt::Address a(std::move(b));

    ASSERT_EQ(a.addressFamily(), ndt::eAddressFamily::kIPv4);
    ASSERT_EQ(a.addressFamilySys(), AF_INET);
    ASSERT_EQ(a.port(), 100);
    ASSERT_EQ(std::holds_alternative<ndt::ipv4_t>(a.ip()), true);
    ASSERT_EQ(std::get<ndt::ipv4_t>(a.ip()), ndt::kIPv4Any);
}

TEST(AddressTests, MoveConstructorIPv6Source)
{
    ndt::Address b(ndt::eAddressFamily::kIPv6, 100);
    ndt::Address a(std::move(b));

    ASSERT_EQ(a.addressFamily(), ndt::eAddressFamily::kIPv6);
    ASSERT_EQ(a.addressFamilySys(), AF_INET6);
    ASSERT_EQ(a.port(), 100);
    ASSERT_EQ(std::holds_alternative<ndt::ipv6_t>(a.ip()), true);
    ASSERT_EQ(std::get<ndt::ipv6_t>(a.ip()), ndt::kIPv6Any);
}

TEST(AddressTests, ConstructorFamilykUnspecThrowLogicError)
{
    EXPECT_THROW(
        {
            try
            {
                ndt::Address a(ndt::eAddressFamily::kUnspec);
            }
            catch (const ndt::exception::LogicError &le)
            {
                EXPECT_STREQ(
                    le.what(),
                    ndt::exception::kOnlyIPv4OrkIPv6FamilyAllowed.c_str());
                throw;
            }
        },
        ndt::exception::LogicError);
}

TEST(AddressTests, ConstructorFamilykIPv4NotThrow)
{
    const auto constructWithIPv4 = []() {
        ndt::Address a(ndt::eAddressFamily::kIPv4);
    };
    EXPECT_NO_THROW(constructWithIPv4());
}

TEST(AddressTests, ConstructorFamilykIPv6NotThrow)
{
    const auto constructWithIPv6 = []() {
        ndt::Address a(ndt::eAddressFamily::kIPv6);
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
                     ndt::AddressFamilySystemToUser)
                {
                    sysFamilies.insert(sysFamily);
                }
                const uint8_t maxValue = *sysFamilies.rbegin();
                const uint8_t invalidFamily = maxValue + 1;
                ndt::Address a(invalidFamily);
            }
            catch (const ndt::exception::LogicError &le)
            {
                EXPECT_THAT(
                    le.what(),
                    testing::StartsWith(ndt::exception::kUnknownAddressFamily));
                throw;
            }
        },
        ndt::exception::LogicError);
}

TEST(AddressTests, ConstructorSysFamilyAFUNSPECThrowLogicError)
{
    EXPECT_THROW(
        {
            try
            {
                ndt::Address a(AF_UNSPEC);
            }
            catch (const ndt::exception::LogicError &le)
            {
                EXPECT_STREQ(
                    le.what(),
                    ndt::exception::kOnlyIPv4OrkIPv6FamilyAllowed.c_str());
                throw;
            }
        },
        ndt::exception::LogicError);
}

TEST(AddressTests, ConstructorSysFamilykIPv4NotThrow)
{
    const auto constructWithIPv4 = []() { ndt::Address a(AF_INET); };
    EXPECT_NO_THROW(constructWithIPv4());
}

TEST(AddressTests, ConstructorSysFamilykIPv6NotThrow)
{
    const auto constructWithIPv6 = []() { ndt::Address a(AF_INET6); };
    EXPECT_NO_THROW(constructWithIPv6());
}

TEST(AddressTests, ConstructorFamilykIPv4ValidInit)
{
    ndt::Address a(ndt::eAddressFamily::kIPv4);
    ASSERT_EQ(a.port(), 0);
    ASSERT_EQ(a.addressFamily(), ndt::eAddressFamily::kIPv4);
    ASSERT_EQ(a.addressFamilySys(), AF_INET);
    ASSERT_EQ(std::holds_alternative<ndt::ipv4_t>(a.ip()), true);
    ASSERT_EQ(std::get<ndt::ipv4_t>(a.ip()), ndt::kIPv4Any);
}

TEST(AddressTests, ConstructorFamilykIPv6ValidInit)
{
    ndt::Address a(ndt::eAddressFamily::kIPv6);
    ASSERT_EQ(a.port(), 0);
    ASSERT_EQ(a.addressFamily(), ndt::eAddressFamily::kIPv6);
    ASSERT_EQ(a.addressFamilySys(), AF_INET6);
    ASSERT_EQ(std::holds_alternative<ndt::ipv6_t>(a.ip()), true);
    ASSERT_EQ(std::get<ndt::ipv6_t>(a.ip()), ndt::kIPv6Any);
}

TEST(AddressTests, ConstructorSysFamilyAFINETValidInit)
{
    ndt::Address a(AF_INET);
    ASSERT_EQ(a.port(), 0);
    ASSERT_EQ(a.addressFamily(), ndt::eAddressFamily::kIPv4);
    ASSERT_EQ(a.addressFamilySys(), AF_INET);
    ASSERT_EQ(std::holds_alternative<ndt::ipv4_t>(a.ip()), true);
    ASSERT_EQ(std::get<ndt::ipv4_t>(a.ip()), ndt::kIPv4Any);
}

TEST(AddressTests, ConstructorSysFamilyAFINET6ValidInit)
{
    ndt::Address a(AF_INET6);
    ASSERT_EQ(a.port(), 0);
    ASSERT_EQ(a.addressFamily(), ndt::eAddressFamily::kIPv6);
    ASSERT_EQ(a.addressFamilySys(), AF_INET6);
    ASSERT_EQ(std::holds_alternative<ndt::ipv6_t>(a.ip()), true);
    ASSERT_EQ(std::get<ndt::ipv6_t>(a.ip()), ndt::kIPv6Any);
}

TEST(AddressTests, ConstructorFamilykUnspecPortThrowLogicError)
{
    EXPECT_THROW(
        {
            try
            {
                ndt::Address a(ndt::eAddressFamily::kUnspec, 111);
            }
            catch (const ndt::exception::LogicError &le)
            {
                EXPECT_STREQ(
                    le.what(),
                    ndt::exception::kOnlyIPv4OrkIPv6FamilyAllowed.c_str());
                throw;
            }
        },
        ndt::exception::LogicError);
}

TEST(AddressTests, ConstructorFamilykIPv4PortNotThrow)
{
    const auto constructWithIPv4 = []() {
        ndt::Address a(ndt::eAddressFamily::kIPv4, 333);
    };
    EXPECT_NO_THROW(constructWithIPv4());
}

TEST(AddressTests, ConstructorFamilykIPv6PortNotThrow)
{
    const auto constructWithIPv6 = []() {
        ndt::Address a(ndt::eAddressFamily::kIPv6, 333);
    };
    EXPECT_NO_THROW(constructWithIPv6());
}

TEST(AddressTests, ConstructorFamilykIPv4PortValidInit)
{
    ndt::Address a(ndt::eAddressFamily::kIPv4, 123);
    ASSERT_EQ(a.port(), 123);
    ASSERT_EQ(a.addressFamily(), ndt::eAddressFamily::kIPv4);
    ASSERT_EQ(a.addressFamilySys(), AF_INET);
    ASSERT_EQ(std::holds_alternative<ndt::ipv4_t>(a.ip()), true);
    ASSERT_EQ(std::get<ndt::ipv4_t>(a.ip()), ndt::kIPv4Any);
}

TEST(AddressTests, ConstructorFamilykIPv6PortValidInit)
{
    ndt::Address a(ndt::eAddressFamily::kIPv6, 123);
    ASSERT_EQ(a.port(), 123);
    ASSERT_EQ(a.addressFamily(), ndt::eAddressFamily::kIPv6);
    ASSERT_EQ(a.addressFamilySys(), AF_INET6);
    ASSERT_EQ(std::holds_alternative<ndt::ipv6_t>(a.ip()), true);
    ASSERT_EQ(std::get<ndt::ipv6_t>(a.ip()), ndt::kIPv6Any);
}

TEST(AddressTests, ConstructorSysFamilyAFUNSPECPortThrowLogicError)
{
    EXPECT_THROW(
        {
            try
            {
                ndt::Address a(AF_UNSPEC, 111);
            }
            catch (const ndt::exception::LogicError &le)
            {
                EXPECT_STREQ(
                    le.what(),
                    ndt::exception::kOnlyIPv4OrkIPv6FamilyAllowed.c_str());
                throw;
            }
        },
        ndt::exception::LogicError);
}

TEST(AddressTests, ConstructorSysFamilyInvalidPortThrowLogicError)
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
            catch (const ndt::exception::LogicError &le)
            {
                EXPECT_THAT(
                    le.what(),
                    testing::StartsWith(ndt::exception::kUnknownAddressFamily));
                throw;
            }
        },
        ndt::exception::LogicError);
}

TEST(AddressTests, ConstructorSysFamilykIPv4PortNotThrow)
{
    const auto constructWithIPv4 = []() { ndt::Address a(AF_INET, 123); };
    EXPECT_NO_THROW(constructWithIPv4());
}

TEST(AddressTests, ConstructorSysFamilykIPv6PortNotThrow)
{
    const auto constructWithIPv6 = []() { ndt::Address a(AF_INET6, 123); };
    EXPECT_NO_THROW(constructWithIPv6());
}

TEST(AddressTests, ConstructorSysFamilykIPv4PortValidInit)
{
    ndt::Address a(AF_INET, 123);
    ASSERT_EQ(a.port(), 123);
    ASSERT_EQ(a.addressFamily(), ndt::eAddressFamily::kIPv4);
    ASSERT_EQ(a.addressFamilySys(), AF_INET);
    ASSERT_EQ(std::holds_alternative<ndt::ipv4_t>(a.ip()), true);
    ASSERT_EQ(std::get<ndt::ipv4_t>(a.ip()), ndt::kIPv4Any);
}

TEST(AddressTests, ConstructorSysFamilykIPv6PortValidInit)
{
    ndt::Address a(AF_INET6, 123);
    ASSERT_EQ(a.port(), 123);
    ASSERT_EQ(a.addressFamily(), ndt::eAddressFamily::kIPv6);
    ASSERT_EQ(a.addressFamilySys(), AF_INET6);
    ASSERT_EQ(std::holds_alternative<ndt::ipv6_t>(a.ip()), true);
    ASSERT_EQ(std::get<ndt::ipv6_t>(a.ip()), ndt::kIPv6Any);
}

TEST(AddressTests, ConstructorIPkIPv4PortValidInit)
{
    ndt::Address a(ndt::kIPv4Loopback, 123);
    ASSERT_EQ(a.port(), 123);
    ASSERT_EQ(a.addressFamily(), ndt::eAddressFamily::kIPv4);
    ASSERT_EQ(a.addressFamilySys(), AF_INET);
    ASSERT_EQ(std::holds_alternative<ndt::ipv4_t>(a.ip()), true);
    ASSERT_EQ(std::get<ndt::ipv4_t>(a.ip()), ndt::kIPv4Loopback);
}

TEST(AddressTests, ConstructorIPkIPv6PortValidInit)
{
    ndt::Address a(ndt::kIPv6Loopback, 123);
    ASSERT_EQ(a.port(), 123);
    ASSERT_EQ(a.addressFamily(), ndt::eAddressFamily::kIPv6);
    ASSERT_EQ(a.addressFamilySys(), AF_INET6);
    ASSERT_EQ(std::holds_alternative<ndt::ipv6_t>(a.ip()), true);
    ASSERT_EQ(std::get<ndt::ipv6_t>(a.ip()), ndt::kIPv6Loopback);
}

TEST(AddressTests, ConstructorSockaddrWithInvalidFamilyThrowLogicError)
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
            catch (const ndt::exception::LogicError &le)
            {
                EXPECT_STREQ(
                    le.what(),
                    ndt::exception::kOnlyIPv4OrkIPv6FamilyAllowed.c_str());
                throw;
            }
        },
        ndt::exception::LogicError);
}

TEST(AddressTests, ConstructorSockaddrWithAFUNSPECFamilyThrowLogicError)
{
    EXPECT_THROW(
        {
            try
            {
                sockaddr sa;
                sa.sa_family = AF_UNSPEC;
                ndt::Address a(sa);
            }
            catch (const ndt::exception::LogicError &le)
            {
                EXPECT_STREQ(
                    le.what(),
                    ndt::exception::kOnlyIPv4OrkIPv6FamilyAllowed.c_str());
                throw;
            }
        },
        ndt::exception::LogicError);
}

TEST(AddressTests, ConstructorSockaddrWithAFINETFamilyNotThrow)
{
    const auto constructWithAF_INET = []() {
        sockaddr_in sa4;
        sa4.sin_family = AF_INET;
        const sockaddr *sa = reinterpret_cast<const sockaddr *>(&sa4);
        ndt::Address a(*sa);
    };
    EXPECT_NO_THROW(constructWithAF_INET());
}

TEST(AddressTests, ConstructorSockaddrWithAFINET6FamilyNotThrow)
{
    const auto constructWithAF_INET6 = []() {
        sockaddr_in6 sa6;
        sa6.sin6_family = AF_INET6;
        const sockaddr *sa = reinterpret_cast<const sockaddr *>(&sa6);
        ndt::Address a(*sa);
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
    ndt::Address a(*sa);

    ASSERT_EQ(a.port(), 123);
    ASSERT_EQ(a.addressFamily(), ndt::eAddressFamily::kIPv4);
    ASSERT_EQ(a.addressFamilySys(), AF_INET);
    ASSERT_EQ(std::holds_alternative<ndt::ipv4_t>(a.ip()), true);
    ASSERT_EQ(std::get<ndt::ipv4_t>(a.ip()), ndt::kIPv4Any);
}

TEST(AddressTests, ConstructorSockaddrWithAFINET6FamilyValidInit)
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

TEST(AddressTests, AddressFamilySetkUnspecThrowLogicError)
{
    EXPECT_THROW(
        {
            try
            {
                ndt::Address a;
                a.addressFamily(ndt::eAddressFamily::kUnspec);
            }
            catch (const ndt::exception::LogicError &le)
            {
                EXPECT_STREQ(
                    le.what(),
                    ndt::exception::kOnlyIPv4OrkIPv6FamilyAllowed.c_str());
                throw;
            }
        },
        ndt::exception::LogicError);
}

TEST(AddressTests, AddressFamilySetAFUNSPECThrowLogicError)
{
    EXPECT_THROW(
        {
            try
            {
                ndt::Address a;
                a.addressFamily(AF_UNSPEC);
            }
            catch (const ndt::exception::LogicError &le)
            {
                EXPECT_STREQ(
                    le.what(),
                    ndt::exception::kOnlyIPv4OrkIPv6FamilyAllowed.c_str());
                throw;
            }
        },
        ndt::exception::LogicError);
}

TEST(AddressTests, AddressFamilySetInvalidThrowLogicError)
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
            catch (const ndt::exception::LogicError &le)
            {
                EXPECT_THAT(
                    le.what(),
                    testing::StartsWith(ndt::exception::kUnknownAddressFamily));
                throw;
            }
        },
        ndt::exception::LogicError);
}

TEST(AddressTests, AddressFamilyGetSetCorrectness)
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

TEST(AddressTests, ipGetSetCorrectness)
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

TEST(AddressTests, portGetSetCorrectness)
{
    ndt::Address a;

    a.port(423);
    ASSERT_EQ(a.port(), 423);
}

TEST(AddressTests, resetCorrectness)
{
    ndt::Address a(ndt::kIPv4Loopback, 23);
    a.reset();
    ASSERT_EQ(a.addressFamily(), ndt::eAddressFamily::kUnspec);
    ASSERT_EQ(a.addressFamilySys(), AF_UNSPEC);
    ASSERT_EQ(a.port(), 0);
    ASSERT_EQ(std::holds_alternative<std::monostate>(a.ip()), true);
    ASSERT_EQ(ndt::utils::memvcmp(a.nativeDataConst(), 0, a.capacity()), true);
}

TEST(AddressTests, capacityCorrectness)
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

TEST(AddressTests, EqualityOperatorCorrectness)
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

TEST(AddressTests, swapCorrectness)
{
    ndt::Address a(AF_INET6, 112);
    const auto aCopy = a;
    ndt::Address b(ndt::kIPv4Loopback, 345);
    const auto bCopy = b;
    swap(a, b);
    ASSERT_EQ(a, bCopy);
    ASSERT_EQ(b, aCopy);
}