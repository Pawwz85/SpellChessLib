#include "OpeningBookEntry.h"

#include <algorithm>
#include <cstdio>
#include <queue>
#include <SpellChess/ZobristHash.h>

void Grimoire::sort_opening_book(std::vector<OpeningBookEntry> &book)
{
    std::sort(book.begin(), book.end(), [](const OpeningBookEntry & a1, const OpeningBookEntry & a2){
        return a1.hash < a2.hash;
    });
}

char encodePromotionTarget(SpellChessLib::PieceType type) {
    switch (type)
    {
    case SpellChessLib::Knight:
        return 1;
    case SpellChessLib::Bishop:
        return 2;
    case SpellChessLib::Rook:
        return 3;
    case SpellChessLib::Queen:
        return 4;
    default:
        return 0;
    }
};

void Grimoire::write_to_file(const std::vector<OpeningBookEntry> &book, const std::string &filename){
    /*
        Each entries ihas dedicated 192 bits (24 bytes):
            - 64 bits for hash   (little endian)
            - 16 bits for weight (little endian)
            - 16 bits for learn  (little endian)
            - 8 bits  for from square
            - 8 bits  for to square
            - 8 bits  for freeze square
            - 8 bits  for jump square
            - 8 bits  for promotion target
            - 56 bits for padding
    */

    FILE * file = fopen(filename.c_str(), "wb");
    char buffer[24];   
    std::fill_n(buffer, 24, 0);
    
    for (auto & entry : book) {
        uint64_t tmp = entry.hash;

        for (size_t i = 0; i < 8 ; ++i){
            buffer[i] = tmp & 255;
            tmp >>= 8;
        };

        buffer[8]  = entry.weight & 255;
        buffer[9]  = entry.weight >> 8;
        buffer[10] = entry.learn & 255;
        buffer[11] = entry.learn >> 8;
        buffer[12] = entry.move.from;
        buffer[13] = entry.move.to;
        buffer[14] = entry.move.freezeSquare;
        buffer[15] = entry.move.jumpSquare;
        buffer[16] = encodePromotionTarget(entry.move.promoteTo);

        fwrite(buffer, 1, 24, file);
    };

    fclose(file);
}

std::vector<Grimoire::OpeningBookEntry> Grimoire::gather_entries(GameNode *game_tree){
    std::vector<OpeningBookEntry> result;
    std::queue<GameNode *> nodes;
    nodes.push(game_tree);

    GameNode * currentNode;

    while(!nodes.empty()) {
        currentNode = nodes.front();
        nodes.pop();

        uint64_t hash = SpellChessLib::calculateHash(currentNode->state);

        if (!currentNode->expanded || currentNode->pruned || currentNode->children == nullptr)
            continue;


        for(size_t i = 0; i < currentNode->childrenCount; ++i)
            if(currentNode->children[i].evaluated && !currentNode->children[i].pruned) {
            int eval = -currentNode->children[i].evalution; 
            const float a = 100.f;
            const float b = 50.f;
            float exponent = exp(-(float(eval)-a)/b);

            if(exponent == HUGE_VALF) {
                continue;
            };

            float win_rate = 1.f / (1.f + exponent);
            uint16_t weight = 100000 * win_rate;

            result.push_back({hash, weight, 0, currentNode->children[i].move});

            nodes.emplace(currentNode->children + i);
        };
    }

    sort_opening_book(result);
    return result;
}
