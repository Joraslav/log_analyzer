#include "ThreadPool.hpp"

#include <cstddef>
#include <mutex>
#include <stdexcept>
#include <stop_token>
#include <thread>
#include <utility>

namespace concurrency {

ThreadPool::ThreadPool(size_t worker_count) {
    if (worker_count == 0) {
        worker_count = std::jthread::hardware_concurrency();
        if (worker_count == 0) {
            worker_count = 1;
        }
    }

    try {
        workers_.reserve(worker_count);
        for (size_t i = 0; i < worker_count; ++i) {
            workers_.emplace_back([this](const std::stop_token& stop_token) {
                this->WorkerLoop(stop_token);
            });
        }
    } catch (...) {
        {
            const std::scoped_lock lock(queue_mutex_);
            stop_ = true;
        }
        condition_variable_.notify_all();
        for (auto& worker : workers_) {
            worker.request_stop();
        }
        workers_.clear();
        throw std::runtime_error("Failed to create ThreadPool workers");
    }
}

ThreadPool::~ThreadPool() noexcept {
    {
        const std::scoped_lock lock(queue_mutex_);
        stop_ = true;
    }
    condition_variable_.notify_all();

    for (auto& worker : workers_) {
        worker.request_stop();
    }
    workers_.clear();
}

void ThreadPool::WorkerLoop(const std::stop_token& stop_token) noexcept {
    while (true) {
        std::unique_lock<std::mutex> lock(queue_mutex_);

        condition_variable_.wait(lock, [this, &stop_token] {
            return !tasks_.empty() || stop_ || stop_token.stop_requested();
        });

        if ((stop_ || stop_token.stop_requested()) && tasks_.empty()) {
            break;
        }

        if (!tasks_.empty()) {
            auto task = std::move(tasks_.front());
            tasks_.pop();
            lock.unlock();

            task();
        }
    }
}

}  // namespace concurrency
