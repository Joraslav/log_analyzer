#pragma once

#include <cstddef>
#include <string>
#include <unordered_map>
#include <vector>

namespace domain {

using WordFrequency = std::unordered_map<std::string, size_t>;
using KeywordHits = std::unordered_map<std::string, size_t>;
using Keywords = std::vector<std::string>;

}  // namespace domain
