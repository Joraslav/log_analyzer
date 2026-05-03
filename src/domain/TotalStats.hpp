#pragma once

#include "FileStats.hpp"
#include "Types.hpp"

#include <cstddef>
#include <vector>

namespace domain {

struct TotalStats {
    size_t file_count{0};
    size_t line_count{0};
    size_t word_count{0};
    size_t char_count{0};
    WordFrequency word_frequency;
    KeywordHits keyword_hits;
    std::vector<FileStats> per_file;
};

}  // namespace domain
