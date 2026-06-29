#include <SpellChess/AsynchronousPlayer.h>
#include <stdexcept>



SpellChessLib::AsynchronousPlayer::Result
SpellChessLib::AsynchronousPlayer::makeResult(const Game& game, const std::chrono::milliseconds & timeOut) {

    if (currentMovePromise.has_value())
        throw std::logic_error("AsynchronousPlayer::makeResult called while previous move is still pending.");

    currentGameState = game.getGameState();
    currentMovePromise.emplace();
    auto future = currentMovePromise->get_future();

    SpellChessMove bestMove;
    if (future.wait_for(timeOut) == std::future_status::ready) 
        bestMove = future.get();
    else
        bestMove = SpellChessMove{ NoSquare, NoSquare, NoSquare, NoSquare, None };
    
    currentMovePromise.reset();
    return Result(bestMove);
}

SpellChessLib::AsynchronousPlayer::Result
SpellChessLib::AsynchronousPlayer::handlePonderHit() {
    throw std::logic_error("AsynchronousPlayer does not support pondering.");
};

void
SpellChessLib::AsynchronousPlayer::startPondering(const Game& game, const SpellChessMove & ponderMove) {
    throw std::logic_error("AsynchronousPlayer does not support pondering.");
};

void
SpellChessLib::AsynchronousPlayer::cancelPondering() {
    // noop, because we are not supporting pondering anyway
};

bool
SpellChessLib::AsynchronousPlayer::supportsPondering() const {
    return false;
};

void
SpellChessLib::AsynchronousPlayer::onNewGame() {
    currentMovePromise.reset();
    wasInterrupted = false;
};

void
SpellChessLib::AsynchronousPlayer::interrupt() {
    wasInterrupted = true;
    if (currentMovePromise.has_value()) {
        currentMovePromise->set_exception(std::make_exception_ptr(std::runtime_error("AsynchronousPlayer interrupted.")));
    }
};

void
SpellChessLib::AsynchronousPlayer::kill() {
    // Note: this code shouldn't be reachable, so if you ever encounter this exception, there is a bug in the game runner logic. The game runner should always call interrupt() before calling kill().
    if (currentMovePromise.has_value()) {
        currentMovePromise->set_exception(std::make_exception_ptr(std::runtime_error("AsynchronousPlayer killed.")));
    }
};

void
SpellChessLib::AsynchronousPlayer::resolveCurrentMovePromise(const SpellChessMove& move) {
    MoveGenerator gen(currentGameState);

    if(!gen.isMoveLegal(move)){
        throw std::invalid_argument("AsynchronousPlayer::resolveCurrentMovePromise called with illegal move.");
    };

    if(wasInterrupted){
        // If the player was interrupted, we don't want to resolve the promise, as the game runner has already moved on. We just ignore the move and return.
        return;
    }

    if (currentMovePromise.has_value()) {
        currentMovePromise->set_value(move);
    } else if (!wasInterrupted) 
        throw std::logic_error("AsynchronousPlayer::resolveCurrentMovePromise called while no move is pending.");
}