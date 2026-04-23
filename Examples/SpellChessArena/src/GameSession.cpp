#include <GameSession.hpp>
#include <BoardGrid/EntityInitializers.hpp>
#include <map>

#include <SpellChess/PlayerRegistry.h>

bool GameSession::GameSessionModel::event(QEvent *e) {

    if(e->type() == gameSessionEvents.GameStateChanged) {
        auto event = (Internal::GameStateChangedEvent *)e;
        gameState = event->isEmpty()? SpellChessLib::GameState() : **event;
        onGameStateChanged(gameState);
        return true;
    } else if (e->type() == gameSessionEvents.MovesSinceRootChanged) {
       auto event = (Internal::MoveListEvent *)e;
       moveList = event->movesSinceRoot();
       onMoveListChanged(moveList);
       return true;
    } else if (e->type() == gameSessionEvents.GameRunnerIdChanged) {
        auto event = (Internal::GameRunnerEvent *)e;
        gameRunnerId = event->getGameRunnerId();
        onGameRunnerIdChanged(gameRunnerId);
        return true;
    };

    return QObject::event(e);
}

GameSession::GameSessionModel::GameSessionModel(std::shared_ptr<SpellChessLib::TimerService> timeServ, 
    const OpenSessionRequest & request,
     QCoreApplication *app) : runnerBridge(app, this), 
    userFunction(GameSessionModel::userFunctionFromRequest(request)), 
    qApplication(app){
    
    using PlayerPointer = typename SpellChessLib::Scheduler::ResourcePointer<SpellChessLib::Player>;
    
    PlayerPointer whitePlayer = playerFromDef(request.getWhitePlayer());
    PlayerPointer blackPlayer = playerFromDef(request.getBlackPlayer());

    gameRunner.reset(new SpellChessLib::GameRunner(timeServ, request.getGameRunnerConfig(),
     std::move(whitePlayer), std::move(blackPlayer)));
    
    gameRunner->attachGameMonitor(runnerBridge.getGameMonitor());
    
    gameThread = new std::thread([this](){
        this->gameRunner->run();
    });

}

GameSession::GameSessionModel::~GameSessionModel(){
    if(gameRunner->isRunning())
        gameRunner->stop();
    gameThread->join();
    delete gameThread;
}

bool GameSession::GameSessionModel::submitMove(const SpellChessLib::SpellChessMove &move){

    switch (userFunction) {
        case Observing: return false;
        case WhitePlayer:
            if(gameState.getSideToMove() == SpellChessLib::White) {
                runnerBridge.submitMove(move);
                return true;
            } 
            break;
        case BlackPlayer:
            if(gameState.getSideToMove() == SpellChessLib::Black) {
                runnerBridge.submitMove(move);
                return true;
            } 
            break;
        case SelfPlay:
            runnerBridge.submitMove(move);
            return true;        
    }

    return false;
}

const SpellChessLib::GameState &GameSession::GameSessionModel::getCurrentState() const{
    return gameState;
}

const std::vector<SpellChessLib::SpellChessMove> &GameSession::GameSessionModel::getMoveList() const{
    return moveList;
}

int GameSession::GameSessionModel::getGameRunnerId() const{
    return gameRunnerId;
}

GameSession::GameSessionModel::UserFunction GameSession::GameSessionModel::userFunctionFromRequest(const OpenSessionRequest &request) {
    using UserFunction = GameSessionModel::UserFunction;
    UserFunction userRole = UserFunction::Observing; 

    bool isWhiteHuman = request.getWhitePlayer().isHuman();
    bool isBlackHuman = request.getBlackPlayer().isHuman();

    if(isWhiteHuman || isBlackHuman)
        userRole = UserFunction::SelfPlay;
    else if(isWhiteHuman) 
        userRole = UserFunction::WhitePlayer;
    else if(isBlackHuman)
        userRole = UserFunction::BlackPlayer;

    return userRole;
}

GameSession::GameSessionModel::PlayerPointer GameSession::GameSessionModel::playerFromDef(const GameSession::PlayerDefinition &def) {
    if (def.isHuman()) {
        return this->runnerBridge.makePlayerFromSession();
    };
    
    using PlayerAlloc = typename SpellChessLib::Scheduler::DefaultObjectAllocator<SpellChessLib::Player>;
    // todo: Make this function request simoultanos game policies of engine players
    // todo: If we implement application wide scheduler, refactor this method to work with it
    const auto & playerEntry = SpellChessLib::PlayerRegistry->get(def.getPlayerId());
    auto playerAllocator = std::make_shared<PlayerAlloc>(playerEntry.instanceFactory);
    return playerAllocator->retrieve();
}

void GameSession::GameSessionView::connectSlots() {
    connect(board, &Board::BoardWidget::onDrag, this, &GameSessionView::handleDrag);
    connect(board, &Board::BoardWidget::onClick, this, &GameSessionView::handleClick);

    connect(model, &GameSessionModel::onGameStateChanged,   this, &GameSessionView::handleGamestateChange);
    connect(model, &GameSessionModel::onMoveListChanged,    this, &GameSessionView::handleMoveListChanged);
    connect(model, &GameSessionModel::onGameRunnerIdChanged,this, &GameSessionView::handleGameRunnerIdChanged);
}

GameSession::LinterInput GameSession::GameSessionView::prepareInfoForLinter() const {
    GameSession::LinterInput result;
    result.state = model->getCurrentState();
    
    switch(model->userFunction) {
        case GameSessionModel::Observing:
            result.playerColor = SpellChessLib::Nobody;
            break;
        case GameSessionModel::WhitePlayer:
            result.playerColor = SpellChessLib::White;
            break;
        case GameSessionModel::BlackPlayer:
            result.playerColor = SpellChessLib::Black;
            break;
        case GameSessionModel::SelfPlay:
            result.playerColor = result.state.getSideToMove();
            break;        
    };

    auto builderState = moveBuilder->build();
    result.selectedSquare = builderState.from;
    result.previewedFreezeSquare = builderState.freezeSquare;
    result.previewedJumpableSquare = builderState.jumpSquare;
    return result;
}

QPointF GameSession::GameSessionView::pointFromSquareIndex(unsigned int sqId) const
{
    if(sqId >= 64)
        return QPointF();
    
    unsigned int x = sqId & 7;
    unsigned int y = sqId >> 3;
    return QPointF(x, y);
}

void GameSession::GameSessionView::handleDrag(Board::DragHandle<Board::BoardWidget> & drag) {
        unsigned int finalSquareIndex = board->squareIndexFromMousePos(drag.getFinalPosition());
        interactionManager->onPieceDragEnded(finalSquareIndex);

        bool shouldAccept = board->boardGridModel.getPropertyValue(finalSquareIndex, Board::HasMoveDot);

        if (shouldAccept)
            drag.accept();
        else
            drag.reject();
};

void GameSession::GameSessionView::handleClick(Board::BoardWidget::ClickHandle &click){
    unsigned int squareIndex = board->squareIndexFromMousePos(click.getPosition());

    if (squareIndex > 63) {
        interactionManager->onSpellDiscarded();
        return;
    };

    const auto & sq = model->getCurrentState().getSquare(squareIndex); 

    if (board->boardGridModel.getPropertyValue(squareIndex, Board::HasMoveDot)) {
        interactionManager->onPieceTargetSquareClicked(squareIndex);
        return;
    }

    if (sq.isEmpty())
        interactionManager->onEmptySquareClicked(squareIndex);
    else
        interactionManager->onPieceClicked(squareIndex, sq.piece);
}

void GameSession::GameSessionView::handleGamestateChange(const SpellChessLib::GameState &state){
    board->entities.clear();

    static std::map<SpellChessLib::PieceType, Board::PieceType> libTypeToBoardTypeMap = {
        {SpellChessLib::Pawn, Board::Pawn},
        {SpellChessLib::Knight, Board::Knight},
        {SpellChessLib::Bishop, Board::Bishop},
        {SpellChessLib::Rook, Board::Rook},
        {SpellChessLib::Queen, Board::Queen},
        {SpellChessLib::King, Board::King}
    };

    auto linterInput = prepareInfoForLinter();
    uint64_t draggableMask =  getDraggablePieces(linterInput);
    for( unsigned int sq = 0; sq < 64; ++sq) {
        const auto & currentSq  = state.getSquare(sq);
        if(!currentSq.isEmpty()) {
           auto & newEntity = board->entities.addNewEntity();
           Board::PieceColor color = currentSq.piece.owner == SpellChessLib::White ? Board::White : Board::Black;
           Board::EntityInitializers::initPiece(newEntity, color,libTypeToBoardTypeMap[currentSq.piece.type]);
           newEntity.setProperty("location", pointFromSquareIndex(sq));
           newEntity.setProperty("size", QSizeF(1.f, 1.f));
           newEntity.setProperty("draggable", (bool)(draggableMask & (1ull << sq)));
        };
    }

    lintBoard(board->boardGridModel, linterInput);
    moveBuilder->reset();

}

GameSession::GameSessionView::GameSessionView(GameSessionModel *model, QWidget *parent) : QWidget(parent), model(model){
    interactionManager = nullptr;// std::make_shared<GameSession::UserInteractionManager>();
    moveBuilder = std::make_shared<GameSession::SpellChessMoveBuilder>();

    board = new Board::BoardWidget(this);
    layout = new QGridLayout(this);

    layout->addWidget(board, 1, 1, 8, 8);

    setLayout(layout);

    connectSlots();
}

GameSession::GameSessionView::~GameSessionView(){
    delete model;
}


#include <moc_GameSession.cpp>


