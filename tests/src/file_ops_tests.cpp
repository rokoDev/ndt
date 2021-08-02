#include <fmt/core.h>
#include <gtest/gtest.h>

#include <cstring>

#include "ndt/file.h"
#include "ndt/utils.h"

class ExistingFileTest : public ::testing::Test
{
   public:
    ExistingFileTest() = default;
    ExistingFileTest(const ExistingFileTest &) = delete;
    ExistingFileTest &operator=(const ExistingFileTest &) = delete;
    ExistingFileTest(ExistingFileTest &&) = delete;
    ExistingFileTest &operator=(ExistingFileTest &&) = delete;

   protected:
    void SetUp() override
    {
        std::error_code errorCode;
        ndt::File testFile;

        testFile.open<ndt::eFileMode::kBinOpenCreateOverW>(testFilePath_,
                                                           errorCode);
        NDT_LOG_ERROR(errorCode);
        ASSERT_EQ(errorCode.value(), 0);

        testFile.write(fileContent_, errorCode);
        NDT_LOG_ERROR(errorCode);
        ASSERT_EQ(errorCode.value(), 0);

        testFile.close(errorCode);
        NDT_LOG_ERROR(errorCode);
        ASSERT_EQ(errorCode.value(), 0);

        const auto kTestFileSize = ndt::File<>::size(testFilePath_, errorCode);
        ASSERT_EQ(errorCode.value(), 0);
        ASSERT_EQ(kTestFileSize, fileContent_.size());
    }

    void TearDown() override
    {
        std::error_code errorCode;
        ndt::File<>::remove(testFilePath_, errorCode);
        NDT_LOG_ERROR(errorCode);
        ASSERT_EQ(errorCode.value(), 0);
    }

    static inline constexpr char testFilePath_[] = "testfile.txt";
    static inline constexpr std::string_view fileContent_ =
        "Hi! This is content of test file. Happy testing!";
    static inline constexpr std::string_view dataToWrite_ = "some data";
};

TEST_F(ExistingFileTest, kTxtReadExistent)
{
    std::error_code errorCode;
    ndt::File testFile([](const std::error_code &aEc) noexcept {
        NDT_LOG_ERROR(aEc);
        ASSERT_EQ(aEc.value(), 0);
    });
    testFile.open<ndt::eFileMode::kTxtReadExistent>(testFilePath_, errorCode);
    ASSERT_EQ(errorCode.value(), 0);

    char rawBuf[128];
    ndt::Buffer buf(rawBuf);

    testFile.read(buf, errorCode);
    ASSERT_EQ(errorCode.value(), 0);
    ASSERT_EQ(buf.size(), fileContent_.size());
    const int result =
        std::memcmp(buf.data(), fileContent_.data(), fileContent_.size());
    ASSERT_EQ(result, 0);
}

TEST_F(ExistingFileTest, kBinReadExistent)
{
    std::error_code errorCode;
    ndt::File testFile([](const std::error_code &aEc) noexcept {
        NDT_LOG_ERROR(aEc);
        ASSERT_EQ(aEc.value(), 0);
    });
    testFile.open<ndt::eFileMode::kBinReadExistent>(testFilePath_, errorCode);
    ASSERT_EQ(errorCode.value(), 0);

    char rawBuf[128];
    ndt::Buffer buf(rawBuf);

    testFile.read(buf, errorCode);
    ASSERT_EQ(errorCode.value(), 0);
    ASSERT_EQ(buf.size(), fileContent_.size());
    const int result =
        std::memcmp(buf.data(), fileContent_.data(), fileContent_.size());
    ASSERT_EQ(result, 0);
}

TEST_F(ExistingFileTest, kTxtOpenCreateOverW)
{
    std::error_code errorCode;
    {
        ndt::File testFile([](const std::error_code &aEc) noexcept {
            NDT_LOG_ERROR(aEc);
            ASSERT_EQ(aEc.value(), 0);
        });
        testFile.open<ndt::eFileMode::kTxtOpenCreateOverW>(testFilePath_,
                                                           errorCode);
        ASSERT_EQ(errorCode.value(), 0);
    }

    const auto kTestFileSize = ndt::File<>::size(testFilePath_, errorCode);
    ASSERT_EQ(errorCode.value(), 0);
    ASSERT_EQ(kTestFileSize, 0);
}

TEST_F(ExistingFileTest, kBinOpenCreateOverW)
{
    std::error_code errorCode;
    {
        ndt::File testFile([](const std::error_code &aEc) noexcept {
            NDT_LOG_ERROR(aEc);
            ASSERT_EQ(aEc.value(), 0);
        });
        testFile.open<ndt::eFileMode::kBinOpenCreateOverW>(testFilePath_,
                                                           errorCode);
        ASSERT_EQ(errorCode.value(), 0);
    }

    const auto kTestFileSize = ndt::File<>::size(testFilePath_, errorCode);
    ASSERT_EQ(errorCode.value(), 0);
    ASSERT_EQ(kTestFileSize, 0);
}

TEST_F(ExistingFileTest, kTxtOpenCreateAppend)
{
    std::error_code errorCode;
    {
        ndt::File testFile([](const std::error_code &aEc) noexcept {
            NDT_LOG_ERROR(aEc);
            ASSERT_EQ(aEc.value(), 0);
        });
        testFile.open<ndt::eFileMode::kTxtOpenCreateAppend>(testFilePath_,
                                                            errorCode);
        ASSERT_EQ(errorCode.value(), 0);

        testFile.write(dataToWrite_, errorCode);
        ASSERT_EQ(errorCode.value(), 0);
    }

    const auto kTestFileSize = ndt::File<>::size(testFilePath_, errorCode);
    ASSERT_EQ(errorCode.value(), 0);
    ASSERT_EQ(kTestFileSize, dataToWrite_.size() + fileContent_.size());
}

TEST_F(ExistingFileTest, kBinOpenCreateAppend)
{
    std::error_code errorCode;
    {
        ndt::File testFile([](const std::error_code &aEc) noexcept {
            NDT_LOG_ERROR(aEc);
            ASSERT_EQ(aEc.value(), 0);
        });
        testFile.open<ndt::eFileMode::kBinOpenCreateAppend>(testFilePath_,
                                                            errorCode);
        ASSERT_EQ(errorCode.value(), 0);

        testFile.write(dataToWrite_, errorCode);
        ASSERT_EQ(errorCode.value(), 0);
    }

    const auto kTestFileSize = ndt::File<>::size(testFilePath_, errorCode);
    ASSERT_EQ(errorCode.value(), 0);
    ASSERT_EQ(kTestFileSize, dataToWrite_.size() + fileContent_.size());
}

TEST_F(ExistingFileTest, kTxtRWExistentOverwriteBeginning)
{
    std::error_code errorCode;

    ndt::File testFile([](const std::error_code &aEc) noexcept {
        NDT_LOG_ERROR(aEc);
        ASSERT_EQ(aEc.value(), 0);
    });
    testFile.open<ndt::eFileMode::kTxtRWExistent>(testFilePath_, errorCode);
    ASSERT_EQ(errorCode.value(), 0);

    testFile.write(dataToWrite_, errorCode);
    ASSERT_EQ(errorCode.value(), 0);

    testFile.close(errorCode);
    ASSERT_EQ(errorCode.value(), 0);

    testFile.open<ndt::eFileMode::kTxtRWExistent>(testFilePath_, errorCode);
    ASSERT_EQ(errorCode.value(), 0);

    char rawBuf[128];
    ndt::Buffer buf(rawBuf);

    buf.setSize(dataToWrite_.size());
    testFile.read(buf, errorCode);
    ASSERT_EQ(errorCode.value(), 0);
    ASSERT_EQ(buf.size(), dataToWrite_.size());
    {
        const int result =
            std::memcmp(dataToWrite_.data(), buf.data(), dataToWrite_.size());
        ASSERT_EQ(result, 0);
    }

    buf.setSize(sizeof(rawBuf));
    testFile.read(buf, errorCode);
    ASSERT_EQ(errorCode.value(), 0);
    constexpr auto kFileContentSizeMinusWritten =
        fileContent_.size() - dataToWrite_.size();
    ASSERT_EQ(buf.size(), kFileContentSizeMinusWritten);
    {
        const int result =
            std::memcmp(fileContent_.data() + dataToWrite_.size(), buf.data(),
                        kFileContentSizeMinusWritten);
        ASSERT_EQ(result, 0);
    }
}

TEST_F(ExistingFileTest, kBinRWExistentOverwriteBeginning)
{
    std::error_code errorCode;

    ndt::File testFile([](const std::error_code &aEc) noexcept {
        NDT_LOG_ERROR(aEc);
        ASSERT_EQ(aEc.value(), 0);
    });
    testFile.open<ndt::eFileMode::kBinRWExistent>(testFilePath_, errorCode);
    ASSERT_EQ(errorCode.value(), 0);

    testFile.write(dataToWrite_, errorCode);
    ASSERT_EQ(errorCode.value(), 0);

    testFile.close(errorCode);
    ASSERT_EQ(errorCode.value(), 0);

    testFile.open<ndt::eFileMode::kTxtRWExistent>(testFilePath_, errorCode);
    ASSERT_EQ(errorCode.value(), 0);

    char rawBuf[128];
    ndt::Buffer buf(rawBuf);

    buf.setSize(dataToWrite_.size());
    testFile.read(buf, errorCode);
    ASSERT_EQ(errorCode.value(), 0);
    ASSERT_EQ(buf.size(), dataToWrite_.size());
    {
        const int result =
            std::memcmp(dataToWrite_.data(), buf.data(), dataToWrite_.size());
        ASSERT_EQ(result, 0);
    }

    buf.setSize(sizeof(rawBuf));
    testFile.read(buf, errorCode);
    ASSERT_EQ(errorCode.value(), 0);
    constexpr auto kFileContentSizeMinusWritten =
        fileContent_.size() - dataToWrite_.size();
    ASSERT_EQ(buf.size(), kFileContentSizeMinusWritten);
    {
        const int result =
            std::memcmp(fileContent_.data() + dataToWrite_.size(), buf.data(),
                        kFileContentSizeMinusWritten);
        ASSERT_EQ(result, 0);
    }
}

TEST_F(ExistingFileTest, kTxtOpenCreateROverW)
{
    std::error_code errorCode;
    {
        ndt::File testFile([](const std::error_code &aEc) noexcept {
            NDT_LOG_ERROR(aEc);
            ASSERT_EQ(aEc.value(), 0);
        });
        testFile.open<ndt::eFileMode::kTxtOpenCreateROverW>(testFilePath_,
                                                            errorCode);
        ASSERT_EQ(errorCode.value(), 0);

        char rawBuf[128];
        ndt::Buffer buf(rawBuf);

        testFile.read(buf, errorCode);
        ASSERT_EQ(errorCode.value(), 0);
        ASSERT_EQ(buf.size(), 0);

        testFile.write(dataToWrite_, errorCode);
        ASSERT_EQ(errorCode.value(), 0);
    }

    const auto kTestFileSize = ndt::File<>::size(testFilePath_, errorCode);
    ASSERT_EQ(errorCode.value(), 0);
    ASSERT_EQ(kTestFileSize, dataToWrite_.size());
}

TEST_F(ExistingFileTest, kBinOpenCreateROverW)
{
    std::error_code errorCode;
    {
        ndt::File testFile([](const std::error_code &aEc) noexcept {
            NDT_LOG_ERROR(aEc);
            ASSERT_EQ(aEc.value(), 0);
        });
        testFile.open<ndt::eFileMode::kBinOpenCreateROverW>(testFilePath_,
                                                            errorCode);
        ASSERT_EQ(errorCode.value(), 0);

        char rawBuf[128];
        ndt::Buffer buf(rawBuf);

        testFile.read(buf, errorCode);
        ASSERT_EQ(errorCode.value(), 0);
        ASSERT_EQ(buf.size(), 0);

        testFile.write(dataToWrite_, errorCode);
        ASSERT_EQ(errorCode.value(), 0);
    }

    const auto kTestFileSize = ndt::File<>::size(testFilePath_, errorCode);
    ASSERT_EQ(errorCode.value(), 0);
    ASSERT_EQ(kTestFileSize, dataToWrite_.size());
}

TEST_F(ExistingFileTest, kTxtOpenCreateRA)
{
    std::error_code errorCode;
    {
        ndt::File testFile([](const std::error_code &aEc) noexcept {
            NDT_LOG_ERROR(aEc);
            ASSERT_EQ(aEc.value(), 0);
        });
        testFile.open<ndt::eFileMode::kTxtOpenCreateRA>(testFilePath_,
                                                        errorCode);
        ASSERT_EQ(errorCode.value(), 0);

        testFile.write(dataToWrite_, errorCode);
        ASSERT_EQ(errorCode.value(), 0);

        char rawBuf[128];
        ndt::Buffer buf(rawBuf);
        testFile.rewind();
        testFile.read(buf, errorCode);
        ASSERT_EQ(errorCode.value(), 0);
        ASSERT_EQ(buf.size(), fileContent_.size() + dataToWrite_.size());
        {
            const int result = std::memcmp(
                buf.data<void>(), fileContent_.data(), fileContent_.size());
            ASSERT_EQ(result, 0);
        }
        {
            const int result =
                std::memcmp(buf.data<char>() + fileContent_.size(),
                            dataToWrite_.data(), dataToWrite_.size());
            ASSERT_EQ(result, 0);
        }
    }

    const auto kTestFileSize = ndt::File<>::size(testFilePath_, errorCode);
    ASSERT_EQ(errorCode.value(), 0);
    ASSERT_EQ(kTestFileSize, fileContent_.size() + dataToWrite_.size());
}

TEST_F(ExistingFileTest, kBinOpenCreateRA)
{
    std::error_code errorCode;
    {
        ndt::File testFile([](const std::error_code &aEc) noexcept {
            NDT_LOG_ERROR(aEc);
            ASSERT_EQ(aEc.value(), 0);
        });
        testFile.open<ndt::eFileMode::kBinOpenCreateRA>(testFilePath_,
                                                        errorCode);
        ASSERT_EQ(errorCode.value(), 0);

        testFile.write(dataToWrite_, errorCode);
        ASSERT_EQ(errorCode.value(), 0);

        char rawBuf[128];
        ndt::Buffer buf(rawBuf);
        testFile.rewind();
        testFile.read(buf, errorCode);
        ASSERT_EQ(errorCode.value(), 0);
        ASSERT_EQ(buf.size(), fileContent_.size() + dataToWrite_.size());
        {
            const int result = std::memcmp(
                buf.data<void>(), fileContent_.data(), fileContent_.size());
            ASSERT_EQ(result, 0);
        }
        {
            const int result =
                std::memcmp(buf.data<char>() + fileContent_.size(),
                            dataToWrite_.data(), dataToWrite_.size());
            ASSERT_EQ(result, 0);
        }
    }

    const auto kTestFileSize = ndt::File<>::size(testFilePath_, errorCode);
    ASSERT_EQ(errorCode.value(), 0);
    ASSERT_EQ(kTestFileSize, fileContent_.size() + dataToWrite_.size());
}