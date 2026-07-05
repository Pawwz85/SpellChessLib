#include <OpenSessionRequest.hpp>
#include <QString>
#include <stdexcept>

std::shared_ptr<BoardViewer::PlayerDefinition> BoardViewer::defaultHumanPlayerDef =
    std::make_shared<BoardViewer::HumanPlayerDefinition>();

bool BoardViewer::HumanPlayerDefinition::isHuman() const{
    return true;
}
bool BoardViewer::HumanPlayerDefinition::isEngine() const{
    return false;
}

const QString & BoardViewer::HumanPlayerDefinition::getName() const{
    static QString resultString = "Human";
    return resultString;
}

uint32_t BoardViewer::HumanPlayerDefinition::getPlayerId() const{
    return (uint32_t)-1;
}
BoardViewer::CPUPlayerDefinition::CPUPlayerDefinition(const SpellChessLib::PlayerEntry & engineEntry) :
    entry(engineEntry), name(engineEntry.name.c_str()) {
}

bool BoardViewer::CPUPlayerDefinition::isHuman() const{
    return false;
}

bool BoardViewer::CPUPlayerDefinition::isEngine() const{
    return true;
}


const QString & BoardViewer::CPUPlayerDefinition::getName() const{
    return name;
}

uint32_t BoardViewer::CPUPlayerDefinition::getPlayerId() const{
    return entry.id;
}

BoardViewer::OpenSessionRequest::OpenSessionRequest(SpellChessLib::GameRunnerConfiguration gameRunnerConfig, std::shared_ptr<PlayerDefinition> whitePlayer, std::shared_ptr<PlayerDefinition> blackPlayer, std::shared_ptr<SpellChessLib::OpeningBook> book) :
    gameRunnerConfig(gameRunnerConfig), 
    whitePlayer(whitePlayer),
    blackPlayer(blackPlayer),
    openingBook(book) {
}

const BoardViewer::PlayerDefinition & BoardViewer::OpenSessionRequest::getWhitePlayer() const{
    return *whitePlayer;
}
const BoardViewer::PlayerDefinition & BoardViewer::OpenSessionRequest::getBlackPlayer() const{
    return *blackPlayer;
}

const SpellChessLib::GameRunnerConfiguration &BoardViewer::OpenSessionRequest::getGameRunnerConfig() const{
    return gameRunnerConfig;
}

BoardViewer::OpenSessionRequestBuilder::OpenSessionRequestBuilder() {
    whitePlayer = blackPlayer = defaultHumanPlayerDef;
    gameRunnerConfigBuilder = std::make_shared<SpellChessLib::GameRunnerConfigurationBuilder>();
}

BoardViewer::OpenSessionRequestBuilder &BoardViewer::OpenSessionRequestBuilder::withWhitePlayer(std::shared_ptr<PlayerDefinition> player){
    this->whitePlayer = player;
    return *this;
}

BoardViewer::OpenSessionRequestBuilder &BoardViewer::OpenSessionRequestBuilder::withBlackPlayer(std::shared_ptr<PlayerDefinition> player){
    this->blackPlayer = player;
    return *this;
}

BoardViewer::OpenSessionRequestBuilder &BoardViewer::OpenSessionRequestBuilder::withOpeningBook(std::shared_ptr<SpellChessLib::OpeningBook> book){
    this->openingBook = book;
    return *this;
}

std::shared_ptr<SpellChessLib::GameRunnerConfigurationBuilder> &BoardViewer::OpenSessionRequestBuilder::getGameRunnerConfigBuilder(){
    return this->gameRunnerConfigBuilder;
}

BoardViewer::OpenSessionRequest BoardViewer::OpenSessionRequestBuilder::build() const{
    return OpenSessionRequest(gameRunnerConfigBuilder->build(), whitePlayer, blackPlayer, openingBook);
}
