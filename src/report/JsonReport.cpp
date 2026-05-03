#include "JsonReport.hpp"

#include "domain/FileStats.hpp"
#include "domain/TotalStats.hpp"
#include "domain/Types.hpp"

#include <glaze/core/reflect.hpp>
#include <glaze/json/write.hpp>

#include <cstddef>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace report {

using domain::TotalStats;

namespace dto {

using domain::KeywordHits;
using domain::WordFrequency;

// Glaze reflection requires external linkage for reflected types.
// NOLINTNEXTLINE(misc-use-internal-linkage)
struct FileStatsDto {
    std::string file_path;
    size_t line_count{0};
    size_t word_count{0};
    size_t char_count{0};
    WordFrequency word_frequency;
    KeywordHits keyword_hits;
};

// Glaze reflection requires external linkage for reflected types.
// NOLINTNEXTLINE(misc-use-internal-linkage)
struct TotalStatsDto {
    size_t file_count{0};
    size_t line_count{0};
    size_t word_count{0};
    size_t char_count{0};
    WordFrequency word_frequency;
    KeywordHits keyword_hits;
    std::vector<FileStatsDto> per_file;
};

}  // namespace dto

namespace {

using domain::FileStats;
using domain::TotalStats;

[[nodiscard]] dto::FileStatsDto ToDto(const FileStats& fs) {
    return dto::FileStatsDto{
        .file_path = fs.file_path.string(),
        .line_count = fs.line_count,
        .word_count = fs.word_count,
        .char_count = fs.char_count,
        .word_frequency = fs.word_frequency,
        .keyword_hits = fs.keyword_hits,
    };
}

[[nodiscard]] dto::TotalStatsDto ToDto(const TotalStats& stats) {
    std::vector<dto::FileStatsDto> per_file;
    per_file.reserve(stats.per_file.size());
    for (const auto& fs : stats.per_file) {
        per_file.push_back(ToDto(fs));
    }

    return dto::TotalStatsDto{
        .file_count = stats.file_count,
        .line_count = stats.line_count,
        .word_count = stats.word_count,
        .char_count = stats.char_count,
        .word_frequency = stats.word_frequency,
        .keyword_hits = stats.keyword_hits,
        .per_file = std::move(per_file),
    };
}

}  // namespace

void WriteJsonReport(const TotalStats& stats,
                     const std::filesystem::path& out_path) {
    const dto::TotalStatsDto dto = ToDto(stats);

    std::string buffer;
    const auto err = glz::write_json(dto, buffer);
    if (err) {
        throw std::runtime_error{"glaze write_json failed: " +
                                 std::string{glz::format_error(err, buffer)}};
    }

    std::ofstream out{out_path};
    if (!out.is_open()) {
        throw std::runtime_error{"Failed to write JSON report to: " +
                                 out_path.string()};
    }

    out << buffer;
    if (!out.good()) {
        throw std::runtime_error{"Failed to write JSON report to: " +
                                 out_path.string()};
    }
}

}  // namespace report
