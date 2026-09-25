#include "PGN4Supplier.h"

#include <fstream>
#include <algorithm>
#include <sstream>
#include <stdexcept>

namespace Extractor {

std::vector<std::filesystem::path> FilesystemPGN4Supplier::discover(const std::filesystem::path & path, bool recursive) {
    if (!std::filesystem::exists(path)) throw std::invalid_argument("path does not exist: " + path.string());
    if (std::filesystem::is_regular_file(path)) return {path};
    if (!std::filesystem::is_directory(path)) throw std::invalid_argument("path is not a file or directory: " + path.string());

    std::vector<std::filesystem::path> result;
    auto accept = [](const std::filesystem::path & candidate) {
        const std::string extension = candidate.extension().string();
        return extension == ".pgn4" || extension == ".pgn";
    };
    if (recursive) {
        for (const auto & entry : std::filesystem::recursive_directory_iterator(path))
            if (entry.is_regular_file() && accept(entry.path())) result.push_back(entry.path());
    } else {
        for (const auto & entry : std::filesystem::directory_iterator(path))
            if (entry.is_regular_file() && accept(entry.path())) result.push_back(entry.path());
    }
    std::sort(result.begin(), result.end());
    return result;
}

FilesystemPGN4Supplier::FilesystemPGN4Supplier(const std::filesystem::path & path, bool recursive)
    : paths(discover(path, recursive)) {}

std::string FilesystemPGN4Supplier::getNext() const {
    if (!hasNext()) throw std::out_of_range("no PGN4 files remaining");
    const auto path = paths[current++];
    std::ifstream input(path, std::ios::binary);
    if (!input) throw std::runtime_error("cannot read file: " + path.string());
    std::ostringstream contents;
    contents << input.rdbuf();
    if (input.bad()) throw std::runtime_error("error reading file: " + path.string());
    return contents.str();
}

}