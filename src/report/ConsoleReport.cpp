#include "ConsoleReport.hpp"

#include "domain/FileStats.hpp"
#include "domain/TotalStats.hpp"
#include "domain/Types.hpp"

#include <algorithm>
#include <cstddef>
#include <ostream>
#include <string>
#include <utility>
#include <vector>

namespace report {

using domain::TotalStats;

namespace {

using domain::FileStats;
using domain::KeywordHits;
using domain::WordFrequency;

constexpr size_t kTopWordsCount = 10;

void PrintSeparator(std::ostream& output) {
    output << "----------------------------------------\n";
}

void PrintTotalStats(const TotalStats& stats, std::ostream& output) {
    output << "=== Log Analyzer Report ===\n";
    PrintSeparator(output);
    output << "Files analyzed : " << stats.file_count << "\n";
    output << "Total lines    : " << stats.line_count << "\n";
    output << "Total words    : " << stats.word_count << "\n";
    output << "Total chars    : " << stats.char_count << "\n";
}

void PrintTopWords(const WordFrequency& word_frequency, std::ostream& output) {
    using Pair = std::pair<std::string, size_t>;

    std::vector<Pair> sorted{word_frequency.begin(), word_frequency.end()};
    std::ranges::sort(sorted, [](const Pair& left, const Pair& right) {
        return left.second > right.second;
    });

    output << "\n--- Top " << kTopWordsCount << " words ---\n";
    const size_t limit = std::min(sorted.size(), kTopWordsCount);
    for (size_t index = 0; index < limit; ++index) {
        const Pair& word_stat = sorted.at(index);
        output << "  " << word_stat.first << ": " << word_stat.second << "\n";
    }
}

void PrintKeywordHits(const KeywordHits& keyword_hits, std::ostream& output) {
    output << "\n--- Keyword hits ---\n";
    if (keyword_hits.empty()) {
        output << "  (no keywords specified)\n";
        return;
    }
    for (const auto& [keyword, count] : keyword_hits) {
        output << "  " << keyword << ": " << count << "\n";
    }
}

void PrintPerFile(const std::vector<FileStats>& per_file,
                  std::ostream& output) {
    output << "\n--- Per-file stats ---\n";
    for (const auto& file_stats : per_file) {
        output << "  " << file_stats.file_path.string() << "\n";
        output << "    lines=" << file_stats.line_count
               << "  words=" << file_stats.word_count
               << "  chars=" << file_stats.char_count << "\n";
    }
}

}  // namespace

void PrintReport(const TotalStats& stats, std::ostream& output) {
    PrintTotalStats(stats, output);
    PrintTopWords(stats.word_frequency, output);
    PrintKeywordHits(stats.keyword_hits, output);
    PrintPerFile(stats.per_file, output);
    PrintSeparator(output);
}

}  // namespace report
