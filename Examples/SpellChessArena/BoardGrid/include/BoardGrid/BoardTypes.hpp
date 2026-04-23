#pragma once

namespace Board {
    enum PieceColor {
            White,
            Black
    };

    enum PieceType {
            Pawn,
            Bishop,
            Knight,
            Rook,
            Queen,
            King
    };
    
    // used by promotion context menu
    enum PawnPromotionAction {
        PromoteToBishop,
        PromoteToKnight,
        PromoteToRook,
        PromoteToQueen
    };
};