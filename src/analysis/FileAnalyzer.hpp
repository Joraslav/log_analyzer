#pragma once

#include "domain/FileStats.hpp"
#include "domain/Types.hpp"

#include <filesystem>

namespace analysis {

/**
 * @brief Анализирует один файл и возвращает статистику по нему.
 *
 * Читает файл построчно, токенизирует каждую строку и собирает:
 * число строк, слов, символов, частоту слов и попадания по ключевым словам.
 *
 * @param file_path Путь к файлу для анализа.
 * @param keywords  Список ключевых слов для поиска (ожидаются в нижнем
 * регистре).
 * @return Статистика по файлу.
 *
 * @throw std::invalid_argument Если путь пуст или файл не существует.
 * @throw std::runtime_error    При ошибке открытия или чтения файла.
 * @exception_safety strong
 */
[[nodiscard]] domain::FileStats AnalyzeFile(
    const std::filesystem::path& file_path, const domain::Keywords& keywords);

}  // namespace analysis
