#include "analysis/FileAnalyzer.hpp"
#include "domain/FileStats.hpp"
#include "domain/Types.hpp"

#include <gtest/gtest.h>

#include <filesystem>
#include <stdexcept>
#include <string>
#include <string_view>
#include <tuple>

namespace {

using namespace analysis;
using namespace std::string_view_literals;
using namespace std::string_literals;

constexpr std::string_view kDataDir{TEST_DATA_DIR};

std::filesystem::path DataPath(std::string_view filename) {
    return std::filesystem::path{std::string{kDataDir}} / "analysis"s /
           filename;
}

class FileAnalyzerTest : public ::testing::Test {
 protected:
    static constexpr std::string_view kSimpleLog{"simple.log"sv};
    static constexpr std::string_view kKeywordsLog{"keywords.log"sv};
};

// simple.log: "hello world\nhello foo\n"
// line 1: "hello world"  → tokens: hello, world
// line 2: "hello foo"    → tokens: hello, foo
// char_count: 11 + 9 = 20 (без символов новой строки)

TEST_F(FileAnalyzerTest, AnalyzeFile_SimpleFile_CorrectLineCount) {
    const auto stats = AnalyzeFile(DataPath(kSimpleLog), {});
    EXPECT_EQ(stats.line_count, 2U);
}

TEST_F(FileAnalyzerTest, AnalyzeFile_SimpleFile_CorrectWordCount) {
    const auto stats = AnalyzeFile(DataPath(kSimpleLog), {});
    EXPECT_EQ(stats.word_count, 4U);
}

TEST_F(FileAnalyzerTest, AnalyzeFile_SimpleFile_CorrectCharCount) {
    const auto stats = AnalyzeFile(DataPath(kSimpleLog), {});
    EXPECT_EQ(stats.char_count, 20U);
}

TEST_F(FileAnalyzerTest, AnalyzeFile_SimpleFile_CorrectWordFrequency) {
    const auto stats = AnalyzeFile(DataPath(kSimpleLog), {});
    EXPECT_EQ(stats.word_frequency.at("hello"s), 2U);
    EXPECT_EQ(stats.word_frequency.at("world"s), 1U);
    EXPECT_EQ(stats.word_frequency.at("foo"s), 1U);
}

TEST_F(FileAnalyzerTest, AnalyzeFile_SimpleFile_FilePathStored) {
    const auto path = DataPath(kSimpleLog);
    const auto stats = AnalyzeFile(path, {});
    EXPECT_EQ(stats.file_path, path);
}

// keywords.log:
// "error connecting to server"  → hits: error
// "warning disk usage high"     → hits: warning
// "error timeout exceeded"      → hits: error

TEST_F(FileAnalyzerTest, AnalyzeFile_KeywordsMatch_CountsHits) {
    const domain::Keywords keywords{"error"s, "warning"s};
    const auto stats = AnalyzeFile(DataPath(kKeywordsLog), keywords);
    EXPECT_EQ(stats.keyword_hits.at("error"s), 2U);
    EXPECT_EQ(stats.keyword_hits.at("warning"s), 1U);
}

TEST_F(FileAnalyzerTest, AnalyzeFile_NoMatchingKeywords_ContainsZeroHits) {
    const domain::Keywords keywords{"critical"s, "fatal"s};
    const auto stats = AnalyzeFile(DataPath(kKeywordsLog), keywords);
    EXPECT_EQ(stats.keyword_hits.at("critical"s), 0U);
    EXPECT_EQ(stats.keyword_hits.at("fatal"s), 0U);
}

TEST_F(FileAnalyzerTest, AnalyzeFile_EmptyKeywords_EmptyHits) {
    const auto stats = AnalyzeFile(DataPath(kSimpleLog), {});
    EXPECT_TRUE(stats.keyword_hits.empty());
}

TEST_F(FileAnalyzerTest, AnalyzeFile_EmptyPath_ThrowsInvalidArgument) {
    EXPECT_THROW(std::ignore = AnalyzeFile({}, {}), std::invalid_argument);
}

TEST_F(FileAnalyzerTest, AnalyzeFile_NonExistentFile_ThrowsRuntimeError) {
    EXPECT_THROW(std::ignore = AnalyzeFile("/nonexistent/path/file.log", {}),
                 std::runtime_error);
}

TEST_F(FileAnalyzerTest, AnalyzeFile_AllKeywordsInitializedWithZeros) {
    const domain::Keywords keywords{"notfound1"s, "notfound2"s, "notfound3"s};
    const auto stats = AnalyzeFile(DataPath(kSimpleLog), keywords);
    ASSERT_EQ(stats.keyword_hits.size(), 3U);
    EXPECT_EQ(stats.keyword_hits.at("notfound1"s), 0U);
    EXPECT_EQ(stats.keyword_hits.at("notfound2"s), 0U);
    EXPECT_EQ(stats.keyword_hits.at("notfound3"s), 0U);
}

TEST_F(FileAnalyzerTest, AnalyzeFile_MixedMatchAndNoMatch_KeywordsCorrect) {
    const domain::Keywords keywords{"hello"s, "notfound"s};
    const auto stats = AnalyzeFile(DataPath(kSimpleLog), keywords);
    EXPECT_EQ(stats.keyword_hits.at("hello"s), 2U);     // found in simple.log
    EXPECT_EQ(stats.keyword_hits.at("notfound"s), 0U);  // not found
}

}  // namespace
