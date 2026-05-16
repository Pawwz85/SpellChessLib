#include <GameSession/GameRunnerBridge.hpp>
#include <stdexcept>


GameSession::Internal::GameRunnerEvents::GameRunnerEvents(): 
    GameStateChanged(QEvent::registerEventType()),
    MovesSinceRootChanged(QEvent::registerEventType()),
    GameRunnerIdChanged(QEvent::registerEventType()),
    TimerPairStateChanged(QEvent::registerEventType())
    {};

GameSession::Internal::GameRunnerEvents GameSession::gameSessionEvents;

GameSession::Internal::GameStateChangedEvent::GameStateChangedEvent(const SpellChessLib::GameState *newState) : 
    QEvent((QEvent::Type)gameSessionEvents.GameStateChanged){
    if(newState == nullptr)
        state = nullptr;
    else 
        state.reset(new SpellChessLib::GameState(*newState));
}

bool GameSession::Internal::GameStateChangedEvent::isEmpty() const {
    return state == nullptr;
}

const SpellChessLib::GameState *GameSession::Internal::GameStateChangedEvent::get() const {
    return state.get();
}

const SpellChessLib::GameState &GameSession::Internal::GameStateChangedEvent::operator*() const{
    return *state;
}

GameSession::Internal::MoveListEvent::MoveListEvent(const std::vector<SpellChessLib::SpellChessMove> &moves) :
    QEvent((QEvent::Type)gameSessionEvents.MovesSinceRootChanged),
    moves(moves) {}

const std::vector<SpellChessLib::SpellChessMove> &GameSession::Internal::MoveListEvent::movesSinceRoot() const {
    return moves;
};

GameSession::Internal::TimerPairEvent::TimerPairEvent(const SpellChessLib::TimerPairState & state) :
    QEvent((QEvent::Type)gameSessionEvents.TimerPairStateChanged),
    state(state) {};

GameSession::Internal::GameRunnerEvent::GameRunnerEvent(int id) : 
    QEvent((QEvent::Type)gameSessionEvents.GameRunnerIdChanged) , gameRunnerId(id) {}

int GameSession::Internal::GameRunnerEvent::getGameRunnerId() const {
    return gameRunnerId;
}

bool GameSession::Internal::GameRunnerEvent::connected() const{
    return gameRunnerId > 0;
};

#define EVENT_EMITTER_INIT(EVENT, STATE) emitter##EVENT (std::make_shared<Internal::EventEmmiter<Internal::EVENT, STATE>> (app, owner))

class GameSessionPlayerFactory : public SpellChessLib::Factory<SpellChessLib::Player> {
    GameSession::GameRunnerBridge * session;
    public:
    GameSessionPlayerFactory(GameSession::GameRunnerBridge * session) : session(session) {};
    std::unique_ptr<SpellChessLib::Player> makeObject() override {
       return std::make_unique<GameSession::GuiPlayer>(session);
    }
};


GameSession::GameRunnerBridge::GameRunnerBridge(QCoreApplication *app, QObject *owner) : gameMonitor(std::make_shared<SpellChessLib::GameMonitor>()),
                                                                               EVENT_EMITTER_INIT(GameStateChangedEvent, SpellChessLib::GameState *),
                                                                               EVENT_EMITTER_INIT(MoveListEvent, std::vector<SpellChessLib::SpellChessMove>),
                                                                               EVENT_EMITTER_INIT(GameRunnerEvent, int),
                                                                               EVENT_EMITTER_INIT(TimerPairEvent, SpellChessLib::TimerPairState)
{
    gameMonitor->getObservableGame().addObserver(emitterGameStateChangedEvent);
    gameMonitor->getObservableMoves().addObserver(emitterMoveListEvent);
    gameMonitor->getTimerPairObserver().addObserver(emitterTimerPairEvent);
    gameMonitor->getGameRunnerIdObserver().addObserver(emitterGameRunnerEvent);

    using PlayerFactory = typename SpellChessLib::Factory<SpellChessLib::Player>; 
    playerFactory =   std::make_shared<GameSessionPlayerFactory>(this);
}

#undef EVENT_EMITTER_INIT


std::shared_ptr<SpellChessLib::GameMonitor> GameSession::GameRunnerBridge::getGameMonitor(){
    return gameMonitor;
}

SpellChessLib::Scheduler::ResourcePointer<SpellChessLib::Player> GameSession::GameRunnerBridge::makePlayerFromSession(){
    using PlayerAllocator = typename SpellChessLib::Scheduler::DefaultObjectAllocator<SpellChessLib::Player>;
    
    auto playerAllocator = std::make_shared<PlayerAllocator>(playerFactory);
    return  playerAllocator->retrieve();
}

void GameSession::GameRunnerBridge::submitMove(SpellChessLib::SpellChessMove move){
    if(movePromise != nullptr) {
        movePromise->set_value(move);
        movePromise.reset();
    }
}

SpellChessLib::Player::Result GameSession::GuiPlayer::makeResult(const SpellChessLib::Game &game, const std::chrono::milliseconds &timeOut){
    session->movePromise.reset(new std::promise<SpellChessLib::SpellChessMove>);
    auto future = session->movePromise->get_future();
    
    if (timeOut == std::chrono::milliseconds(0)) {
        return future.get();
    };

    auto status = future.wait_for(timeOut);
    if(status == std::future_status::ready)
        return Result(future.get());
    else 
        return Result({});
}

SpellChessLib::Player::Result GameSession::GuiPlayer::handlePonderHit()
{
    return SpellChessLib::Player::Result({}); // this functions will never be called as pondering is disabled
}

void GameSession::GuiPlayer::startPondering(const SpellChessLib::Game &game, const SpellChessLib::SpellChessMove &ponderMove){
    // noop
}

void GameSession::GuiPlayer::cancelPondering(){
    // noop
}

bool GameSession::GuiPlayer::supportsPondering() const {
    return false;
}

void GameSession::GuiPlayer::onNewGame(){
    // noop
}


void GameSession::GuiPlayer::interrupt(){
    try {
         if (session->movePromise != nullptr)
            session->movePromise->set_exception(std::make_exception_ptr(std::exception("interrupted")));
    } catch(...) {
    }; 
};

void GameSession::GuiPlayer::kill(){
    // noop, we have no reasonable way to kill user session
}

GameSession::GuiPlayer::GuiPlayer(GameRunnerBridge *session) : 
    session(session){
}
