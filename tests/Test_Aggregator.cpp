#include "analysis/Aggregator.hpp"
#include "domain/FileStats.hpp"
#include "domain/TotalStats.hpp"
#include "domain/Types.hpp"

#include <gtest/gtest.h>

#include <string>
#include <utility>
#include <vector>

namespace {

using namespace analysis;
using namespace domain;
using namespace std::string_literals;

[[nodiscard]] FileStats MakeFileStats(WordFrequency word_freq,
                                      KeywordHits kw_hits) {
    FileStats stats;
    stats.word_frequency = std::move(word_freq);
    stats.keyword_hits = std::move(kw_hits);
    return stats;
}

class AggregatorTest : public ::testing::Test {};

TEST_F(AggregatorTest, Aggregate_EmptyInput_ZeroCounts) {
    const auto total = Aggregate({});
    EXPECT_EQ(total.file_count, 0U);
    EXPECT_EQ(total.line_count, 0U);
    EXPECT_EQ(total.word_count, 0U);
    EXPECT_EQ(total.char_count, 0U);
    EXPECT_TRUE(total.per_file.empty());
}

TEST_F(AggregatorTest, Aggregate_SingleFile_FileCountOne) {
    FileStats stats;
    stats.line_count = 3;
    stats.word_count = 10;
    stats.char_count = 50;
    const auto total = Aggregate({stats});
    EXPECT_EQ(total.file_count, 1U);
}

TEST_F(AggregatorTest, Aggregate_MultipleFiles_SumsCounts) {
    FileStats s1;
    s1.line_count = 2;
    s1.word_count = 4;
    s1.char_count = 20;
    FileStats s2;
    s2.line_count = 3;
    s2.word_count = 9;
    s2.char_count = 45;
    const auto total = Aggregate({s1, s2});
    EXPECT_EQ(total.file_count, 2U);
    EXPECT_EQ(total.line_count, 5U);
    EXPECT_EQ(total.word_count, 13U);
    EXPECT_EQ(total.char_count, 65U);
}

TEST_F(AggregatorTest, Aggregate_MultipleFiles_MergesWordFrequency) {
    const auto s1 = MakeFileStats({{"hello"s, 2}, {"world"s, 1}}, {});
    const auto s2 = MakeFileStats({{"hello"s, 1}, {"foo"s, 3}}, {});
    const auto total = Aggregate({s1, s2});
    EXPECT_EQ(total.word_frequency.at("hello"s), 3U);
    EXPECT_EQ(total.word_frequency.at("world"s), 1U);
    EXPECT_EQ(total.word_frequency.at("foo"s), 3U);
}

TEST_F(AggregatorTest, Aggregate_MultipleFiles_MergesKeywordHits) {
    const auto s1 = MakeFileStats({}, {{"error"s, 2}});
    const auto s2 = MakeFileStats({}, {{"error"s, 1}, {"warning"s, 3}});
    const auto total = Aggregate({s1, s2});
    EXPECT_EQ(total.keyword_hits.at("error"s), 3U);
    EXPECT_EQ(total.keyword_hits.at("warning"s), 3U);
}

TEST_F(AggregatorTest, Aggregate_MultipleFiles_PopulatesPerFile) {
    FileStats s1;
    s1.line_count = 1;
    FileStats s2;
    s2.line_count = 3;
    const auto total = Aggregate({s1, s2});
    ASSERT_EQ(total.per_file.size(), 2U);
    EXPECT_EQ(total.per_file[0].line_count, 1U);
    EXPECT_EQ(total.per_file[1].line_count, 3U);
}

TEST_F(AggregatorTest, Aggregate_WithZeroKeywordHits_PreservesZeros) {
    const auto s1 = MakeFileStats({}, {{"error"s, 2}, {"warning"s, 0}});
    const auto s2 = MakeFileStats({}, {{"error"s, 0}, {"warning"s, 1}});
    const auto total = Aggregate({s1, s2});
    EXPECT_EQ(total.keyword_hits.at("error"s), 2U);
    EXPECT_EQ(total.keyword_hits.at("warning"s), 1U);
}

TEST_F(AggregatorTest, Aggregate_AllKeywordHitsZero_ContainsZeros) {
    const auto s1 = MakeFileStats({}, {{"critical"s, 0}, {"fatal"s, 0}});
    const auto s2 = MakeFileStats({}, {{"critical"s, 0}, {"fatal"s, 0}});
    const auto total = Aggregate({s1, s2});
    EXPECT_EQ(total.keyword_hits.at("critical"s), 0U);
    EXPECT_EQ(total.keyword_hits.at("fatal"s), 0U);
}

}  // namespace
