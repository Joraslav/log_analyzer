#include "app/Cli.hpp"
#include "app/CliArgs.hpp"
#include "domain/Types.hpp"

#include <gtest/gtest.h>

#include <array>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <tuple>
#include <vector>

namespace {

using namespace std::string_view_literals;
using app::ParseArgs;

class CliTest : public ::testing::Test {};

struct InvalidCliArgsCase {
    std::string_view case_name;
    std::vector<std::string_view> argv;
};

class CliInvalidArgsParameterizedTest
    : public CliTest,
      public ::testing::WithParamInterface<InvalidCliArgsCase> {};

// ---------------------------------------------------------------------------
// Базовые случаи
// ---------------------------------------------------------------------------

TEST_F(CliTest, ParseArgs_OnlyRootDir_ReturnsCorrectPath) {
    const std::array<std::string_view, 1> argv = {"/tmp/logs"sv};
    const auto args = ParseArgs(std::span<const std::string_view>{argv});
    EXPECT_EQ(args.root_dir, "/tmp/logs");
}

TEST_F(CliTest, ParseArgs_OnlyRootDir_EmptyKeywords) {
    const std::array<std::string_view, 1> argv = {"/tmp/logs"sv};
    const auto args = ParseArgs(std::span<const std::string_view>{argv});
    EXPECT_TRUE(args.keywords.empty());
}

TEST_F(CliTest, ParseArgs_OnlyRootDir_NoJsonOutput) {
    const std::array<std::string_view, 1> argv = {"/tmp/logs"sv};
    const auto args = ParseArgs(std::span<const std::string_view>{argv});
    EXPECT_FALSE(args.json_output.has_value());
}

// ---------------------------------------------------------------------------
// --keywords
// ---------------------------------------------------------------------------

TEST_F(CliTest, ParseArgs_WithKeywords_ParsedCorrectly) {
    const std::array<std::string_view, 3> argv = {"/tmp/logs"sv, "--keywords"sv,
                                                  "error,warn,info"sv};
    const auto args = ParseArgs(std::span<const std::string_view>{argv});
    ASSERT_EQ(args.keywords.size(), 3U);
    EXPECT_EQ(args.keywords[0], "error");
    EXPECT_EQ(args.keywords[1], "warn");
    EXPECT_EQ(args.keywords[2], "info");
}

TEST_F(CliTest, ParseArgs_WithSingleKeyword_ParsedCorrectly) {
    const std::array<std::string_view, 3> argv = {"/tmp/logs"sv, "--keywords"sv,
                                                  "error"sv};
    const auto args = ParseArgs(std::span<const std::string_view>{argv});
    ASSERT_EQ(args.keywords.size(), 1U);
    EXPECT_EQ(args.keywords[0], "error");
}

TEST_F(CliTest, ParseArgs_WithUppercaseKeywords_NormalizesToLowercase) {
    const std::array<std::string_view, 3> argv = {"/tmp/logs"sv, "--keywords"sv,
                                                  "ERROR, Warn ,InFo"sv};
    const auto args = ParseArgs(std::span<const std::string_view>{argv});
    ASSERT_EQ(args.keywords.size(), 3U);
    EXPECT_EQ(args.keywords[0], "error");
    EXPECT_EQ(args.keywords[1], "warn");
    EXPECT_EQ(args.keywords[2], "info");
}

// ---------------------------------------------------------------------------
// --json
// ---------------------------------------------------------------------------

TEST_F(CliTest, ParseArgs_WithJsonOutput_ParsedCorrectly) {
    const std::array<std::string_view, 3> argv = {"/tmp/logs"sv, "--json"sv,
                                                  "/tmp/out.json"sv};
    const auto args = ParseArgs(std::span<const std::string_view>{argv});
    ASSERT_TRUE(args.json_output.has_value());
    EXPECT_EQ(*args.json_output, "/tmp/out.json");
}

// ---------------------------------------------------------------------------
// Комбинация флагов
// ---------------------------------------------------------------------------

TEST_F(CliTest, ParseArgs_AllFlags_AllParsedCorrectly) {
    const std::array<std::string_view, 5> argv = {"/tmp/logs"sv, "--keywords"sv,
                                                  "error,warn"sv, "--json"sv,
                                                  "/tmp/report.json"sv};
    const auto args = ParseArgs(std::span<const std::string_view>{argv});
    EXPECT_EQ(args.root_dir, "/tmp/logs");
    ASSERT_EQ(args.keywords.size(), 2U);
    EXPECT_EQ(args.keywords[0], "error");
    EXPECT_EQ(args.keywords[1], "warn");
    ASSERT_TRUE(args.json_output.has_value());
    EXPECT_EQ(*args.json_output, "/tmp/report.json");
}

// ---------------------------------------------------------------------------
// Ошибки
// ---------------------------------------------------------------------------

TEST_F(CliTest, ParseArgs_NoArgs_ThrowsInvalidArgument) {
    const std::span<const std::string_view> empty{};
    EXPECT_THROW(std::ignore = ParseArgs(empty), std::invalid_argument);
}

TEST_P(CliInvalidArgsParameterizedTest,
       ParseArgs_InvalidArguments_ThrowsInvalidArgument) {
    const auto& [case_name, argv] = GetParam();
    (void)case_name;
    EXPECT_THROW(
        std::ignore = ParseArgs(std::span<const std::string_view>{argv}),
        std::invalid_argument);
}

INSTANTIATE_TEST_SUITE_P(
    InvalidArgs, CliInvalidArgsParameterizedTest,
    ::testing::Values(
        InvalidCliArgsCase{"UnknownFlag"sv, {"/tmp/logs"sv, "--unknown"sv}},
        InvalidCliArgsCase{"KeywordsWithoutValue"sv,
                           {"/tmp/logs"sv, "--keywords"sv}},
        InvalidCliArgsCase{"JsonWithoutValue"sv, {"/tmp/logs"sv, "--json"sv}}),
    [](const auto& info) { return std::string{info.param.case_name}; });

// ---------------------------------------------------------------------------
// Граничные случаи
// ---------------------------------------------------------------------------

TEST_F(CliTest, ParseArgs_WithEmptyKeywordValue_IgnoresEmpty) {
    const std::array<std::string_view, 3> argv = {"/tmp/logs"sv, "--keywords"sv,
                                                  ",,"sv};
    const auto args = ParseArgs(std::span<const std::string_view>{argv});
    EXPECT_TRUE(args.keywords.empty());
}

TEST_F(CliTest, ParseArgs_WithWhitespaceOnlyKeyword_TrimsAndIgnores) {
    const std::array<std::string_view, 3> argv = {"/tmp/logs"sv, "--keywords"sv,
                                                  "  ,  ,  "sv};
    const auto args = ParseArgs(std::span<const std::string_view>{argv});
    EXPECT_TRUE(args.keywords.empty());
}

TEST_F(CliTest, ParseArgs_KeywordsWithLeadingTrailingSpace_Trimmed) {
    const std::array<std::string_view, 3> argv = {"/tmp/logs"sv, "--keywords"sv,
                                                  "  error  ,  warn  "sv};
    const auto args = ParseArgs(std::span<const std::string_view>{argv});
    ASSERT_EQ(args.keywords.size(), 2U);
    EXPECT_EQ(args.keywords[0], "error");
    EXPECT_EQ(args.keywords[1], "warn");
}

TEST_F(CliTest, ParseArgs_DuplicateKeywords_LastValueWins) {
    const std::array<std::string_view, 5> argv = {"/tmp/logs"sv, "--keywords"sv,
                                                  "error,warn"sv,
                                                  "--keywords"sv, "critical"sv};
    const auto args = ParseArgs(std::span<const std::string_view>{argv});
    ASSERT_EQ(args.keywords.size(), 1U);
    EXPECT_EQ(args.keywords[0], "critical");
}

TEST_F(CliTest, ParseArgs_DuplicateJson_LastValueWins) {
    const std::array<std::string_view, 5> argv = {
        "/tmp/logs"sv, "--json"sv, "/tmp/first.json"sv, "--json"sv,
        "/tmp/second.json"sv};
    const auto args = ParseArgs(std::span<const std::string_view>{argv});
    ASSERT_TRUE(args.json_output.has_value());
    EXPECT_EQ(*args.json_output, "/tmp/second.json");
}

TEST_F(CliTest, ParseArgs_FlagsInReverseOrder_ParsedCorrectly) {
    const std::array<std::string_view, 5> argv = {
        "/tmp/logs"sv, "--json"sv, "/tmp/report.json"sv, "--keywords"sv,
        "error,warn"sv};
    const auto args = ParseArgs(std::span<const std::string_view>{argv});
    EXPECT_EQ(args.root_dir, "/tmp/logs");
    ASSERT_TRUE(args.json_output.has_value());
    EXPECT_EQ(*args.json_output, "/tmp/report.json");
    ASSERT_EQ(args.keywords.size(), 2U);
    EXPECT_EQ(args.keywords[0], "error");
    EXPECT_EQ(args.keywords[1], "warn");
}

}  // namespace
