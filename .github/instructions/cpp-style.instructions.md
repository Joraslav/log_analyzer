---
description: "Use when writing, reviewing or refactoring C++ code. Covers naming conventions, include order, RAII, error handling, NOLINT policy and documentation style."
applyTo: "**/*.{cpp,hpp}"
---

# Соглашения об именовании

| Сущность | Стиль | Пример |
|---|---|---|
| Классы, структуры | `PascalCase` | `TaskDB`, `ScopedStatement` |
| Методы, функции | `PascalCase` | `AddUser()`, `GetAllTasks()` |
| Переменные, параметры | `snake_case` | `user_id`, `db_path` |
| Приватные поля класса | `snake_case_` (с `_` в конце) | `db_`, `statement_manager_` |
| Параметры конструктора (при совпадении с полем) | `p_` префикс | `p_user_id`, `p_text` |
| `constexpr`/`static` константы | `kPascalCase` | `kUserId`, `kMinLevel` |

Публичные поля структур (`struct`) — `snake_case` **без** завершающего `_`. Суффикс `_` только у приватных полей классов (`class`).
| `static constexpr` строки (в классах) | `UPPER_SNAKE_CASE` | `INSERT_USER`, `SELECT_USER` |
| Пространства имён | `snake_case` | `database`, `bot`, `logging` |
| Файлы заголовков | `PascalCase.hpp` | `TaskDB.hpp`, `ScopedStatement.hpp` |
| Файлы реализации | `PascalCase.cpp` | `TaskDB.cpp` |

# Стиль кода

- Использовать стандарт языка C++23 или новее
- Заголовочная защита: `#pragma once` (не `#ifndef`)
- Порядок `#include`: заголовок класса → стандартная библиотека → сторонние → проектные, каждая группа отдельно
- Использовать `constexpr` везде, где значение можно вычислить на этапе компиляции
- Использовать `const` везде, где объект, ссылка, указатель или параметр не должны изменяться
- Предпочитать `std::string_view` для строковых параметров (read-only), `const std::string&` — когда нужна совместимость
- Использовать `[[nodiscard]]` на функциях, возвращающих ресурс, ID или код ошибки
- `noexcept` — на деструкторах и функциях, которые гарантированно не бросают исключений
- `override` и `final` — обязательны на виртуальных методах и классах
- Копирование запрещать явно через `= delete` когда не нужно
- Анонимные пространства имён для символов уровня translation unit
- `std::move` — при передаче во владение, не при возврате (NRVO)
- Конструкторы помечать `explicit` (в том числе многоаргументные)

# Управление ресурсами (RAII)

- **Никаких владеющих сырых указателей** — владение выражать через `std::unique_ptr`, `std::shared_ptr` или RAII-объекты; сырые указатели допустимы только как невладеющие/optional-ссылки, когда ссылка не подходит по семантике

# Обработка ошибок

- Исключения наследовать от `std::exception` или его потомков
- Если библиотека использует собственный тип исключений, сначала перехватывать исключение библиотеки, затем `std::exception` для собственного кода
- Для кода, который вызывает библиотеку и собственную логику, использовать отдельные блоки `catch`, например: `catch (const Library::Exception& e)`, затем `catch (const std::exception& e)`
- Документировать `@throw` в doxygen-комментарии
- Указывать `@exception_safety` (basic/strong/nothrow) на методах, изменяющих состояние
- `std::optional<T>` — для "может не существовать", не `nullptr`

# NOLINT-комментарии

Перед каждым `// NOLINT(...)` обязателен комментарий с обоснованием:
```cpp
// std::getenv is safe here: single-threaded startup, before any threads are spawned
const char* value = std::getenv(variable_name);  // NOLINT(concurrency-mt-unsafe)
```

# Документирование кода

Doxygen-стиль для публичного API:
```cpp
/**
 * @brief Краткое описание
 * @param user_id ID пользователя
 * @return `std::optional<int64_t>` — ID задачи или `std::nullopt`
 * @throw `std::runtime_error` если ...
 * @warning Операция необратима!
 */
```
