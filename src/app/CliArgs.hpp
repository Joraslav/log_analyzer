#pragma once

#include "domain/Types.hpp"

#include <cstddef>
#include <filesystem>
#include <optional>

namespace app {

/**
 * @brief Распарсенные аргументы командной строки.
 */
struct CliArgs {
    std::filesystem::path root_dir;
    domain::Keywords keywords;
    std::optional<std::filesystem::path> json_output;
    std::optional<size_t> thread_count;
};

}  // namespace app
