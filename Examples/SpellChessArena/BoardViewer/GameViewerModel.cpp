#include <GameViewerModel.hpp>

#include <SpellChess/PlayerRegistry.h>
#include <SpellChess/MoveGen.h>
#include <SpellChess/OpeningBookPlayerWrapper.h>

#include <EngineTools/RunningEnginesService.hpp>
#include <EngineTools/EngineRolesService.hpp>

BoardViewer::GameViewerModel::GameViewerModel(std::shared_ptr<SpellChessLib::TimerService> timeServ, const OpenSessionRequest &request, QCoreApplication *app) : runnerAdapter(app, this),
                                                                                                                                                                 application(app),
                                                                                                                                                                 whitePlayerName(request.getWhitePlayer().getName()),
                                                                                                                                                                 blackPlayerName(request.getBlackPlayer().getName())
{
    using PlayerPointer = typename SpellChessLib::Scheduler::ResourcePointer<SpellChessLib::Player>;
        
    auto & engineRolesService =     EngineTools::EngineRolesService::getInstance(); 
    auto & runningEngineService = RunningEnginesService::getInstance(); 

    PlayerPointer whitePlayer = playerFromDef(request.getWhitePlayer(), request);

    if (request.getWhitePlayer().isHuman()) {
        players[SpellChessLib::White] = (SpellChessLib::AsynchronousPlayer*)whitePlayer.get();
        isPlayerHuman[SpellChessLib::White] = true;
    } else {
        auto entry = runningEngineService.getLastest();
        if (entry.has_value())
            engineRolesService.assignEngineToRole(EngineTools::WhiteSideEngine, entry.value());
    }

    PlayerPointer blackPlayer = playerFromDef(request.getBlackPlayer(), request);
    if (request.getBlackPlayer().isHuman()) {
        players[SpellChessLib::Black] = (SpellChessLib::AsynchronousPlayer*)blackPlayer.get();
        isPlayerHuman[SpellChessLib::Black] = true;
    } else {
        auto entry = runningEngineService.getLastest();
        if (entry.has_value())
            engineRolesService.assignEngineToRole(EngineTools::BlackSideEngine, entry.value());
    }

    gameRunner.reset(new SpellChessLib::GameRunner(timeServ, request.getGameRunnerConfig(),
     std::move(whitePlayer), std::move(blackPlayer)));
    

    gameRunner->attachGameMonitor(runnerAdapter.getGameMonitor());
    
    document = gameRunner->getGame().getDocument();
    currentPosition = document->getRoot();

    gameThread = new std::thread([this](){
        this->gameRunner->run();
    });
};

BoardViewer::GameViewerModel::~GameViewerModel() {
    if(gameRunner->isRunning())
        gameRunner->stop();
    gameThread->join();
    delete gameThread;
}

SpellChessLib::SpellChessDocument::Handle BoardViewer::GameViewerModel::getCurrentPosition() const {
    return currentPosition;
}

std::shared_ptr<const SpellChessLib::SpellChessDocument> BoardViewer::GameViewerModel::getDocument() const {
    return document;
};

bool BoardViewer::GameViewerModel::event(QEvent *e){
    if(e->type() == gameSessionEvents.GameStateChanged) {
        if(document->clearDirtyFlag()) {
            currentPosition = document->getTail();
            documentChanged(*document);
        }
        changed(*this);
        return true;
    } else if (e->type() == gameSessionEvents.MovesSinceRootChanged) {
       // Noop
       return true;
    } else if (e->type() == gameSessionEvents.GameRunnerIdChanged) {
        auto event = (Internal::GameRunnerEvent *)e;
        gameRunnerId = event->getGameRunnerId();
        return true;
    } else if (e->type() == gameSessionEvents.TimerPairStateChanged) {
        auto event = (Internal::TimerPairEvent *) e;
        timers = event->state;
        onTimerChanged(timers);
        return true;
    }

    return QObject::event(e);
}

void BoardViewer::GameViewerModel::handleMakeMove(SpellChessLib::SpellChessMove move)
{
    if (isCurrentStateTail()) 
        dispatchMoveToGameRunner(move);
    else
        navigateDocument(move);
}

bool BoardViewer::GameViewerModel::isCurrentStateTail() const{
    return currentPosition.lock() == document->getTail().lock();
}

void BoardViewer::GameViewerModel::dispatchMoveToGameRunner(const SpellChessLib::SpellChessMove &move){
    SpellChessLib::AsynchronousPlayer * player = players[getCurrentState().getSideToMove()];

    if (player) {
        try {
            player->resolveCurrentMovePromise(move);
        } catch(...) {};
    }

}

void BoardViewer::GameViewerModel::navigateDocument(const SpellChessLib::SpellChessMove &move){
    currentPosition = document->makeMove(currentPosition, move);

    if(document->clearDirtyFlag())
        documentChanged(*document);

    changed(*this);
}

SpellChessLib::Scheduler::ResourcePointer<SpellChessLib::Player> makeCPUPlayer(const BoardViewer::PlayerDefinition &def, const BoardViewer::OpenSessionRequest & request) {
    using PlayerAlloc = typename SpellChessLib::Scheduler::DefaultObjectAllocator<SpellChessLib::Player>;

    const auto & playerEntry = SpellChessLib::PlayerRegistry->get(def.getPlayerId());
    auto playerAllocator = std::make_shared<PlayerAlloc>(playerEntry.instanceFactory);
    if (request.openingBook == nullptr)
        return playerAllocator->retrieve(); 
    else {
        auto wrapperAlloc = std::make_shared<SpellChessLib::PlayerWithOpeningBookAllocator>(playerAllocator, request.openingBook);
        return wrapperAlloc->retrieve();
    }
};


SpellChessLib::Scheduler::ResourcePointer<SpellChessLib::Player> makeHumanPlayer(const BoardViewer::PlayerDefinition &def, const BoardViewer::OpenSessionRequest & request) {
    using PlayerFactory = typename SpellChessLib::Factory<SpellChessLib::Player>;
    using AsyncPlayerFactory = typename SpellChessLib::DefaultFactory<SpellChessLib::AsynchronousPlayer>;
    static auto factory = std::make_shared<AsyncPlayerFactory>();
    return SpellChessLib::Scheduler::makeResource(std::reinterpret_pointer_cast<PlayerFactory>(factory));
};

BoardViewer::GameViewerModel::PlayerPointer BoardViewer::GameViewerModel::playerFromDef(const PlayerDefinition &def, const OpenSessionRequest &request){
    return def.isHuman() ? makeHumanPlayer(def, request) : makeCPUPlayer(def, request);
}

bool BoardViewer::GameViewerModel::canUserSubmitMove() {
    SpellChessLib::PlayerColor sideToMove = getCurrentState().getSideToMove();
    return !isCurrentStateTail() || isPlayerHuman[sideToMove];
};

QString BoardViewer::GameViewerModel::getWhitePlayerName() const {
    return whitePlayerName;
}

QString BoardViewer::GameViewerModel::getBlackPlayerName() const {
    return blackPlayerName;
}

