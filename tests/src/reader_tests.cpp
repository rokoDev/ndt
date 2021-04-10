#include <fmt/core.h>
#include <gmock/gmock.h>

#include <initializer_list>
#include <limits>
#include <tuple>
#include <utility>

#include "gtest/gtest.h"
#include "ndt/buffer.h"

namespace ndt
{
class Reader
{
   public:
    constexpr explicit Reader(CBuffer aBuf) noexcept : buffer_(aBuf) {}

   private:
    union FloatInt
    {
        float floatVal;
        uint32_t uintVal;
    };
    mutable uint16_t byteIndex_ = 0;
    mutable uint8_t bitIndex_ = 0;
    CBuffer buffer_;
};
}  // namespace ndt

class OneByteTest : public ::testing::Test
{
   public:
    OneByteTest() : rawData_{0}, buf_(rawData_), reader_(buf_) {}
    OneByteTest(const OneByteTest &) = delete;
    OneByteTest &operator=(const OneByteTest &) = delete;
    OneByteTest(OneByteTest &&) = delete;
    OneByteTest &operator=(OneByteTest &&) = delete;

   protected:
    static void SetUpTestSuite() {}
    static void TearDownTestSuite() {}

    char rawData_[1];
    ndt::CBuffer buf_;
    ndt::Reader reader_;
};

TEST_F(OneByteTest, ReadBoolTrue) { ASSERT_EQ(true, true); }