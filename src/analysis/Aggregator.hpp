#pragma once

#include "domain/FileStats.hpp"
#include "domain/TotalStats.hpp"

#include <vector>

namespace analysis {

/**
 * @brief Агрегирует статистику по нескольким файлам в общую.
 *
 * Суммирует счётчики строк, слов, символов; объединяет частоты слов
 * и попадания по ключевым словам; сохраняет статистику каждого файла.
 *
 * @param file_stats Список статистик по отдельным файлам.
 * @return Агрегированная статистика по всем файлам.
 * @exception_safety strong
 */
[[nodiscard]] domain::TotalStats Aggregate(
    const std::vector<domain::FileStats>& file_stats);

}  // namespace analysis
