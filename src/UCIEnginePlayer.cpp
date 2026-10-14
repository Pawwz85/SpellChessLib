#include <SpellChess/UCIEnginePlayer.h>
#include <SpellChess/SpellChessUciLoaderPort.h>
#include <SpellChess/Player.h>
#include <SpellChess/SpellChessGame.h>
#include <SpellChess/Board.h>
#include <UCILoader/EngineConnection.h>
#include <UCILoader/ProcessWrapper.h>
#include <UCILoader/UCI.h>
#include <stdexcept>
#include <vector>

SpellChessLib::Player::Result SpellChessLib::UCIEnginePlayer::waitForResult(std::chrono::milliseconds duration)
{
    if (duration.count() != 0)
        searchConnection->waitFor(duration);
    else
        searchConnection->waitFor(std::chrono::milliseconds::max());

    const auto status = searchConnection->getStatus();
    if (status != UCILoader::ResultReady)
    {
        switch (status)
        {
        case UCILoader::TimedOut:
            throw std::runtime_error("UCI engine search timed out.");
        case UCILoader::Terminated:
            throw std::runtime_error("UCI engine terminated before returning a result.");
        case UCILoader::Stopped:
            throw std::runtime_error("UCI engine search was stopped before returning a result.");
        default:
            throw std::runtime_error("UCI engine returned an unexpected search status.");
        }
    }

    const auto& res = searchConnection->getResult();
    if (res.bestMove == nullptr)
        throw std::runtime_error("UCI engine returned no best move.");

    return (res.ponderMove == nullptr) ? *res.bestMove : SpellChessLib::Player::Result(*res.bestMove, *res.ponderMove);
}

SpellChessLib::Player::Result SpellChessLib::UCIEnginePlayer::makeResult(const Game& game, const std::chrono::milliseconds& timeOut) {
    searchConnection = instance->search(game.getGoParams(), StateFormatter(game.getRoot()), game.getMovesSinceRoot());
    return waitForResult(timeOut);
}

SpellChessLib::Player::Result SpellChessLib::UCIEnginePlayer::handlePonderHit()
{
    searchConnection->ponderhit();
    return waitForResult(std::chrono::milliseconds(0));
}

void SpellChessLib::UCIEnginePlayer::startPondering(const Game& game, const SpellChessMove& ponderMove){
    auto params = game.getGoParams();
    params.Ponder = true;
    auto moves = game.getMovesSinceRoot();
    GameState state = game.getRoot();
    moves.push_back(ponderMove);
    searchConnection = instance->search(params, StateFormatter(game.getRoot()), moves);
}

void SpellChessLib::UCIEnginePlayer::cancelPondering(){
    if (searchConnection == nullptr)
        return;

    searchConnection->stop();
    searchConnection->waitFor(std::chrono::seconds(2));

    // Cancellation does not require a result. In particular, getResult() must
    // not be called for TimedOut, Terminated, or Stopped searches.
    (void)searchConnection->getStatus();
    searchConnection.reset();
}

bool SpellChessLib::UCIEnginePlayer::supportsPondering() const{
    return true;
}

void SpellChessLib::UCIEnginePlayer::interrupt(){
    if(searchConnection != nullptr)
        searchConnection->stop();
}

void SpellChessLib::UCIEnginePlayer::kill(){
    instance->quit();
}

void SpellChessLib::UCIEnginePlayer::onNewGame()
{
   instance->sync();
   instance->ucinewgame();
}

bool SpellChessLib::UCIEnginePlayer::isAlive()
{
    return instance->healthCheck();
}

std::unique_ptr<SpellChessLib::Player> SpellChessLib::UCIPlayerFactory::makeObject()
{
    auto process = UCILoader::openProcess({ pathToExecutable }, "/");
    std::shared_ptr<UCILoader::EngineInstance<SpellChessMove>> instance(SpellChessEngineInstanceBuilder->build(process));
    instance->sync();
    return std::make_unique<SpellChessLib::UCIEnginePlayer>(instance);
}
