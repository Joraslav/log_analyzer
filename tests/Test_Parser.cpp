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

TEST_F(ParserTest, TokenizeWhitespaceTrimLower_SplitsByWhitespace) {
    const auto tokens =
        TokenizeWhitespaceTrimLower("alpha beta\tgamma\ndelta"sv);
    const std::vector<std::string> expected{"alpha"s, "beta"s, "gamma"s,
                                            "delta"s};

    EXPECT_EQ(tokens, expected);
}

TEST_F(ParserTest, TokenizeWhitespaceTrimLower_TrimPunctuationOnEdges) {
    const auto tokens =
        TokenizeWhitespaceTrimLower("[Error], (Warning)! {Info}: ;debug;"sv);
    const std::vector<std::string> expected{"error"s, "warning"s, "info"s,
                                            "debug"s};

    EXPECT_EQ(tokens, expected);
}

TEST_F(ParserTest, TokenizeWhitespaceTrimLower_LowercasesTokens) {
    const auto tokens = TokenizeWhitespaceTrimLower("HTTP Timeout FoObAr"sv);
    const std::vector<std::string> expected{"http"s, "timeout"s, "foobar"s};

    EXPECT_EQ(tokens, expected);
}

TEST_F(ParserTest, TokenizeWhitespaceTrimLower_DropsPunctuationOnlyTokens) {
    const auto tokens = TokenizeWhitespaceTrimLower("... !!! ???"sv);

    EXPECT_TRUE(tokens.empty());
}

TEST_F(ParserTest, TokenizeWhitespaceTrimLower_KeepsInnerPunctuation) {
    const auto tokens =
        TokenizeWhitespaceTrimLower("error-code can't v1.2.3"sv);
    const std::vector<std::string> expected{"error-code"s, "can't"s, "v1.2.3"s};

    EXPECT_EQ(tokens, expected);
}

}  // namespace
