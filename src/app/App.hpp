#pragma once

#include "CliArgs.hpp"

#include <ostream>

namespace app {

/**
 * @brief Запускает анализ логов согласно переданным аргументам.
 *
 * 1. Сканирует директорию на наличие .log/.txt файлов.
 * 2. Анализирует каждый файл (строки, слова, символы, ключевые слова).
 * 3. Агрегирует результаты.
 * 4. Выводит консольный отчёт в @p output.
 * 5. Если указан json_output — сохраняет JSON-отчёт.
 *
 * @param args   Распарсенные аргументы командной строки.
 * @param output Поток для консольного отчёта.
 * @return 0 при успехе, 1 при ошибке.
 */
[[nodiscard]] int Run(const CliArgs& args, std::ostream& output);

}  // namespace app
