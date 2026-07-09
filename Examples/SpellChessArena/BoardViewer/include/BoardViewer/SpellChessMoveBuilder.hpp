#pragma once

#include <SpellChess/Board.h>

namespace BoardViewer {

    class SpellChessMoveBuilder {
        SpellChessLib::SpellChessMove currentValue;
        public:
        SpellChessMoveBuilder();
        SpellChessMoveBuilder & setFromSquare(uint8_t fromSq);
        SpellChessMoveBuilder & setDestSquare(uint8_t destSq);
        SpellChessMoveBuilder & setFreezeSquare(uint8_t freezeSq);
        SpellChessMoveBuilder & setJumpSquare(uint8_t jumpSq);
        SpellChessMoveBuilder & setPromotion(SpellChessLib::PieceType promotionTarget);
        SpellChessMoveBuilder & reset();
        SpellChessMoveBuilder & clearSquares();
        SpellChessMoveBuilder & clearSpells();
        SpellChessLib::SpellChessMove build();  
    };

};