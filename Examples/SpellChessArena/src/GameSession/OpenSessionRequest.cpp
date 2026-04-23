#include <GameSession/OpenSessionRequest.hpp>
#include <stdexcept>

std::shared_ptr<GameSession::PlayerDefinition> GameSession::defaultHumanPlayerDef =
    std::make_shared<GameSession::HumanPlayerDefinition>();

bool GameSession::HumanPlayerDefinition::isHuman() const{
    return true;
}
bool GameSession::HumanPlayerDefinition::isEngine() const{
    return false;
}

const QString & GameSession::HumanPlayerDefinition::getName() const{
    return "Human";
}

uint32_t GameSession::HumanPlayerDefinition::getPlayerId() const{
    return (uint32_t)-1;
}
GameSession::CPUPlayerDefinition::CPUPlayerDefinition(const SpellChessLib::PlayerEntry & engineEntry) :
    entry(engineEntry), name(engineEntry.name.c_str()) {
}

bool GameSession::CPUPlayerDefinition::isHuman() const{
    return false;
}

bool GameSession::CPUPlayerDefinition::isEngine() const{
    return true;
}


const QString & GameSession::CPUPlayerDefinition::getName() const{
    return name;
}

uint32_t GameSession::CPUPlayerDefinition::getPlayerId() const{
    return entry.id;
}

GameSession::OpenSessionRequest::OpenSessionRequest(SpellChessLib::GameRunnerConfiguration gameRunnerConfig, std::shared_ptr<PlayerDefinition> whitePlayer, std::shared_ptr<PlayerDefinition> blackPlayer) :
    gameRunnerConfig(gameRunnerConfig), 
    whitePlayer(whitePlayer),
    blackPlayer(blackPlayer) {
}

const GameSession::PlayerDefinition & GameSession::OpenSessionRequest::getWhitePlayer() const{
    return *whitePlayer;
}
const GameSession::PlayerDefinition & GameSession::OpenSessionRequest::getBlackPlayer() const{
    return *blackPlayer;
}

const SpellChessLib::GameRunnerConfiguration &GameSession::OpenSessionRequest::getGameRunnerConfig() const{
    return gameRunnerConfig;
}

GameSession::OpenSessionRequestBuilder::OpenSessionRequestBuilder() {
    whitePlayer = blackPlayer = defaultHumanPlayerDef;
    gameRunnerConfigBuilder = std::make_shared<SpellChessLib::GameRunnerConfigurationBuilder>();
}

GameSession::OpenSessionRequestBuilder &GameSession::OpenSessionRequestBuilder::withWhitePlayer(std::shared_ptr<PlayerDefinition> player){
    this->whitePlayer = player;
    return *this;
}

GameSession::OpenSessionRequestBuilder &GameSession::OpenSessionRequestBuilder::withBlackPlayer(std::shared_ptr<PlayerDefinition> player){
    this->blackPlayer = player;
    return *this;
}

std::shared_ptr<SpellChessLib::GameRunnerConfigurationBuilder> &GameSession::OpenSessionRequestBuilder::getGameRunnerConfigBuilder(){
    return this->gameRunnerConfigBuilder;
}

GameSession::OpenSessionRequest GameSession::OpenSessionRequestBuilder::build() const{
    return OpenSessionRequest(gameRunnerConfigBuilder->build(), whitePlayer, blackPlayer);
}
