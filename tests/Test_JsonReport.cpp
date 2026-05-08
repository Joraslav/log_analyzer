#include "domain/TotalStats.hpp"
#include "report/JsonReport.hpp"

#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>

namespace {

using namespace domain;
using namespace report;
using namespace std::string_literals;
using namespace std::string_view_literals;

std::filesystem::path TempJsonPath() {
    return std::filesystem::temp_directory_path() / "test_report.json"s;
}

TotalStats MakeTotalStats() {
    TotalStats stats;
    stats.file_count = 1;
    stats.line_count = 3;
    stats.word_count = 6;
    stats.char_count = 40;
    stats.word_frequency = {{"hello"s, 2}, {"world"s, 1}};
    stats.keyword_hits = {{"error"s, 1}};
    return stats;
}

class JsonReportTest : public ::testing::Test {
 protected:
    void TearDown() override { std::filesystem::remove(TempJsonPath()); }

    static std::string ReadFile(const std::filesystem::path& path) {
        std::ifstream file{path};
        EXPECT_TRUE(file.is_open());
        std::ostringstream content;
        content << file.rdbuf();
        return content.str();
    }
};

class JsonReportFieldParameterizedTest
    : public JsonReportTest,
      public ::testing::WithParamInterface<std::string_view> {};

TEST_F(JsonReportTest, WriteJsonReport_CreatesFile) {
    WriteJsonReport(MakeTotalStats(), TempJsonPath());
    EXPECT_TRUE(std::filesystem::exists(TempJsonPath()));
}

TEST_P(JsonReportFieldParameterizedTest,
       WriteJsonReport_ContainsExpectedFields) {
    WriteJsonReport(MakeTotalStats(), TempJsonPath());
    const std::string content = ReadFile(TempJsonPath());
    EXPECT_NE(content.find(std::string{GetParam()}), std::string::npos);
}

INSTANTIATE_TEST_SUITE_P(JsonFields, JsonReportFieldParameterizedTest,
                         ::testing::Values("file_count"sv, "word_frequency"sv,
                                           "keyword_hits"sv, "per_file"sv));

TEST_F(JsonReportTest, WriteJsonReport_EmptyStats_CreatesValidFile) {
    const TotalStats empty_stats;
    WriteJsonReport(empty_stats, TempJsonPath());
    EXPECT_TRUE(std::filesystem::exists(TempJsonPath()));
}

TEST_F(JsonReportTest, WriteJsonReport_WithZeroKeywordHits_IncludesZeros) {
    TotalStats stats;
    stats.keyword_hits = {{"error"s, 2}, {"warning"s, 0}, {"critical"s, 0}};
    WriteJsonReport(stats, TempJsonPath());
    const std::string content = ReadFile(TempJsonPath());
    EXPECT_NE(content.find("error"s), std::string::npos);
    EXPECT_NE(content.find("warning"s), std::string::npos);
    EXPECT_NE(content.find("critical"s), std::string::npos);
}

TEST_F(JsonReportTest, WriteJsonReport_InvalidPath_ThrowsRuntimeError) {
    const auto invalid_path = "/nonexistent/invalid/path/report.json"s;
    EXPECT_THROW(WriteJsonReport(MakeTotalStats(), invalid_path),
                 std::runtime_error);
}

}  // namespace
