#include <EmptyBoardWidget.hpp>

#include <BoardGrid/EntityInitializers.hpp>

QPointF pointFromSquareIndex(unsigned int sqId)
{
    if(sqId >= 64)
        return QPointF();
    
    unsigned int x = sqId & 7;
    unsigned int y = sqId >> 3;
    return QPointF(x, y);
}

EmptyBoardWidget::EmptyBoardWidget() {

    const Board::PieceType pieceRow[] = {
        Board::Rook, Board::Knight, Board::Bishop, Board::Queen, Board::King, Board::Bishop, Board::Knight, Board::Rook};
    
    for (size_t index = 0; index < 8; ++index) {
        auto & e = entities.addNewEntity();
        Board::EntityInitializers::initPiece(e, Board::Black, pieceRow[index]);
        e.setProperty("location", pointFromSquareIndex(index));

        auto & e2 = entities.addNewEntity();
        Board::EntityInitializers::initPiece(e2, Board::Black, Board::Pawn);
        e2.setProperty("location", pointFromSquareIndex(index + 8));
    }; 

    for (size_t index = 48; index < 56; ++index) {
        auto & e = entities.addNewEntity();
        Board::EntityInitializers::initPiece(e, Board::White, Board::Pawn);
        e.setProperty("location", pointFromSquareIndex(index));

        auto & e2 = entities.addNewEntity();
        Board::EntityInitializers::initPiece(e2, Board::White, pieceRow[index - 48]);
        e2.setProperty("location", pointFromSquareIndex(index + 8));
    }; 

};
