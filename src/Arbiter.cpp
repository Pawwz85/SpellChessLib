#include <SpellChess/Arbiter.h>
#include <SpellChess/MoveGen.h>

SpellChessLib::ArbiterVerdict
SpellChessLib::arbite(const GameState & pos, const GameHistory & history) {

    if (pos.getHalfMoveCounter() >= 100)
        return ArbiterVerdict::DrawByFiftyMoveRule;

    if (pos.countPiece(Piece{King, White}) == 0 || pos.countPiece(Piece{King, Black}) == 0)
        return ArbiterVerdict::KingCapture;

    MoveGenerator gen(pos);

    if (gen.generateMoves().empty()) {
        return gen.isInCheck() ?
            ArbiterVerdict::Checkmate :
            ArbiterVerdict::Stalemate;
    }

    if (!pos.hasSufficientMatingMaterial(White) && !pos.hasSufficientMatingMaterial(Black))
        return ArbiterVerdict::DrawByInsufficientMaterial;

    if (history.countOccurences(pos) >= 3)
        return ArbiterVerdict::ThreefoldRepetition;

    return ArbiterVerdict::None;
};