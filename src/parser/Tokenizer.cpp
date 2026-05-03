#include "Tokenizer.hpp"

#include <cctype>
#include <cstddef>
#include <string>
#include <string_view>
#include <vector>

namespace parser {

namespace {

[[nodiscard]] bool IsWhitespace(const unsigned char ch) noexcept {
    return std::isspace(ch) != 0;
}

[[nodiscard]] bool IsPunctuation(const unsigned char ch) noexcept {
    return std::ispunct(ch) != 0;
}

[[nodiscard]] std::string NormalizeToken(std::string_view token) {
    size_t left = 0;
    size_t right = token.size();

    while (left < right &&
           IsPunctuation(static_cast<unsigned char>(token.at(left)))) {
        ++left;
    }
    while (left < right &&
           IsPunctuation(static_cast<unsigned char>(token.at(right - 1)))) {
        --right;
    }

    std::string normalized;
    normalized.reserve(right - left);

    for (size_t index = left; index < right; ++index) {
        const auto ch = static_cast<unsigned char>(token.at(index));
        normalized.push_back(static_cast<char>(std::tolower(ch)));
    }

    return normalized;
}

}  // namespace

std::vector<std::string> TokenizeWhitespaceTrimLower(std::string_view text) {
    std::vector<std::string> tokens;
    size_t index = 0;

    while (index < text.size()) {
        while (index < text.size() &&
               IsWhitespace(static_cast<unsigned char>(text.at(index)))) {
            ++index;
        }

        const size_t begin = index;

        while (index < text.size() &&
               !IsWhitespace(static_cast<unsigned char>(text.at(index)))) {
            ++index;
        }

        if (begin == index) {
            continue;
        }

        const std::string normalized =
            NormalizeToken(text.substr(begin, index - begin));
        if (!normalized.empty()) {
            tokens.push_back(normalized);
        }
    }

    return tokens;
}

}  // namespace parser
