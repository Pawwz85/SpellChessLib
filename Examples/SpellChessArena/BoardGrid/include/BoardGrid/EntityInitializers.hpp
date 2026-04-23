#pragma once 

#include "BoardTypes.hpp"
#include "BoardEntity.h"

namespace Board {

    namespace EntityInitializers {
        extern void initPiece(Board::BoardEntity & entity, PieceColor color, PieceType type); 
    };

};