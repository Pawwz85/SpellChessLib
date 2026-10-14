#pragma once

#include <cstddef>
#include <vector>

namespace SpellChessLib {
    
    class GameState; 
    struct SpellChessMove;

    class GameHistory {
        public:
        virtual ~GameHistory() = default;
        virtual size_t countOccurences(const GameState & position) const = 0;
    };

    enum class ArbiterVerdict {
        None,
        Checkmate,
        KingCapture,
        Stalemate,
        DrawByInsufficientMaterial,
        DrawByFiftyMoveRule,
        ThreefoldRepetition
    };

    extern ArbiterVerdict arbite(const GameState & pos, const GameHistory & history);
    extern ArbiterVerdict arbite(const GameState & root, const std::vector<SpellChessLib::SpellChessMove> & movesSinceRoot);
}