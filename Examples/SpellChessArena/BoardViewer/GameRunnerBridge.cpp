#include <GameRunnerBridge.hpp>
#include <stdexcept>


BoardViewer::Internal::GameRunnerEvents::GameRunnerEvents(): 
    GameStateChanged(QEvent::registerEventType()),
    MovesSinceRootChanged(QEvent::registerEventType()),
    GameRunnerIdChanged(QEvent::registerEventType()),
    TimerPairStateChanged(QEvent::registerEventType())
    {};

BoardViewer::Internal::GameRunnerEvents BoardViewer::gameSessionEvents;

BoardViewer::Internal::GameStateChangedEvent::GameStateChangedEvent(const SpellChessLib::GameState *newState) : 
    QEvent((QEvent::Type)gameSessionEvents.GameStateChanged){
    if(newState == nullptr)
        state = nullptr;
    else 
        state.reset(new SpellChessLib::GameState(*newState));
}

bool BoardViewer::Internal::GameStateChangedEvent::isEmpty() const {
    return state == nullptr;
}

const SpellChessLib::GameState* BoardViewer::Internal::GameStateChangedEvent::get() const {
    return state.get();
}

const SpellChessLib::GameState& BoardViewer::Internal::GameStateChangedEvent::operator*() const{
    return *state;
}

BoardViewer::Internal::MoveListEvent::MoveListEvent(const std::vector<SpellChessLib::SpellChessMove> &moves) :
    QEvent((QEvent::Type)gameSessionEvents.MovesSinceRootChanged),
    moves(moves) {}

const std::vector<SpellChessLib::SpellChessMove> &BoardViewer::Internal::MoveListEvent::movesSinceRoot() const {
    return moves;
};

BoardViewer::Internal::TimerPairEvent::TimerPairEvent(const SpellChessLib::TimerPairState & state) :
    QEvent((QEvent::Type)gameSessionEvents.TimerPairStateChanged),
    state(state) {};

BoardViewer::Internal::GameRunnerEvent::GameRunnerEvent(int id) : 
    QEvent((QEvent::Type)gameSessionEvents.GameRunnerIdChanged) , gameRunnerId(id) {}

int BoardViewer::Internal::GameRunnerEvent::getGameRunnerId() const {
    return gameRunnerId;
}

bool BoardViewer::Internal::GameRunnerEvent::connected() const{
    return gameRunnerId > 0;
};

#define EVENT_EMITTER_INIT(EVENT, STATE) emitter##EVENT (std::make_shared<Internal::EventEmmiter<Internal::EVENT, STATE>> (app, owner))

BoardViewer::GameRunnerBridge::GameRunnerBridge(QCoreApplication *app, QObject *owner) : gameMonitor(std::make_shared<SpellChessLib::GameMonitor>()),
                                                                               EVENT_EMITTER_INIT(GameStateChangedEvent, SpellChessLib::GameState *),
                                                                               EVENT_EMITTER_INIT(MoveListEvent, std::vector<SpellChessLib::SpellChessMove>),
                                                                               EVENT_EMITTER_INIT(GameRunnerEvent, int),
                                                                               EVENT_EMITTER_INIT(TimerPairEvent, SpellChessLib::TimerPairState)
{
    gameMonitor->getObservableGame().addObserver(emitterGameStateChangedEvent);
    gameMonitor->getObservableMoves().addObserver(emitterMoveListEvent);
    gameMonitor->getTimerPairObserver().addObserver(emitterTimerPairEvent);
    gameMonitor->getGameRunnerIdObserver().addObserver(emitterGameRunnerEvent);
}

#undef EVENT_EMITTER_INIT


std::shared_ptr<SpellChessLib::GameMonitor> BoardViewer::GameRunnerBridge::getGameMonitor(){
    return gameMonitor;
}

