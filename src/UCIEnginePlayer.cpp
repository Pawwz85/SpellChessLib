#include <SpellChess/UCIEnginePlayer.h>
#include <SpellChess/SpellChessUciLoaderPort.h>
#include <UCILoader/Logger.h>

SpellChessLib::Player::Result SpellChessLib::UCIEnginePlayer::waitForResult()
{
    searchConnection->waitFor(std::chrono::seconds(10));
    auto res = searchConnection->getResult();
    return (res.ponderMove == nullptr) ? *res.bestMove : SpellChessLib::Player::Result(*res.bestMove, *res.ponderMove);
}

SpellChessLib::Player::Result SpellChessLib::UCIEnginePlayer::makeResult(const Game& game, const std::chrono::milliseconds& timeOut) {
    searchConnection = instance->search(game.getGoParams(), StateFormatter(game.getRoot()), game.getMovesSinceRoot());
    return waitForResult();
}

SpellChessLib::Player::Result SpellChessLib::UCIEnginePlayer::handlePonderHit()
{
    searchConnection->ponderhit();
    return waitForResult();
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
    interrupt();
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
