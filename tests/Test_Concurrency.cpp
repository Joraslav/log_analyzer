#include "app/App.hpp"
#include "app/CliArgs.hpp"
#include "concurrency/ThreadPool.hpp"
#include "concurrency/ThreadSafeAggregator.hpp"
#include "domain/FileStats.hpp"
#include "domain/TotalStats.hpp"
#include "domain/Types.hpp"

#include <gtest/gtest.h>

#include <atomic>
#include <cstddef>
#include <filesystem>
#include <future>
#include <latch>
#include <optional>
#include <sstream>
#include <string>  // NOLINT(misc-include-cleaner) - used for "..."s literals
#include <string_view>  // NOLINT(misc-include-cleaner) - used for "..."sv literals
#include <utility>
#include <vector>

namespace {

using namespace std::string_literals;
using namespace std::string_view_literals;

using app::CliArgs;
using concurrency::ThreadPool;
using concurrency::ThreadSafeAggregator;
using domain::FileStats;
using domain::KeywordHits;
using domain::TotalStats;
using domain::WordFrequency;

constexpr std::string_view kDataDir{TEST_DATA_DIR};

struct FileStatsInput {
    std::string_view file_name;
    size_t line_count;
    size_t word_count;
    size_t char_count;
    WordFrequency word_frequency;
    KeywordHits keyword_hits;
};

[[nodiscard]] std::filesystem::path DataPath(std::string_view subdir) {
    return std::filesystem::path{std::string{kDataDir}} / std::string{subdir};
}

[[nodiscard]] FileStats MakeFileStats(FileStatsInput input) {
    FileStats stats;
    stats.file_path = std::filesystem::path{std::string{input.file_name}};
    stats.line_count = input.line_count;
    stats.word_count = input.word_count;
    stats.char_count = input.char_count;
    stats.word_frequency = std::move(input.word_frequency);
    stats.keyword_hits = std::move(input.keyword_hits);
    return stats;
}

void ExpectBasicStats(const TotalStats& total, size_t input_count) {
    EXPECT_EQ(total.file_count, input_count);
    EXPECT_EQ(total.line_count, 10U);
    EXPECT_EQ(total.word_count, 17U);
    EXPECT_EQ(total.char_count, 74U);
    EXPECT_EQ(total.per_file.size(), input_count);
}

void ExpectWordFrequencies(const TotalStats& total) {
    EXPECT_EQ(total.word_frequency.at("hello"s), 4U);
    EXPECT_EQ(total.word_frequency.at("world"s), 1U);
    EXPECT_EQ(total.word_frequency.at("foo"s), 3U);
    EXPECT_EQ(total.word_frequency.at("bar"s), 5U);
}

void ExpectKeywordHits(const TotalStats& total) {
    EXPECT_EQ(total.keyword_hits.at("error"s), 6U);
    EXPECT_EQ(total.keyword_hits.at("warn"s), 3U);
}

void ExpectMergedTotals(const TotalStats& total, const size_t input_count) {
    ExpectBasicStats(total, input_count);
    ExpectWordFrequencies(total);
    ExpectKeywordHits(total);
}

class ThreadPoolTest : public ::testing::TestWithParam<size_t> {};

class ThreadSafeAggregatorTest : public ::testing::Test {};

class AppConcurrencyTest : public ::testing::TestWithParam<size_t> {
 protected:
    std::ostringstream output;
};

TEST_P(ThreadPoolTest, Enqueue_ReturnsResultsInSubmissionOrder) {
    ThreadPool pool(GetParam());
    std::vector<std::future<size_t>> futures;
    futures.reserve(8);

    for (size_t index = 0; index < 8; ++index) {
        futures.push_back(pool.Enqueue([index] { return index * index; }));
    }

    for (size_t index = 0; index < futures.size(); ++index) {
        EXPECT_EQ(futures.at(index).get(), index * index);
    }
}

TEST_P(ThreadPoolTest, Enqueue_ExecutesAllTasks_WhenGateIsReleased) {
    ThreadPool pool(GetParam());
    std::latch start_gate{1};
    std::atomic<size_t> completed{0};
    std::vector<std::future<void>> futures;
    futures.reserve(4);

    for (size_t index = 0; index < 4; ++index) {
        futures.push_back(pool.Enqueue([&completed, &start_gate] {
            start_gate.wait();
            completed.fetch_add(1, std::memory_order_relaxed);
        }));
    }

    start_gate.count_down();

    for (auto& future : futures) {
        future.get();
    }

    EXPECT_EQ(completed.load(std::memory_order_relaxed), 4U);
}

INSTANTIATE_TEST_SUITE_P(PoolSizes, ThreadPoolTest,
                         ::testing::Values(1U, 2U, 4U, 8U));

TEST_F(ThreadSafeAggregatorTest,
       AddFileStats_FromMultipleThreads_MergesTotals) {
    ThreadSafeAggregator aggregator;
    ThreadPool pool(4);
    std::latch start_gate{1};

    const std::vector<FileStats> inputs = {
        MakeFileStats(FileStatsInput{
            .file_name = "one.log"sv,
            .line_count = 2U,
            .word_count = 5U,
            .char_count = 20U,
            .word_frequency = {{"hello"s, 2U}, {"world"s, 1U}},
            .keyword_hits = {{"error"s, 1U}, {"warn"s, 0U}},
        }),
        MakeFileStats(FileStatsInput{
            .file_name = "two.log"sv,
            .line_count = 3U,
            .word_count = 4U,
            .char_count = 18U,
            .word_frequency = {{"hello"s, 1U}, {"foo"s, 3U}},
            .keyword_hits = {{"error"s, 2U}, {"warn"s, 1U}},
        }),
        MakeFileStats(FileStatsInput{
            .file_name = "three.log"sv,
            .line_count = 1U,
            .word_count = 2U,
            .char_count = 9U,
            .word_frequency = {{"bar"s, 4U}},
            .keyword_hits = {{"error"s, 0U}, {"warn"s, 2U}},
        }),
        MakeFileStats(FileStatsInput{
            .file_name = "four.log"sv,
            .line_count = 4U,
            .word_count = 6U,
            .char_count = 27U,
            .word_frequency = {{"hello"s, 1U}, {"bar"s, 1U}},
            .keyword_hits = {{"error"s, 3U}, {"warn"s, 0U}},
        }),
    };

    std::vector<std::future<void>> futures;
    futures.reserve(inputs.size());

    for (const auto& stats : inputs) {
        futures.push_back(pool.Enqueue([&aggregator, &start_gate, stats] {
            start_gate.wait();
            aggregator.AddFileStats(stats);
        }));
    }

    start_gate.count_down();

    for (auto& future : futures) {
        future.get();
    }

    const auto total = aggregator.GetTotalStats();
    ExpectMergedTotals(total, inputs.size());
}

TEST_P(AppConcurrencyTest, Run_SmallDirWithThreadCount_ReturnsZero) {
    const CliArgs args{
        .root_dir = DataPath("small"sv),
        .keywords = {},
        .json_output = std::nullopt,
        .thread_count = GetParam(),
    };

    EXPECT_EQ(app::Run(args, output), 0);
    const std::string out = output.str();
    EXPECT_NE(out.find("Files analyzed"sv), std::string::npos);
    EXPECT_NE(out.find("Total lines"sv), std::string::npos);
}

INSTANTIATE_TEST_SUITE_P(ThreadCounts, AppConcurrencyTest,
                         ::testing::Values(1U, 2U, 4U, 8U));

}  // namespace
