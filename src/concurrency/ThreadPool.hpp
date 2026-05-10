#pragma once

#include <condition_variable>
#include <cstddef>
#include <functional>
#include <future>
#include <mutex>
#include <queue>
#include <stdexcept>
#include <stop_token>
#include <thread>
#include <type_traits>
#include <utility>
#include <vector>

namespace concurrency {

/**
 * @brief Пул рабочих потоков для параллельного выполнения задач.
 *
 * Управляет фиксированным количеством потоков, принимающих задачи через
 * Enqueue(). Каждая задача выполняется в одном из рабочих потоков и возвращает
 * результат через std::future.
 *
 * Пример использования:
 * @code
 * ThreadPool pool(4);
 * auto result = pool.Enqueue([](int x) { return x * 2; }, 5);
 * int value = result.get();  // Блокирует до завершения, возвращает 10
 * @endcode
 *
 * @warning Все рабочие потоки останавливаются и дожидаются завершения при
 * деструкторе.
 * @thread_safety Метод Enqueue() безопасен для вызова из разных потоков.
 */
class ThreadPool {
 public:
    /**
     * @brief Конструктор пула потоков.
     *
     * @param worker_count Количество рабочих потоков. Если 0 (по умолчанию),
     *                     используется std::jthread::hardware_concurrency().
     * @throw std::runtime_error Если не удалось создать рабочие потоки.
     * @exception_safety strong
     */
    explicit ThreadPool(size_t worker_count = 0);

    /**
     * @brief Постановка задачи в очередь на выполнение.
     *
     * Принимает функцию и её аргументы, создаёт std::packaged_task и возвращает
     * std::future для получения результата.
     *
     * @tparam Func  Тип функции (function object, lambda, function pointer).
     * @tparam Args  Типы аргументов функции.
     *
     * @param func Функция для выполнения. Может быть lambda, function object
     * или указатель.
     * @param args Аргументы, передаваемые в функцию при вызове.
     *
     * @return std::future<ReturnType> — будущий результат выполнения.
     *         Вызовите .get() для получения результата (блокирует до
     * завершения).
     *
     * @throw std::runtime_error Если пул был остановлен (деструктор вызван).
     * @exception_safety strong
     */
    template <typename Func, typename... Args>
    [[nodiscard]] auto Enqueue(Func&& func, Args&&... args)
        -> std::future<std::invoke_result_t<Func, Args...>> {
        using ReturnType = std::invoke_result_t<Func, Args...>;
        using TaskType = std::packaged_task<ReturnType()>;

        auto task = std::make_shared<TaskType>(
            std::bind(std::forward<Func>(func), std::forward<Args>(args)...));

        std::future<ReturnType> result = task->get_future();

        {
            std::lock_guard<std::mutex> lock(queue_mutex_);
            if (stop_) {
                throw std::runtime_error(
                    "Cannot enqueue task: ThreadPool is stopped");
            }
            tasks_.emplace([task]() { (*task)(); });
        }

        condition_variable_.notify_one();
        return result;
    }

    /**
     * @brief Деструктор. Останавливает пул и дожидается завершения всех
     * потоков.
     *
     * Устанавливает флаг stop_ = true, уведомляет все потоки, затем дожидается
     * их завершения.
     *
     * @exception_safety nothrow
     */
    ~ThreadPool() noexcept;

    // Копирование и перемещение запрещены
    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;
    ThreadPool(ThreadPool&&) = delete;
    ThreadPool& operator=(ThreadPool&&) = delete;

 private:
    /**
     * @brief Функция, исполняемая каждым рабочим потоком.
     *
     * В бесконечном цикле ожидает задач из очереди tasks_ и выполняет их,
     * пока флаг stop_ не будет установлен в true.
     */
    void WorkerLoop(const std::stop_token& stop_token) noexcept;

    std::vector<std::jthread> workers_;
    std::queue<std::function<void()>> tasks_;
    std::mutex queue_mutex_;
    std::condition_variable condition_variable_;
    bool stop_ = false;
};

}  // namespace concurrency
