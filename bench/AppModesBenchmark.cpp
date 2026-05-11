#include "analysis/Aggregator.hpp"
#include "analysis/FileAnalyzer.hpp"
#include "concurrency/ThreadPool.hpp"
#include "concurrency/ThreadSafeAggregator.hpp"
#include "domain/FileStats.hpp"
#include "domain/TotalStats.hpp"
#include "domain/Types.hpp"
#include "io/FileScanner.hpp"

#include <benchmark/benchmark.h>

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <future>
#include <span>
#include <stdexcept>
#include <string>
#include <system_error>
#include <vector>

namespace {

namespace fs = std::filesystem;

using analysis::Aggregate;
using analysis::AnalyzeFile;
using concurrency::ThreadPool;
using concurrency::ThreadSafeAggregator;
using domain::FileStats;
using domain::Keywords;
using domain::TotalStats;
using io::ScanDirectory;

[[nodiscard]] TotalStats RunSingleThread(const std::span<const fs::path> paths,
                                         const Keywords& keywords) {
    std::vector<FileStats> file_stats;
    file_stats.reserve(paths.size());

    for (const auto& path : paths) {
        file_stats.push_back(AnalyzeFile(path, keywords));
    }

    return Aggregate(file_stats);
}

[[nodiscard]] TotalStats RunMultiThread(const std::span<const fs::path> paths,
                                        const Keywords& keywords,
                                        const size_t thread_count) {
    ThreadSafeAggregator aggregator;
    ThreadPool pool(thread_count);

    std::vector<std::future<void>> futures;
    futures.reserve(paths.size());

    for (const auto& path : paths) {
        futures.push_back(pool.Enqueue([&aggregator, path, &keywords] {
            aggregator.AddFileStats(AnalyzeFile(path, keywords));
        }));
    }

    for (auto& future : futures) {
        future.get();
    }

    return aggregator.GetTotalStats();
}

[[nodiscard]] const std::vector<fs::path>& DatasetPaths() {
    static const fs::path dataset_dir = [] {
        constexpr size_t kFileCount = 128;
        constexpr size_t kLinesPerFile = 2000;

        const fs::path root =
            fs::temp_directory_path() / "log_analyzer_benchmark_dataset";
        const fs::path marker = root / ".ready";

        std::error_code ec;
        fs::create_directories(root, ec);
        if (ec) {
            throw std::runtime_error(
                "Failed to create benchmark dataset directory");
        }

        if (!fs::exists(marker)) {
            for (size_t file_index = 0; file_index < kFileCount; ++file_index) {
                const fs::path file_path =
                    root / ("generated_" + std::to_string(file_index) + ".log");
                std::ofstream file{file_path};
                if (!file.is_open()) {
                    throw std::runtime_error(
                        "Failed to create benchmark dataset file");
                }

                for (size_t line_index = 0; line_index < kLinesPerFile;
                     ++line_index) {
                    file << "error warning timeout failed line=" << line_index
                         << " file=" << file_index << '\n';
                }
            }

            std::ofstream ready_file{marker};
            if (!ready_file.is_open()) {
                throw std::runtime_error(
                    "Failed to finalize benchmark dataset");
            }
            ready_file << "ok\n";
        }

        return root;
    }();

    static const std::vector<fs::path> paths = [] {
        return ScanDirectory(dataset_dir);
    }();

    return paths;
}

[[nodiscard]] const Keywords& BenchmarkKeywords() {
    static const Keywords keywords = {"error", "warning", "timeout", "failed"};
    return keywords;
}

void BenchmarkSingleThread(benchmark::State& state) {
    const auto& paths = DatasetPaths();
    if (paths.empty()) {
        state.SkipWithError(
            "Generated benchmark dataset is empty or not found");
        return;
    }

    const auto& keywords = BenchmarkKeywords();

    for (auto _ : state) {
        auto stats = RunSingleThread(paths, keywords);
        benchmark::DoNotOptimize(stats);
    }

    state.SetItemsProcessed(static_cast<int64_t>(state.iterations()) *
                            static_cast<int64_t>(paths.size()));
}

void BenchmarkMultiThread(benchmark::State& state) {
    const auto& paths = DatasetPaths();
    if (paths.empty()) {
        state.SkipWithError(
            "Generated benchmark dataset is empty or not found");
        return;
    }

    const auto& keywords = BenchmarkKeywords();
    const auto thread_count = static_cast<size_t>(state.range(0));

    for (auto _ : state) {
        auto stats = RunMultiThread(paths, keywords, thread_count);
        benchmark::DoNotOptimize(stats);
    }

    state.SetItemsProcessed(static_cast<int64_t>(state.iterations()) *
                            static_cast<int64_t>(paths.size()));
}

}  // namespace

BENCHMARK(BenchmarkSingleThread);
BENCHMARK(BenchmarkMultiThread)->Arg(2)->Arg(4)->Arg(8)->Arg(16);
