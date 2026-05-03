#include "FileAnalyzer.hpp"

#include "domain/FileStats.hpp"
#include "domain/Types.hpp"
#include "parser/Tokenizer.hpp"

#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <string>
#include <unordered_set>

namespace analysis {

using domain::FileStats;
using domain::Keywords;
using parser::TokenizeWhitespaceTrimLower;

FileStats AnalyzeFile(const std::filesystem::path& file_path,
                      const Keywords& keywords) {
    if (file_path.empty()) {
        throw std::invalid_argument("File path must not be empty");
    }

    std::ifstream file{file_path};
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file: " + file_path.string());
    }

    FileStats stats;
    stats.file_path = file_path;

    std::unordered_set<std::string> keyword_lookup;
    keyword_lookup.reserve(keywords.size());
    for (const auto& keyword : keywords) {
        keyword_lookup.insert(keyword);
        stats.keyword_hits[keyword] = 0;
    }

    std::string line;
    while (std::getline(file, line)) {
        ++stats.line_count;
        stats.char_count += line.size();

        const auto tokens = TokenizeWhitespaceTrimLower(line);
        stats.word_count += tokens.size();

        for (const auto& token : tokens) {
            ++stats.word_frequency[token];
            if (keyword_lookup.contains(token)) {
                ++stats.keyword_hits[token];
            }
        }
    }

    if (file.bad()) {
        throw std::runtime_error("Error reading file: " + file_path.string());
    }

    return stats;
}

}  // namespace analysis
