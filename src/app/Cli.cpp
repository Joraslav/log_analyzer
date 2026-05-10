#include "Cli.hpp"

#include "CliArgs.hpp"
#include "domain/Types.hpp"

#include <algorithm>
#include <cctype>
#include <cstddef>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>

namespace app {

namespace {

using domain::Keywords;

[[nodiscard]] std::string NormalizeKeyword(std::string_view token) {
    while (!token.empty() &&
           std::isspace(static_cast<unsigned char>(token.front()))) {
        token.remove_prefix(1);
    }
    while (!token.empty() &&
           std::isspace(static_cast<unsigned char>(token.back()))) {
        token.remove_suffix(1);
    }

    std::string normalized{token};
    std::ranges::transform(normalized, normalized.begin(),
                           [](const unsigned char ch) {
                               return static_cast<char>(std::tolower(ch));
                           });
    return normalized;
}

[[nodiscard]] Keywords ParseKeywords(std::string_view value) {
    Keywords keywords;
    size_t start = 0;
    while (start < value.size()) {
        const size_t end = value.find(',', start);
        const std::string_view token = (end == std::string_view::npos)
                                           ? value.substr(start)
                                           : value.substr(start, end - start);
        const std::string normalized = NormalizeKeyword(token);
        if (!normalized.empty()) {
            keywords.push_back(normalized);
        }
        if (end == std::string_view::npos) {
            break;
        }
        start = end + 1;
    }
    return keywords;
}

}  // namespace

CliArgs ParseArgs(std::span<const std::string_view> args) {
    if (args.empty()) {
        throw std::invalid_argument{
            "Usage: log_analyzer <root_dir> [--keywords w1,w2] [--json <path>] "
            "[--threads N]"};
    }

    CliArgs result;
    result.root_dir = std::string{args.front()};

    const auto tail = args.subspan(1);
    for (auto it = tail.begin(); it != tail.end(); ++it) {
        const std::string_view flag = *it;

        if (flag == "--keywords") {
            ++it;
            if (it == tail.end()) {
                throw std::invalid_argument{"--keywords requires a value"};
            }
            result.keywords = ParseKeywords(*it);
        } else if (flag == "--json") {
            ++it;
            if (it == tail.end()) {
                throw std::invalid_argument{"--json requires a path"};
            }
            result.json_output = std::string{*it};
        } else if (flag == "--threads") {
            ++it;
            if (it == tail.end()) {
                throw std::invalid_argument{"--threads requires a value"};
            }
            try {
                result.thread_count = std::stoul(std::string{*it});
            } catch (const std::invalid_argument&) {
                throw std::invalid_argument{
                    "--threads requires a numeric value"};
            } catch (const std::out_of_range&) {
                throw std::invalid_argument{"--threads value is out of range"};
            }
        } else {
            throw std::invalid_argument{"Unknown argument: " +
                                        std::string{flag}};
        }
    }

    return result;
}

}  // namespace app
