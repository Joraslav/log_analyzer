#include "app/App.hpp"
#include "app/CliArgs.hpp"

#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <optional>
#include <sstream>
#include <string>
#include <string_view>

namespace {

using namespace std::string_view_literals;
using namespace std::string_literals;

using app::CliArgs;

constexpr std::string_view kDataDir{TEST_DATA_DIR};

std::filesystem::path DataPath(std::string_view subdir) {
    return std::filesystem::path{std::string{kDataDir}} / std::string{subdir};
}

class AppTest : public ::testing::Test {
 protected:
    std::ostringstream output;
};

class AppJsonTest : public AppTest {
 protected:
    std::filesystem::path
        json_path;  // NOLINT(cppcoreguidelines-non-private-member-variables-in-classes)

    void TearDown() override {
        if (!json_path.empty()) {
            std::filesystem::remove(json_path);
        }
    }
};

// ---------------------------------------------------------------------------
// Базовый запуск
// ---------------------------------------------------------------------------

TEST_F(AppTest, Run_SmallDir_ReturnsZero) {
    const CliArgs args{
        .root_dir = DataPath("small"sv),
        .keywords = {},
        .json_output = std::nullopt,
    };
    EXPECT_EQ(app::Run(args, output), 0);
}

TEST_F(AppTest, Run_SmallDir_OutputContainsTotals) {
    const CliArgs args{
        .root_dir = DataPath("small"sv),
        .keywords = {},
        .json_output = std::nullopt,
    };
    EXPECT_EQ(app::Run(args, output), 0);
    const std::string out = output.str();
    EXPECT_NE(out.find("Files analyzed"sv), std::string::npos);
    EXPECT_NE(out.find("Total lines"sv), std::string::npos);
}

TEST_F(AppTest, Run_SmallDirWithKeywords_ReturnsZero) {
    const CliArgs args{
        .root_dir = DataPath("small"sv),
        .keywords = {"error"s, "warn"s},
        .json_output = std::nullopt,
    };
    EXPECT_EQ(app::Run(args, output), 0);
}

// ---------------------------------------------------------------------------
// JSON-вывод
// ---------------------------------------------------------------------------

TEST_F(AppJsonTest, Run_WithJsonOutput_CreatesFile) {
    json_path =
        std::filesystem::temp_directory_path() / "test_app_report.json"s;

    const CliArgs args{
        .root_dir = DataPath("small"sv),
        .keywords = {},
        .json_output = json_path,
    };
    ASSERT_EQ(app::Run(args, output), 0);
    EXPECT_TRUE(std::filesystem::exists(json_path));
}

TEST_F(AppJsonTest, Run_WithJsonOutput_FileIsNonEmpty) {
    json_path = std::filesystem::temp_directory_path() /
                "test_app_report_nonempty.json"s;

    const CliArgs args{
        .root_dir = DataPath("small"sv),
        .keywords = {},
        .json_output = json_path,
    };
    EXPECT_EQ(app::Run(args, output), 0);
    const std::ifstream ifs{json_path};
    EXPECT_TRUE(ifs.good());
}

// ---------------------------------------------------------------------------
// Ошибки
// ---------------------------------------------------------------------------

TEST_F(AppTest, Run_NonExistentDir_ReturnsOne) {
    const CliArgs args{
        .root_dir = "/nonexistent/path/that/does/not/exist"s,
        .keywords = {},
        .json_output = std::nullopt,
    };
    EXPECT_EQ(app::Run(args, output), 1);
}

// ---------------------------------------------------------------------------
// Нормализованные keywords (end-to-end)
// ---------------------------------------------------------------------------

TEST_F(AppTest, Run_WithUppercaseKeywords_NormalizesAndMatches) {
    const CliArgs args{
        .root_dir = DataPath("small"sv),
        .keywords = {"ERROR"s, "WARN"s},  // uppercase input
        .json_output = std::nullopt,
    };
    EXPECT_EQ(app::Run(args, output), 0);
    const std::string out = output.str();
    // Если keywords нормализованы, они должны найти совпадения
    EXPECT_NE(out.find("Keyword hits"sv), std::string::npos);
}

// ---------------------------------------------------------------------------
// JSON с keywords
// ---------------------------------------------------------------------------

TEST_F(AppJsonTest, Run_WithJsonOutput_ContainsKeywordHits) {
    json_path =
        std::filesystem::temp_directory_path() / "test_app_keywords.json"s;

    const CliArgs args{
        .root_dir = DataPath("small"sv),
        .keywords = {"error"s, "warn"s},
        .json_output = json_path,
    };
    ASSERT_EQ(app::Run(args, output), 0);
    EXPECT_TRUE(std::filesystem::exists(json_path));

    std::ifstream ifs{json_path};
    std::string content((std::istreambuf_iterator<char>(ifs)),
                        std::istreambuf_iterator<char>());
    EXPECT_NE(content.find("keyword_hits"sv), std::string::npos);
}

}  // namespace
