#include "Aggregator.hpp"

#include "domain/FileStats.hpp"
#include "domain/TotalStats.hpp"

#include <vector>

namespace analysis {

using domain::FileStats;
using domain::TotalStats;

TotalStats Aggregate(const std::vector<FileStats>& file_stats) {
    TotalStats total;
    total.file_count = file_stats.size();
    total.per_file.reserve(file_stats.size());

    for (const auto& fs : file_stats) {
        total.line_count += fs.line_count;
        total.word_count += fs.word_count;
        total.char_count += fs.char_count;

        for (const auto& [word, count] : fs.word_frequency) {
            total.word_frequency[word] += count;
        }
        for (const auto& [keyword, count] : fs.keyword_hits) {
            total.keyword_hits[keyword] += count;
        }

        total.per_file.push_back(fs);
    }

    return total;
}

}  // namespace analysis
