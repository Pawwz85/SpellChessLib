#include <SpellChess/Player.h>

bool SpellChessLib::Player::ponderhit(const SpellChessMove& move) const {
    if (ponderMove == nullptr) return false;
    return memcmp(&move, ponderMove.get(), sizeof(SpellChessMove)) == 0;
}

void SpellChessLib::Player::setPonderMove(const SpellChessMove& move){
    ponderMove.reset(new SpellChessMove(move));
}

SpellChessLib::Player::Result SpellChessLib::Player::search(const Game& game)
{
    auto movesSinceRoot = game.getMovesSinceRoot();
    
    if (isPondering()) {
        if (movesSinceRoot.empty() || !ponderhit(movesSinceRoot.back())) {
            clearPonderMove();
            cancelPondering();
        }
        else
           return handlePonderHit();
    }

    return makeResult(game, game.getRemainingTime(game.getGameState().getSideToMove()));
}

void SpellChessLib::Player::ponder(const Game& game, const SpellChessMove& ponderMove){
    if (!supportsPondering())
        throw std::exception("Pondering not supported");

    if(isPondering())
        throw std::exception("Already pondering");
    
    setPonderMove(ponderMove);
    startPondering(game, ponderMove);
}
