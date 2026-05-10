#include "ThreadSafeAggregator.hpp"

#include "domain/FileStats.hpp"
#include "domain/TotalStats.hpp"

#include <mutex>

namespace concurrency {

void ThreadSafeAggregator::AddFileStats(const domain::FileStats& file_stats) {
    const std::scoped_lock lock(mutex_);

    total_stats_.file_count++;
    total_stats_.line_count += file_stats.line_count;
    total_stats_.word_count += file_stats.word_count;
    total_stats_.char_count += file_stats.char_count;

    for (const auto& [word, count] : file_stats.word_frequency) {
        total_stats_.word_frequency[word] += count;
    }

    for (const auto& [keyword, count] : file_stats.keyword_hits) {
        total_stats_.keyword_hits[keyword] += count;
    }

    total_stats_.per_file.push_back(file_stats);
}

domain::TotalStats ThreadSafeAggregator::GetTotalStats() const {
    const std::scoped_lock lock(mutex_);
    return total_stats_;
}

void ThreadSafeAggregator::Reset() {
    const std::scoped_lock lock(mutex_);
    total_stats_ = domain::TotalStats{};
}

}  // namespace concurrency
