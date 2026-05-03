#pragma once

#include "domain/Types.hpp"

#include <cstddef>
#include <filesystem>

namespace domain {

struct FileStats {
    std::filesystem::path file_path;
    size_t line_count{0};
    size_t word_count{0};
    size_t char_count{0};
    WordFrequency word_frequency;
    KeywordHits keyword_hits;
};

}  // namespace domain
