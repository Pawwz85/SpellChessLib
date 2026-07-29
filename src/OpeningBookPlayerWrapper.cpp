#include <SpellChess/OpeningBookPlayerWrapper.h>

SpellChessLib::PlayerWithOpeningBook::PlayerWithOpeningBook(Scheduler::ResourcePointer<Player> && wraped, std::shared_ptr<OpeningBook> book)
    : book(book), wrapped(std::move(wraped))
{
}

SpellChessLib::Player::Result SpellChessLib::PlayerWithOpeningBook::makeResult(const Game& game, const std::chrono::milliseconds & timeOut) {
    return Result({}); // return invalid move, as this player does not generate moves on its own
};

SpellChessLib::Player::Result SpellChessLib::PlayerWithOpeningBook::handlePonderHit() {
    return Result({}); // return invalid move, as this player does not generate moves on its own
};

bool SpellChessLib::PlayerWithOpeningBook::supportsPondering() const {
    return wrapped->supportsPondering();
};

void SpellChessLib::PlayerWithOpeningBook::onNewGame() {
    wrapped->onNewGame();
}

void SpellChessLib::PlayerWithOpeningBook::interrupt() {
    wrapped->interrupt();
};

void SpellChessLib::PlayerWithOpeningBook::kill() {
    wrapped->kill();
}

SpellChessLib::Player::Result SpellChessLib::PlayerWithOpeningBook::search(const Game& game) {
    auto move = book->pickRandomMove(game.getGameState());

    if (move.has_value())
        return Result(move.value());

    return wrapped->search(game);
};

void SpellChessLib::PlayerWithOpeningBook::ponder(const Game& game, const SpellChessMove & ponderMove){
    wrapped->ponder(game, ponderMove);
}

void SpellChessLib::PlayerWithOpeningBookAllocator::release(SpellChessLib::Player* o){
    delete o;
}

void SpellChessLib::PlayerWithOpeningBookAllocator::destroy(SpellChessLib::Player* o) {
    delete o;
};

SpellChessLib::Scheduler::ResourcePointer<SpellChessLib::Player> SpellChessLib::PlayerWithOpeningBookAllocator::retrieve() {
    return Scheduler::ResourcePointer<SpellChessLib::Player>(
        new PlayerWithOpeningBook(wrappedPlayerAllocator->retrieve(), book),
        shared_from_this()
    );
};

SpellChessLib::PlayerWithOpeningBookAllocator::PlayerWithOpeningBookAllocator(std::shared_ptr<Scheduler::ObjectAllocator<Player>> alloc, std::shared_ptr<OpeningBook> book) :
    wrappedPlayerAllocator(alloc), book(book){
}
