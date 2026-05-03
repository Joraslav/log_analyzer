#include "app/App.hpp"
#include "app/Cli.hpp"
#include "app/CliArgs.hpp"

#include <cstddef>
#include <exception>
#include <iostream>
#include <span>
#include <string_view>
#include <vector>

int main(int argc, char* argv[]) {
    std::vector<std::string_view> raw_args;
    if (argc > 1) {
        // Skip argv[0] (program name), convert remaining args to string_view.
        const std::span<char*> argv_span{argv, static_cast<size_t>(argc)};
        raw_args.reserve(argv_span.size() - 1);
        for (const char* const arg : argv_span.subspan(1)) {
            raw_args.emplace_back(arg);
        }
    }
    try {
        const app::CliArgs args =
            app::ParseArgs(std::span<const std::string_view>{raw_args});
        return app::Run(args, std::cout);
    } catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << "\n";
        return 1;
    }
}
