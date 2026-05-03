#pragma once

#include "domain/TotalStats.hpp"

#include <filesystem>

namespace report {

/**
 * @brief Сохраняет отчёт о статистике логов в JSON-файл.
 *
 * Сериализует агрегированную статистику через glaze и записывает
 * форматированный JSON в указанный файл.
 *
 * @param stats    Агрегированная статистика по всем файлам.
 * @param out_path Путь к выходному JSON-файлу.
 *
 * @throw std::runtime_error При ошибке сериализации или записи файла.
 */
void WriteJsonReport(const domain::TotalStats& stats,
                     const std::filesystem::path& out_path);

}  // namespace report
