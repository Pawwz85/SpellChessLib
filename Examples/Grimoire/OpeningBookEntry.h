#pragma once

#include <SpellChess/Board.h>
#include <vector>
#include <cstdint>
#include <string>

namespace Grimoire {

    struct GameNode;

    struct OpeningBookEntry {
        uint64_t hash;
        uint16_t weight;
        uint16_t learn;
        SpellChessLib::SpellChessMove move;
    };

    void sort_opening_book( std::vector<OpeningBookEntry> & book);
    void write_to_file(const std::vector<OpeningBookEntry> & book, const std::string & filename);
    std::vector<OpeningBookEntry> gather_entries(GameNode * game_tree);
};