#pragma once

#include <BoardGrid/BoardGridModel.h>
#include <SpellChess/SpellChessGame.h>
#include <array>
#include <optional>

namespace BoardViewer {

   struct LinterInput {
        SpellChessLib::GameState state;
        SpellChessLib::PlayerColor playerColor;
        SpellChessLib::SquareIndex selectedSquare;
        SpellChessLib::SquareIndex previewedFreezeSquare;
        SpellChessLib::SquareIndex previewedJumpableSquare;
        std::optional<SpellChessLib::SpellIdentificator> selectedSpell;
        bool isDragging;
   };


   namespace Internal {  
        uint64_t getMovablePieces(const LinterInput & input);
        uint64_t getValidTargetSquares(const LinterInput & input);   
        uint64_t getInteractiveSquares(const LinterInput & input);
        uint64_t getFrozenSquares(const LinterInput & input);
        uint64_t getJumpableSquares(const LinterInput & input);
        uint64_t getOccupiedSquares(const LinterInput & input);
        uint64_t getSquaresWithSnowflakes(const LinterInput & input);
        uint64_t getSquaresWithRings(const LinterInput & input);
    };

   void lintBoard(Board::BoardGridModel & model, const LinterInput & input); 
   uint64_t getDraggablePieces(const LinterInput & input);
};