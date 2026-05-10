#pragma once

#include "domain/FileStats.hpp"
#include "domain/TotalStats.hpp"

#include <mutex>

namespace concurrency {

/**
 * @brief Потокобезопасный агрегатор статистики по файлам.
 *
 * Позволяет нескольким потокам одновременно добавлять статистику по файлам,
 * при этом корректно обновляя общую агрегированную статистику.
 *
 * Использует std::mutex для синхронизации доступа к общей статистике.
 * Каждый вызов AddFileStats выполняется под lock_guard, что гарантирует
 * консистентность данных.
 *
 * Типичное использование с ThreadPool:
 * @code
 * concurrency::ThreadPool pool(4);
 * concurrency::ThreadSafeAggregator agg;
 *
 * for (const auto& path : file_paths) {
 *     pool.Enqueue([&agg, &path](const auto& keywords) {
 *         auto stats = analysis::AnalyzeFile(path, keywords);
 *         agg.AddFileStats(stats);  // Потокобезопасно!
 *     }, keywords);
 * }
 * // Дождаться завершения всех потоков...
 * auto result = agg.GetTotalStats();
 * @endcode
 *
 * @warning Не копируется и не переносится (копирование запрещено явно через
 * delete).
 * @thread_safety Все публичные методы потокобезопасны для параллельного вызова.
 */
class ThreadSafeAggregator {
 public:
    /**
     * @brief Конструктор по умолчанию. Инициализирует агрегатор с нулевой
     * статистикой.
     *
     * Создаёт новый мьютекс и инициализирует total_stats_ пустыми значениями.
     *
     * @exception_safety nothrow
     */
    ThreadSafeAggregator() = default;

    /**
     * @brief Деструктор.
     *
     * Освобождает мьютекс и все накопленные данные. Гарантирует, что все
     * параллельные операции завершены перед удалением объекта.
     *
     * @exception_safety nothrow
     */
    ~ThreadSafeAggregator() = default;

    /**
     * @brief Добавить статистику по файлу в агрегированный результат.
     *
     * Потокобезопасно обновляет накопленную статистику: увеличивает счётчики
     * (файлов, строк, слов, символов), объединяет частоты слов и попадания по
     * ключевым словам, сохраняет информацию о файле в список per_file.
     *
     * @param file_stats Статистика по одному файлу, полученная из FileAnalyzer.
     *
     * @exception_safety strong (либо успешное обновление, либо без изменений)
     * @thread_safety Безопасен для вызова из разных потоков одновременно.
     */
    void AddFileStats(const domain::FileStats& file_stats);

    /**
     * @brief Получить копию полной агрегированной статистики.
     *
     * Возвращает полный снимок накопленной статистики на момент вызова.
     * Результат не будет изменён параллельными вызовами AddFileStats из других
     * потоков.
     *
     * @return domain::TotalStats — полная статистика по всем обработанным
     * файлам: file_count, line_count, word_count, char_count, word_frequency,
     *         keyword_hits, per_file.
     *
     * @exception_safety nothrow (только копирование данных)
     * @thread_safety Безопасен для вызова из разных потоков, включая
     *                одновременные вызовы AddFileStats.
     */
    [[nodiscard]] domain::TotalStats GetTotalStats() const;

    /**
     * @brief Очистить накопленную статистику и привести агрегатор в начальное
     * состояние.
     *
     * После вызова все счётчики обнуляются, словари очищаются, список per_file
     * становится пустым. Используется для повторного использования агрегатора
     * на новом наборе данных.
     *
     * @exception_safety nothrow
     * @thread_safety Безопасен для вызова из разных потоков, но гарантии даёт
     *                только сама операция очистки; до и после Reset() могут
     * быть параллельные вызовы AddFileStats.
     */
    void Reset();

    // Копирование и перемещение запрещены (класс не должен копироваться
    // из-за мьютекса, и перемещение может нарушить инварианты)
    ThreadSafeAggregator(const ThreadSafeAggregator&) = delete;
    ThreadSafeAggregator& operator=(const ThreadSafeAggregator&) = delete;
    ThreadSafeAggregator(ThreadSafeAggregator&&) = delete;
    ThreadSafeAggregator& operator=(ThreadSafeAggregator&&) = delete;

 private:
    /// Мьютекс для синхронизации доступа к total_stats_. Помечен mutable
    /// чтобы позволить GetTotalStats() оставаться const методом.
    mutable std::mutex mutex_;

    /// Накопленная статистика по всем файлам. Доступ защищён mutex_.
    domain::TotalStats total_stats_;
};

}  // namespace concurrency