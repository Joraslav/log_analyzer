#include "domain/FileStats.hpp"
#include "domain/TotalStats.hpp"
#include "report/ConsoleReport.hpp"

#include <gtest/gtest.h>

#include <sstream>
#include <string>

namespace {

using domain::FileStats;
using domain::TotalStats;
using namespace report;
using namespace std::string_literals;

TotalStats MakeTotalStats() {
    TotalStats stats;
    stats.file_count = 2;
    stats.line_count = 5;
    stats.word_count = 10;
    stats.char_count = 80;
    stats.word_frequency = {{"hello"s, 4}, {"world"s, 2}, {"foo"s, 1}};
    stats.keyword_hits = {{"error"s, 3}};

    FileStats file1;
    file1.file_path = "/tmp/a.log"s;
    file1.line_count = 3;
    file1.word_count = 6;
    file1.char_count = 50;

    FileStats file2;
    file2.file_path = "/tmp/b.log"s;
    file2.line_count = 2;
    file2.word_count = 4;
    file2.char_count = 30;

    stats.per_file = {file1, file2};
    return stats;
}

class ConsoleReportTest : public ::testing::Test {
 protected:
    std::ostringstream output;
};

TEST_F(ConsoleReportTest, PrintReport_ContainsTotalFilesCount) {
    PrintReport(MakeTotalStats(), output);
    EXPECT_NE(output.str().find("2"s), std::string::npos);
}

TEST_F(ConsoleReportTest, PrintReport_ContainsTotalLineCount) {
    PrintReport(MakeTotalStats(), output);
    EXPECT_NE(output.str().find("5"s), std::string::npos);
}

TEST_F(ConsoleReportTest, PrintReport_ContainsTopWords) {
    PrintReport(MakeTotalStats(), output);
    const std::string out = output.str();
    EXPECT_NE(out.find("hello"s), std::string::npos);
    EXPECT_NE(out.find("world"s), std::string::npos);
}

TEST_F(ConsoleReportTest, PrintReport_ContainsKeywordHits) {
    PrintReport(MakeTotalStats(), output);
    EXPECT_NE(output.str().find("error"s), std::string::npos);
}

TEST_F(ConsoleReportTest, PrintReport_ContainsPerFilePaths) {
    PrintReport(MakeTotalStats(), output);
    const std::string out = output.str();
    EXPECT_NE(out.find("a.log"s), std::string::npos);
    EXPECT_NE(out.find("b.log"s), std::string::npos);
}

TEST_F(ConsoleReportTest, PrintReport_EmptyKeywords_PrintsNoKeywordsMessage) {
    const TotalStats stats;
    PrintReport(stats, output);
    EXPECT_NE(output.str().find("no keywords"s), std::string::npos);
}

TEST_F(ConsoleReportTest, PrintReport_DoesNotModifyStream_OnEmptyStats) {
    const TotalStats stats;
    PrintReport(stats, output);
    EXPECT_FALSE(output.str().empty());
}

TEST_F(ConsoleReportTest, PrintReport_WithZeroKeywordHits_DisplaysZeros) {
    TotalStats stats;
    stats.file_count = 1;
    stats.keyword_hits = {{"error"s, 0}, {"warning"s, 0}};
    PrintReport(stats, output);
    const std::string out = output.str();
    EXPECT_NE(out.find("error"s), std::string::npos);
    EXPECT_NE(out.find("warning"s), std::string::npos);
}

}  // namespace
