#pragma once

#include <string>
#include <filesystem>
#include <vector>

namespace Extractor {

    class PGN4Supplier {
        public:
        virtual ~PGN4Supplier() = default;
        virtual bool hasNext() const = 0;
        virtual std::string getNext() const = 0;
    };

    class FilesystemPGN4Supplier final : public PGN4Supplier {
        std::vector<std::filesystem::path> paths;
        mutable size_t current = 0;

    public:
        FilesystemPGN4Supplier(const std::filesystem::path & path, bool recursive);
        bool hasNext() const override { return current < paths.size(); }
        std::string getNext() const override;
        static std::vector<std::filesystem::path> discover(const std::filesystem::path & path, bool recursive);
    };

}