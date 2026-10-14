#include <SpellChess/GameRunnerConfiguration.h>
#include <SpellChess/Board.h>
#include <SpellChess/TimerService.h>

SpellChessLib::GameRunnerConfigurationBuilder& SpellChessLib::GameRunnerConfigurationBuilder::withPosition(const std::string& fen, const std::vector<SpellChessMove>& moves){
    config.rootFen = fen;
    config.rootMoves = moves;
    return *this;
}

SpellChessLib::GameRunnerConfigurationBuilder& SpellChessLib::GameRunnerConfigurationBuilder::withPondering(bool allowPondering){
    config.allowPondering = allowPondering;
    return *this;
}

SpellChessLib::GameRunnerConfigurationBuilder& SpellChessLib::GameRunnerConfigurationBuilder::withInfiniteTime(GameRunnerConfigurationBuilder::PlayerColor color){
    if (color != GameRunnerConfigurationBuilder::Black) {
        config.whPlayerTimerInfo.type = GameRunnerConfiguration::INFINITE;
        config.whPlayerTimerInfo.data.infiniteTimerData = nullptr;
    }
    
    if (color != GameRunnerConfigurationBuilder::White) {
        config.blPlayerTimerInfo.type = GameRunnerConfiguration::INFINITE;
        config.blPlayerTimerInfo.data.infiniteTimerData = nullptr;
    }

    return *this;
}

SpellChessLib::GameRunnerConfigurationBuilder& SpellChessLib::GameRunnerConfigurationBuilder::withClassicalTimer(const uint32_t& base, const uint32_t& inc, const uint32_t & margin, GameRunnerConfigurationBuilder::PlayerColor color)
{
    if (color != GameRunnerConfigurationBuilder::Black) {
        config.whPlayerTimerInfo.type = GameRunnerConfiguration::CLASSICAL;
        config.whPlayerTimerInfo.data.classicalTimerData.base = base;
        config.whPlayerTimerInfo.data.classicalTimerData.inc = inc;
        config.whPlayerTimerInfo.data.classicalTimerData.margin = margin;
    }
    if (color != GameRunnerConfigurationBuilder::White) {
        config.blPlayerTimerInfo.type = GameRunnerConfiguration::CLASSICAL;
        config.blPlayerTimerInfo.data.classicalTimerData.base = base;
        config.blPlayerTimerInfo.data.classicalTimerData.inc = inc;
        config.blPlayerTimerInfo.data.classicalTimerData.margin = margin;
    }
    return *this;
}

SpellChessLib::GameRunnerConfigurationBuilder& SpellChessLib::GameRunnerConfigurationBuilder::withNodeLimit(const uint32_t& value){
    config.stopConditions.nodes = value;
    return *this;
}

SpellChessLib::GameRunnerConfigurationBuilder& SpellChessLib::GameRunnerConfigurationBuilder::withDepthLimit(const uint32_t& value) {
    config.stopConditions.depth = value;
    return *this;
}

SpellChessLib::GameRunnerConfigurationBuilder& SpellChessLib::GameRunnerConfigurationBuilder::withMoveTimeLimit(const uint32_t& value) {
    config.stopConditions.moveTime = value;
    return *this;
}

SpellChessLib::GameRunnerConfigurationBuilder& SpellChessLib::GameRunnerConfigurationBuilder::withEvent(const std::string& value){
    config.event = value;
    return *this;
}

SpellChessLib::GameRunnerConfigurationBuilder& SpellChessLib::GameRunnerConfigurationBuilder::withSite(const std::string& value) {
    config.site = value;
    return *this;
}

SpellChessLib::GameRunnerConfigurationBuilder& SpellChessLib::GameRunnerConfigurationBuilder::withRound(const std::string& value) {
    config.round = value;
    return *this;
}

std::unique_ptr<SpellChessLib::TimerController> SpellChessLib::GameRunnerConfiguration::createTimerFromInfo(std::shared_ptr<TimerService> timerService, const TimerInfo& info){
    TimerControllerFactory factory(timerService);
  
    switch (info.type)
    {
    case INFINITE:
        return factory.makeInfiniteTimerController();
    case CLASSICAL:
        return factory.makeClassicTimerController(info.data.classicalTimerData.base, info.data.classicalTimerData.inc, info.data.classicalTimerData.margin);
    default:
        return nullptr;
    }
}
