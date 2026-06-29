#pragma once

#include <BoardGrid/BoardGridModel.h>
#include <SpellChess/SpellChessGame.h>
#include <array>

namespace BoardViewer {

   struct LinterInput {
        SpellChessLib::GameState state;
        SpellChessLib::PlayerColor playerColor;
        SpellChessLib::SquareIndex selectedSquare;
        SpellChessLib::SquareIndex previewedFreezeSquare;
        SpellChessLib::SquareIndex previewedJumpableSquare;
   };

   namespace Internal {  
        uint64_t getMovablePieces(const LinterInput & input);
        uint64_t getValidTargetSquares(const LinterInput & input);   
        uint64_t getInteractiveSquares(const LinterInput & input);
        uint64_t getFrozenSquares(const LinterInput & input);
        uint64_t getJumpableSquares(const LinterInput & input);
    };

   void lintBoard(Board::BoardGridModel & model, const LinterInput & input); 
   uint64_t getDraggablePieces(const LinterInput & input);
};