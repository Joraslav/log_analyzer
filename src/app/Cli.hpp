#pragma once

#include "CliArgs.hpp"

#include <span>
#include <string_view>

namespace app {

/**
 * @brief Парсит аргументы командной строки.
 *
 * Ожидаемый формат:
 * @code
 * log_analyzer <root_dir> [--keywords word1,word2,...] [--json <path>]
 * @endcode
 *
 * @param args Аргументы командной строки без argv[0].
 * @return Распарсенные аргументы.
 *
 * @throw std::invalid_argument Если аргументы некорректны или отсутствует
 * root_dir.
 */
[[nodiscard]] CliArgs ParseArgs(std::span<const std::string_view> args);

}  // namespace app
