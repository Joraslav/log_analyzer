#include "parser/Tokenizer.hpp"

#include <gtest/gtest.h>

#include <string>
#include <string_view>
#include <vector>

namespace {

using namespace parser;
using namespace std::string_literals;
using namespace std::string_view_literals;

class ParserTest : public ::testing::Test {};

struct TokenizeCase {
    std::string_view case_name;
    std::string_view input;
    std::vector<std::string> expected;
};

class ParserParameterizedTest
    : public ParserTest,
      public ::testing::WithParamInterface<TokenizeCase> {};

TEST_P(ParserParameterizedTest,
       TokenizeWhitespaceTrimLower_InputVariants_ExpectedTokens) {
    const auto& [case_name, input, expected] = GetParam();
    (void)case_name;
    const auto tokens = TokenizeWhitespaceTrimLower(input);
    EXPECT_EQ(tokens, expected);
}

INSTANTIATE_TEST_SUITE_P(
    TokenizerScenarios, ParserParameterizedTest,
    ::testing::Values(TokenizeCase{"SplitByWhitespace"sv,
                                   "alpha beta\tgamma\ndelta"sv,
                                   {"alpha"s, "beta"s, "gamma"s, "delta"s}},
                      TokenizeCase{"TrimPunctuation"sv,
                                   "[Error], (Warning)! {Info}: ;debug;"sv,
                                   {"error"s, "warning"s, "info"s, "debug"s}},
                      TokenizeCase{"LowercaseTokens"sv,
                                   "HTTP Timeout FoObAr"sv,
                                   {"http"s, "timeout"s, "foobar"s}},
                      TokenizeCase{
                          "DropOnlyPunctuation"sv, "... !!! ???"sv, {}},
                      TokenizeCase{"KeepInnerPunctuation"sv,
                                   "error-code can't v1.2.3"sv,
                                   {"error-code"s, "can't"s, "v1.2.3"s}},
                      TokenizeCase{"EmptyInput"sv, ""sv, {}},
                      TokenizeCase{"WhitespaceOnlyInput"sv, "  \t\n  "sv, {}}),
    [](const auto& param_info) {
        return std::string{param_info.param.case_name};
    });

}  // namespace
