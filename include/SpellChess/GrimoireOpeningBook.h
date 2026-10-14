#pragma once

#include "Board.h"
#include "OpeningBook.h"
#include <mutex>
#include <cstdint>
#include <cstdio>
#include <filesystem>
#include <memory>
#include <vector>

namespace SpellChessLib {

    class GrimoireOpeningBook : public OpeningBook {
        mutable FILE * file;
        mutable std::mutex mtx;

        struct entry {
            uint64_t hash;
            uint16_t weight;
            uint16_t learn;
            SpellChessLib::SpellChessMove move;
        };

        // return Index of first entry with given hash, return -1 if not found
        long  findPosition(uint64_t hash) const;

        // returns file position of given index
        long calculateFilePositionByIndex(long index) const;

        // load hash of entry at given index
        uint64_t load_hash(long index) const;

        // load whole entry at given index
        entry loadEntry(long index) const;

        // load all entries associated with given position
        std::vector<entry> loadAllEntries(uint64_t hash) const;

        public:
         GrimoireOpeningBook(const std::filesystem::path & path);
        ~GrimoireOpeningBook();

        // check whatever given position exists within the book
        bool containsPosition(const GameState & state) const override;

        // pick move using weighs stored in the book
        std::optional<SpellChessMove> pickRandomMove(const GameState & state) const override;
        
        // pick random move ignoring weighrs stored in the book
        std::optional<SpellChessMove> pickRandomUniformMove(const GameState & state) const override;

        // count entries in the book
        long count() const override;

    };

    class GrimoireOpeningBookLoader : public OpeningBookLoader {
        public:
        std::shared_ptr<OpeningBook> loadOpeningBook(const std::filesystem::path & path) const override {
            return std::make_shared<GrimoireOpeningBook>(path);
        };
    };

};