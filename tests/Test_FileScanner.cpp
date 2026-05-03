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

TEST_F(FileScannerTest, ScanDirectory_ReturnsLogAndTxtFiles) {
    const auto results = ScanDirectory(SmallDir());
    EXPECT_FALSE(results.empty());
}

TEST_F(FileScannerTest, ScanDirectory_IncludesLogFiles) {
    const auto results = ScanDirectory(SmallDir());
    EXPECT_TRUE(ContainsFilename(results, "sample.log"sv));
}

TEST_F(FileScannerTest, ScanDirectory_IncludesTxtFiles) {
    const auto results = ScanDirectory(SmallDir());
    EXPECT_TRUE(ContainsFilename(results, "sample.txt"sv));
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

TEST_F(FileScannerTest, ScanDirectory_IncludesUppercaseLogExtension) {
    const auto results = ScanDirectory(SmallDir());
    EXPECT_TRUE(ContainsFilename(results, "sample_upper.LOG"sv));
}

TEST_F(FileScannerTest, ScanDirectory_IncludesUppercaseTxtExtension) {
    const auto results = ScanDirectory(SmallDir());
    EXPECT_TRUE(ContainsFilename(results, "sample_upper.TXT"sv));
}

}  // namespace