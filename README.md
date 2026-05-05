# log_analyzer

Консольная утилита на C++23 для статистического анализа текстовых лог-файлов. Сканирует директорию, считает строки, слова и символы, строит частотный словарь, ищет ключевые слова и выводит результаты в консоль или в JSON.

## Возможности

- Рекурсивный обход директории — находит все файлы с расширениями `.log` и `.txt`
- Подсчёт строк, слов и символов для каждого файла и суммарно
- Частотный словарь слов (топ-10 в консольном отчёте, полный список в JSON)
- Поиск произвольных ключевых слов с подсчётом совпадений
- Консольный отчёт с сводной таблицей по каждому файлу
- Экспорт полного отчёта в JSON (библиотека [glaze](https://github.com/stephenberry/glaze))
- Покрытие юнит-тестами через GoogleTest (8 наборов тестов)
- Поддержка Address Sanitizer / UBSan в режиме Debug

## Требования

| Инструмент | Минимальная версия |
|---|---|
| GCC / Clang | C++23 (GCC 15, Clang 17+) |
| CMake | 3.31 |
| Conan | 2.x |

## Быстрый старт

### 1. Установка зависимостей (Conan)

```bash
./scripts/create_env.sh Debug
```

Скрипт устанавливает зависимости через Conan и генерирует `CMakeUserPresets.json`.  
На Fedora автоматически применяются профили `Fedora-Debug` / `Fedora-Release`.  
Для Release-сборки:

```bash
./scripts/create_env.sh Release
```

### 2. Конфигурация и сборка

```bash
cmake --preset conan-debug
cmake --build build
```

Для Release:

```bash
cmake --preset conan-release
cmake --build build
```

### 3. Запуск

```bash
./build/log_analyzer <директория> [--keywords слово1,слово2,...] [--json <путь>]
```

**Примеры:**

```bash
# Проанализировать директорию logs/
./build/log_analyzer logs/

# Искать ключевые слова "error" и "warning", сохранить JSON
./build/log_analyzer logs/ --keywords error,warning --json report.json
```

## Аргументы командной строки

| Аргумент | Обязателен | Описание |
|---|---|---|
| `<root_dir>` | Да | Корневая директория для сканирования |
| `--keywords word1,word2,...` | Нет | Список ключевых слов через запятую (без пробелов) |
| `--json <path>` | Нет | Путь к выходному JSON-файлу |

## Пример вывода

```
=== Log Analyzer Report ===
----------------------------------------
Files analyzed : 3
Total lines    : 1024
Total words    : 8743
Total chars    : 61200
----------------------------------------
--- Top 10 words ---
  error: 142
  warning: 98
  info: 76
  ...

--- Keyword hits ---
  error: 142
  warning: 98

--- Per-file stats ---
  logs/app.log        lines=512  words=4200  chars=30100
  logs/system.log     lines=300  words=2800  chars=20500
  logs/debug.log      lines=212  words=1743  chars=10600
```

### Пайплайн обработки

```planetext
CLI args → FileScanner → FileAnalyzer (×N файлов) → Aggregator → ConsoleReport
                                                                 └→ JsonReport
```

1. `FileScanner` — рекурсивно находит `.log`/`.txt` файлы
2. `FileAnalyzer` — читает файл построчно, токенизирует через `Tokenizer`, собирает `FileStats`
3. `Aggregator` — суммирует все `FileStats` в `TotalStats`
4. `ConsoleReport` — печатает сводку в `std::ostream`
5. `JsonReport` — сериализует `TotalStats` в JSON через glaze

## Тесты

```bash
ctest --test-dir build --output-on-failure
```

Наборы тестов: `Test_FileScanner`, `Test_Parser`, `Test_FileAnalyzer`, `Test_Aggregator`, `Test_ConsoleReport`, `Test_JsonReport`, `Test_Cli`, `Test_App`.

## Дополнительные инструменты

### Sanitizers (ASan + UBSan)

```bash
cmake --preset conan-debug -DLOG_ANALYZER_ENABLE_SANITIZERS=ON
cmake --build build
```

Включается только для Debug-сборок на GCC/Clang. Перед включением автоматически проверяется поддержка компилятора.

### clang-tidy

```bash
./scripts/check_tidy.sh --build-type Debug
# Автоматически исправить предупреждения:
./scripts/check_tidy.sh --build-type Debug --fix
```

### clang-format

```bash
# Применить форматирование ко всем файлам src/ и tests/:
./scripts/fix_style.sh

# Только показать список файлов без изменений:
./scripts/fix_style.sh --list
```

### Очистка сборки

```bash
./scripts/clean.sh
```

## Зависимости

| Библиотека | Версия | Назначение |
|---|---|---|
| [glaze](https://github.com/stephenberry/glaze) | 7.4.0 | JSON-сериализация |
| [GoogleTest](https://github.com/google/googletest) | 1.17.0 | Юнит-тестирование |

Зависимости управляются через **Conan 2** и не требуют ручной установки.
