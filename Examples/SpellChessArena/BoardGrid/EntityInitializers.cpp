#include <EntityInitializers.hpp>
#include <PieceSpriteSet.hpp>

void Board::EntityInitializers::initPiece(Board::BoardEntity & entity, PieceColor color, PieceType type){
    static const std::map<Board::PieceType, const char *>  pieceTypeToId = {
        {Board::Pawn,     "pawn"},
        {Board::Bishop,   "bishop"},
        {Board::Knight,   "knight"},
        {Board::Rook,     "rook"},
        {Board::Queen,    "queen"},
        {Board::King,     "king"}    
    };

    QString spriteId = QString(color == White ? "white_" : "black_") + pieceTypeToId.at(type);

    PieceSpriteSet & spriteSet = PieceSpriteSet::getInstance();

    entity.setSprite(spriteSet.getSprite(spriteId));
    entity.setProperty("location", QPointF(0.5, 0.5));
    entity.setProperty("size", QSizeF(1.f, 1.f));
}; 
