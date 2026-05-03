#include "FileScanner.hpp"

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <stdexcept>
#include <string>
#include <system_error>
#include <vector>

namespace io {

// Brings operator""s into scope; header <string> provides it but clang-tidy
// cannot track UDL operators through transitive includes
using std::string_literals::operator""s;  // NOLINT(misc-include-cleaner)

namespace {

[[nodiscard]] bool IsSupportedExtension(
    const std::filesystem::path& path) noexcept {
    std::string extension = path.extension().string();
    std::ranges::transform(extension, extension.begin(),
                           [](const unsigned char ch) {
                               return static_cast<char>(std::tolower(ch));
                           });

    return extension == ".log" || extension == ".txt";
}

}  // namespace

std::vector<std::filesystem::path> ScanDirectory(
    const std::filesystem::path& root_dir) {
    try {
        if (root_dir.empty()) {
            throw std::invalid_argument{
                "Directory path must not be empty"s};  // NOLINT(misc-include-cleaner)
        }

        std::error_code ec;
        const auto status = std::filesystem::status(root_dir, ec);
        if (ec) {
            if (ec == std::errc::no_such_file_or_directory) {
                throw std::invalid_argument{
                    "Directory does not exist: "s +
                    root_dir.string()};  // NOLINT(misc-include-cleaner)
            }
            throw std::runtime_error{"Failed to inspect directory: "s +
                                     root_dir.string() + ": "s + ec.message()};
        }
        if (!std::filesystem::exists(status)) {
            throw std::invalid_argument{
                "Directory does not exist: "s +
                root_dir.string()};  // NOLINT(misc-include-cleaner)
        }
        if (!std::filesystem::is_directory(status)) {
            throw std::invalid_argument{
                "Path is not a directory: "s +
                root_dir.string()};  // NOLINT(misc-include-cleaner)
        }

        std::vector<std::filesystem::path> result;

        for (const auto& entry :
             std::filesystem::recursive_directory_iterator{root_dir}) {
            if (entry.is_regular_file() && IsSupportedExtension(entry.path())) {
                result.push_back(entry.path());
            }
        }

        return result;
    } catch (const std::filesystem::filesystem_error& ex) {
        throw std::runtime_error{"Filesystem scan failed: "s.append(
            ex.what())};  // NOLINT(misc-include-cleaner)
    }
}

}  // namespace io
