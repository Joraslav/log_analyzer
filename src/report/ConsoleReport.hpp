#pragma once

#include "domain/TotalStats.hpp"

#include <ostream>

namespace report {

/**
 * @brief Выводит отчёт о статистике логов в поток вывода.
 *
 * Печатает общую статистику (файлы/строки/слова/символы),
 * топ-10 слов по частоте, попадания по ключевым словам
 * и краткую статистику по каждому файлу.
 *
 * @param stats  Агрегированная статистика по всем файлам.
 * @param output Поток для вывода отчёта (например, std::cout).
 */
void PrintReport(const domain::TotalStats& stats, std::ostream& output);

}  // namespace report
