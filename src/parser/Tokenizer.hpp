#pragma once

#include <string>
#include <string_view>
#include <vector>

namespace parser {

/**
 * @brief Разбивает текст по пробелам, удаляет пунктуацию по краям токенов и
 * приводит токены к нижнему регистру.
 *
 * @return Вектор нормализованных токенов.
 *
 * Пример: "[Error], CODE-42!" -> {"error", "code-42"}
 */
[[nodiscard]] std::vector<std::string> TokenizeWhitespaceTrimLower(
    std::string_view text);

}  // namespace parser
