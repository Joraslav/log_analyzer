#pragma once

#include <filesystem>
#include <vector>

namespace io {

/**
 * @brief Рекурсивно сканирует директорию и возвращает пути к файлам .log и
 * .txt.
 *
 * @param root_dir Корневая директория для сканирования.
 * @return Список путей к файлам с расширениями .log и .txt.
 *
 * @throw std::invalid_argument Если путь пуст, директория не существует
 *        или путь не является директорией.
 * @throw std::runtime_error При ошибках std::filesystem во время обхода.
 * @exception_safety strong
 */
[[nodiscard]] std::vector<std::filesystem::path> ScanDirectory(
    const std::filesystem::path& root_dir);

}  // namespace io
