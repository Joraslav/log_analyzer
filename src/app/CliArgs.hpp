#pragma once

#include "domain/Types.hpp"

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
};

}  // namespace app
