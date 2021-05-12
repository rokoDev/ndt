#include <fmt/core.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <chrono>

#include "ndt/address.h"
#include "ndt/buffer.h"

class PacketStack
{
   public:
    PacketStack(const std::size_t aMaxSize = 4096) {}

    void add(const std::chrono::microseconds &aDelay,
             const ndt::Address &aSender, ndt::CBuffer &aData) noexcept
    {
    }

    void add(const ndt::Address &aSender, ndt::CBuffer &aData) noexcept {}

    void get(std::chrono::microseconds &aDelay, const ndt::Address &aSender,
             ndt::CBuffer &aData) const noexcept
    {
    }

   private:
    char rawData_[1024];
};

class PacketStackTest : public ::testing::Test
{
   public:
    PacketStackTest() : cbuf(rawBuf_), buf(rawBuf_){};
    PacketStackTest(const PacketStackTest &) = delete;
    PacketStackTest &operator=(const PacketStackTest &) = delete;
    PacketStackTest(PacketStackTest &&) = delete;
    PacketStackTest &operator=(PacketStackTest &&) = delete;

   protected:
    static void SetUpTestSuite() {}

    static void TearDownTestSuite() {}

    PacketStack stack;
    char rawBuf_[1024];
    ndt::CBuffer cbuf;
    ndt::Buffer buf;
};

TEST(PacketStackTest, Get) {}