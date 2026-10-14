#include <SpellChess/RandomPlayer.h>
#include <SpellChess/MoveGen.h>
#include <SpellChess/Board.h>
#include <SpellChess/Player.h>
#include <SpellChess/SpellChessGame.h>
#include <random>
#include <cstddef>
#include <memory>
#include <vector>

SpellChessLib::Player::Result SpellChessLib::RandomPlayer::makeResult(const Game& game, const std::chrono::milliseconds& timeOut)
{
    auto s = game.getGameState();
    MoveGenerator moveGen(s);
    auto moves = moveGen.generateMoves();
    size_t random_index = std::uniform_int_distribution<size_t>(0, moves.size() - 1)(dev);
    return moves[random_index].move;
}

SpellChessLib::Player::Result SpellChessLib::RandomPlayer::handlePonderHit() {
    return SpellChessLib::Player::Result({});
}

void SpellChessLib::RandomPlayer::startPondering(const Game& game, const SpellChessMove& ponderMove)
{}

void SpellChessLib::RandomPlayer::cancelPondering()
{}

bool SpellChessLib::RandomPlayer::supportsPondering() const{
    return false;
}

void SpellChessLib::RandomPlayer::interrupt()
{}

void SpellChessLib::RandomPlayer::kill()
{}

void SpellChessLib::RandomPlayer::onNewGame()
{}

std::unique_ptr<SpellChessLib::Player> SpellChessLib::RandomPlayer::FactoryClass::makeObject()
{
    return std::make_unique<SpellChessLib::RandomPlayer>();
}
