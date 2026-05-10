#include "io/FileScanner.hpp"

#include <gtest/gtest.h>

#include <algorithm>
#include <filesystem>
#include <stdexcept>
#include <string>
#include <string_view>
#include <tuple>
#include <vector>

namespace {

using namespace io;
using namespace std::string_view_literals;

class FileScannerTest : public ::testing::Test {
 protected:
    static constexpr std::string_view kDataDir = TEST_DATA_DIR;

    static const std::filesystem::path& SmallDir() {
        static const std::filesystem::path kPath =
            std::filesystem::path{std::string{kDataDir}} / "small"sv;
        return kPath;
    }

    static const std::filesystem::path& MediumDir() {
        static const std::filesystem::path kPath =
            std::filesystem::path{std::string{kDataDir}} / "medium"sv;
        return kPath;
    }

    static const std::filesystem::path& EdgeCasesDir() {
        static const std::filesystem::path kPath =
            std::filesystem::path{std::string{kDataDir}} / "edge_cases"sv;
        return kPath;
    }

    [[nodiscard]] static bool ContainsFilename(
        const std::vector<std::filesystem::path>& paths,
        std::string_view filename) noexcept {
        return std::ranges::any_of(paths, [filename](const auto& p) {
            return p.filename().string() == filename;
        });
    }
};

struct ExtensionCase {
    std::string_view case_name;
    std::string_view filename;
    bool should_exist;
};

class FileScannerExtensionParameterizedTest
    : public FileScannerTest,
      public ::testing::WithParamInterface<ExtensionCase> {};

TEST_F(FileScannerTest, ScanDirectory_ReturnsLogAndTxtFiles) {
    const auto results = ScanDirectory(SmallDir());
    EXPECT_FALSE(results.empty());
}

TEST_F(FileScannerTest, ScanDirectory_ExcludesNonSupportedExtensions) {
    const auto results = ScanDirectory(SmallDir());
    EXPECT_FALSE(ContainsFilename(results, "ignored.csv"sv));
}

TEST_F(FileScannerTest, ScanDirectory_ThrowsForNonExistentDirectory) {
    EXPECT_THROW(std::ignore = ScanDirectory("/non/existent/path"),
                 std::invalid_argument);
}

TEST_F(FileScannerTest, ScanDirectory_ThrowsWhenPathIsFile) {
    const auto file_path = SmallDir() / "sample.log"sv;
    EXPECT_THROW(std::ignore = ScanDirectory(file_path), std::invalid_argument);
}

TEST_F(FileScannerTest, ScanDirectory_ThrowsForEmptyPath) {
    EXPECT_THROW(std::ignore = ScanDirectory({}), std::invalid_argument);
}

TEST_F(FileScannerTest, ScanDirectory_ReturnsEmptyWhenNoMatchingFiles) {
    const auto results = ScanDirectory(EdgeCasesDir());
    EXPECT_TRUE(results.empty());
}

TEST_F(FileScannerTest, ScanDirectory_FindsFilesRecursively) {
    const auto results = ScanDirectory(MediumDir());
    EXPECT_TRUE(ContainsFilename(results, "nested.log"sv));
}

TEST_P(FileScannerExtensionParameterizedTest,
       ScanDirectory_ExtensionVariants_FindsExpectedFiles) {
    const auto results = ScanDirectory(SmallDir());
    const auto [case_name, filename, should_exist] = GetParam();
    (void)case_name;
    EXPECT_EQ(ContainsFilename(results, filename), should_exist);
}

INSTANTIATE_TEST_SUITE_P(
    ExtensionCoverage, FileScannerExtensionParameterizedTest,
    ::testing::Values(ExtensionCase{"LowerLog"sv, "sample.log"sv, true},
                      ExtensionCase{"LowerTxt"sv, "sample.txt"sv, true},
                      ExtensionCase{"UpperLog"sv, "sample_upper.LOG"sv, true},
                      ExtensionCase{"UpperTxt"sv, "sample_upper.TXT"sv, true},
                      ExtensionCase{"UnsupportedCsv"sv, "ignored.csv"sv,
                                    false}),
    [](const auto& param_info) {
        return std::string{param_info.param.case_name};
    });

}  // namespace