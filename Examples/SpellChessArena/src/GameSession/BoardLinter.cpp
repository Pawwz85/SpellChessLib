#include <GameSession/BoardLinter.hpp>
#include <SpellChess/MoveGen.h>

//! checks whatever linter input satisfies spell requirements of given move candidate
bool isMoveValidGivenLinterInput(const GameSession::LinterInput & input, const SpellChessLib::MoveCandidate & move) {

    if (move.spellPolicy == SpellChessLib::UserCanAddSpell)
        return true;

    if(move.spellPolicy == SpellChessLib::SpellGeneratorEnforced) {
        return move.move.freezeSquare == input.previewedFreezeSquare && move.move.jumpSquare == input.previewedJumpableSquare;
    };

    return false;
}

uint64_t GameSession::Internal::getMovablePieces(const LinterInput & input) {

    if (input.playerColor != input.state.getSideToMove()    ||
        input.playerColor == SpellChessLib::Nobody)
        return 0ull;

    SpellChessLib::MoveGenerator gen(input.state);
    
    uint64_t result = 0ull;
    auto moves = gen.generateMoves();
    for(const auto & move: moves)
        if(isMoveValidGivenLinterInput(input, move))
            result |= 1ull << (move.move.from);
    return result;
};

uint64_t GameSession::Internal::getValidTargetSquares(const LinterInput & input) {

    if (input.playerColor != input.state.getSideToMove()    ||
        input.playerColor == SpellChessLib::Nobody)
        return 0ull;

    SpellChessLib::MoveGenerator gen(input.state);
    
    uint64_t result = 0ull;
    auto moves = gen.generateMoves();
    for(const auto & move: moves)
        if( isMoveValidGivenLinterInput(input, move) && move.move.from == input.selectedSquare)
            result |= 1ull << (move.move.to);
    return result;
};

uint64_t GameSession::Internal::getJumpableSquares(const LinterInput & input) {
    uint64_t result = 0ull;

    if (input.state.getJumpSquare() != SpellChessLib::NoSquare)
        result |= 1ull << input.state.getJumpSquare();

    if (input.previewedJumpableSquare != SpellChessLib::NoSquare)
        result != 1ull << input.previewedJumpableSquare;

    return result;
};

uint64_t freezeArea(SpellChessLib::SquareIndex freezeCenter) {

    if (freezeCenter == SpellChessLib::NoSquare)   
        return 0ull;

    uint64_t result = 1ull << freezeCenter;
    result |= (result << 1) | (result >> 1);
    result |= (result << 8) | (result << 8);
    return result;
};

uint64_t  GameSession::Internal::getFrozenSquares(const LinterInput & input) {
    uint64_t result = 0ull;
    result |= freezeArea(input.state.getFreezeSquare());
    result |= freezeArea(input.previewedFreezeSquare);
    return result;
};

uint64_t GameSession::Internal::getInteractiveSquares(const LinterInput & input) {
    return  input.selectedSquare == SpellChessLib::NoSquare ? 
            getMovablePieces(input) :
            getValidTargetSquares(input);
};


 void GameSession::lintBoard(Board::BoardGridModel & model, const LinterInput & input) {
    uint64_t interactable = Internal::getInteractiveSquares(input);
    model.startEdit()
    .setPropertyBitmask(Board::Frozen,          Internal::getFrozenSquares(input))
    .setPropertyBitmask(Board::Jumpable,        Internal::getJumpableSquares(input))
    .setPropertyBitmask(Board::Interactable,    interactable)
    .setPropertyBitmask(Board::HasMoveDot,      input.selectedSquare != SpellChessLib::NoSquare ? interactable : 0ull)
    .commit();
 };

 uint64_t GameSession::getDraggablePieces(const LinterInput & input) {
    return input.selectedSquare != SpellChessLib::NoSquare ? 
        0ull :
        Internal::getInteractiveSquares(input);
 };