#include "App.hpp"

#include "analysis/Aggregator.hpp"
#include "analysis/FileAnalyzer.hpp"
#include "CliArgs.hpp"
#include "domain/FileStats.hpp"
#include "io/FileScanner.hpp"
#include "report/ConsoleReport.hpp"
#include "report/JsonReport.hpp"

#include <exception>
#include <iostream>
#include <ostream>
#include <vector>

namespace app {

using analysis::Aggregate;
using analysis::AnalyzeFile;
using io::ScanDirectory;
using report::PrintReport;
using report::WriteJsonReport;

int Run(const CliArgs& args, std::ostream& output) {
    try {
        const auto paths = ScanDirectory(args.root_dir);

        std::vector<domain::FileStats> file_stats;
        file_stats.reserve(paths.size());
        for (const auto& path : paths) {
            file_stats.push_back(AnalyzeFile(path, args.keywords));
        }

        const auto total = Aggregate(file_stats);

        PrintReport(total, output);

        if (args.json_output.has_value()) {
            WriteJsonReport(total, *args.json_output);
        }

        return 0;
    } catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << "\n";
        return 1;
    }
}

}  // namespace app
