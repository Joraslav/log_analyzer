#include "App.hpp"

#include "analysis/Aggregator.hpp"
#include "analysis/FileAnalyzer.hpp"
#include "CliArgs.hpp"
#include "concurrency/ThreadPool.hpp"
#include "concurrency/ThreadSafeAggregator.hpp"
#include "domain/FileStats.hpp"
#include "domain/TotalStats.hpp"
#include "domain/Types.hpp"
#include "io/FileScanner.hpp"
#include "report/ConsoleReport.hpp"
#include "report/JsonReport.hpp"

#include <cstddef>
#include <exception>
#include <filesystem>
#include <future>
#include <iostream>
#include <ostream>
#include <span>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

using analysis::Aggregate;
using analysis::AnalyzeFile;
using app::CliArgs;
using concurrency::ThreadPool;
using concurrency::ThreadSafeAggregator;
using domain::FileStats;
using domain::Keywords;
using domain::TotalStats;
using io::ScanDirectory;
using report::PrintReport;
using report::WriteJsonReport;
namespace fs = std::filesystem;

[[nodiscard]] TotalStats RunSingleThread(const std::span<const fs::path> paths,
                                         const Keywords& keywords) {
    try {
        std::vector<FileStats> file_stats;
        file_stats.reserve(paths.size());

        for (const auto& path : paths) {
            file_stats.push_back(AnalyzeFile(path, keywords));
        }

        return Aggregate(file_stats);
    } catch (const std::exception& ex) {
        throw std::runtime_error("Error: " + std::string{ex.what()});
    }
}

[[nodiscard]] TotalStats RunMultiThread(const std::span<const fs::path> paths,
                                        const Keywords& keywords,
                                        const size_t thread_count) {
    try {
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
    } catch (const std::exception& ex) {
        throw std::runtime_error("Error: " + std::string{ex.what()});
    }
}

}  // namespace

namespace app {

int Run(const CliArgs& args, std::ostream& output) {
    try {
        const auto paths = ScanDirectory(args.root_dir);

        const TotalStats stats =
            args.thread_count.has_value()
                ? RunMultiThread(paths, args.keywords, *args.thread_count)
                : RunSingleThread(paths, args.keywords);

        PrintReport(stats, output);

        if (args.json_output.has_value()) {
            WriteJsonReport(stats, *args.json_output);
        }

        return 0;
    } catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << "\n";
        return 1;
    }
}

}  // namespace app
