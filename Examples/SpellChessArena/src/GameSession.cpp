#include <GameSession.hpp>
#include <BoardGrid/EntityInitializers.hpp>
#include <map>

#include <SpellChess/PlayerRegistry.h>
#include <SpellChess/MoveGen.h>
#include <SpellChess/OpeningBookPlayerWrapper.h>

SpellChessLib::PieceType typeFromAction(Board::PawnPromotionAction action) {
    switch (action)
    {
        case Board::PromoteToBishop:
            return SpellChessLib::Bishop;
        case Board::PromoteToKnight:
            return SpellChessLib::Knight;
        case Board::PromoteToRook:
            return SpellChessLib::Rook;
        case Board::PromoteToQueen:
            return SpellChessLib::Queen;    
    };
};

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
    
    PlayerPointer whitePlayer = playerFromDef(request.getWhitePlayer(), request);
    PlayerPointer blackPlayer = playerFromDef(request.getBlackPlayer(), request);

    gameRunner.reset(new SpellChessLib::GameRunner(timeServ, request.getGameRunnerConfig(),
     std::move(whitePlayer), std::move(blackPlayer)));
    
    gameRunner->attachGameMonitor(runnerBridge.getGameMonitor());
    
    gameThread = new std::thread([this](){
        this->gameRunner->run();
    });

    rootState = gameRunner->getGame().getRoot();
}

GameSession::GameSessionModel::~GameSessionModel(){
    if(gameRunner->isRunning())
        gameRunner->stop();
    gameThread->join();
    delete gameThread;
}

bool GameSession::GameSessionModel::submitMove(const SpellChessLib::SpellChessMove &move){

    SpellChessLib::MoveGenerator gen(getCurrentState());
    if(!gen.isMoveLegal(move))
        return false;

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

const SpellChessLib::GameState & GameSession::GameSessionModel::getInitialState() const{
    return rootState;
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

    if(isWhiteHuman && isBlackHuman)
        userRole = UserFunction::SelfPlay;
    else if(isWhiteHuman) 
        userRole = UserFunction::WhitePlayer;
    else if(isBlackHuman)
        userRole = UserFunction::BlackPlayer;

    return userRole;
}

using PlayerAlloc = typename SpellChessLib::Scheduler::DefaultObjectAllocator<SpellChessLib::Player>;

SpellChessLib::Scheduler::ResourcePointer<SpellChessLib::Player> makeCPUPlayer(const GameSession::PlayerDefinition &def, const GameSession::OpenSessionRequest & request) {
    const auto & playerEntry = SpellChessLib::PlayerRegistry->get(def.getPlayerId());
    auto playerAllocator = std::make_shared<PlayerAlloc>(playerEntry.instanceFactory);
    if (request.openingBook == nullptr)
        return playerAllocator->retrieve(); 
    else {
        auto wrapperAlloc = std::make_shared<SpellChessLib::PlayerWithOpeningBookAllocator>(playerAllocator, request.openingBook);
        return wrapperAlloc->retrieve();
    }
};


GameSession::GameSessionModel::PlayerPointer GameSession::GameSessionModel::playerFromDef(const GameSession::PlayerDefinition &def, const OpenSessionRequest & request) {
    if (def.isHuman()) {
        return this->runnerBridge.makePlayerFromSession();
    };

    return makeCPUPlayer(def, request);
}

void GameSession::GameSessionView::connectSlots() {
    connect(board, &Board::BoardWidget::onDrag, this, &GameSessionView::handleDrag);
    connect(board, &Board::BoardWidget::onClick, this, &GameSessionView::handleClick);

    connect(model, &GameSessionModel::onGameStateChanged,   this, &GameSessionView::handleGamestateChange);
    connect(model, &GameSessionModel::onMoveListChanged,    this, &GameSessionView::handleMoveListChanged);
    connect(model, &GameSessionModel::onGameRunnerIdChanged,this, &GameSessionView::handleGameRunnerIdChanged);

    connect(&whiteSpellPanel, &SpellPanelModel::spellSelected, this, &GameSessionView::handleSpellSelect);
    connect(&whiteSpellPanel, &SpellPanelModel::spellDiscarded, this, &GameSessionView::handleSpellDiscarded);
    connect(&blackSpellPanel, &SpellPanelModel::spellSelected, this, &GameSessionView::handleSpellSelect);
    connect(&blackSpellPanel, &SpellPanelModel::spellDiscarded, this, &GameSessionView::handleSpellDiscarded);
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
    interactionManager->onPieceDrag(drag);
};

void GameSession::GameSessionView::handleClick(Board::BoardWidget::ClickHandle &click){
    unsigned int squareIndex = board->squareIndexFromBoardPos(click.getPosition());

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
        {SpellChessLib::Pawn,   Board::Pawn},
        {SpellChessLib::Knight, Board::Knight},
        {SpellChessLib::Bishop, Board::Bishop},
        {SpellChessLib::Rook,   Board::Rook},
        {SpellChessLib::Queen,  Board::Queen},
        {SpellChessLib::King,   Board::King}
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

    whiteSpellPanel.setSpell(state.getSpell(SpellChessLib::White, SpellChessLib::FreezeSpell));
    whiteSpellPanel.setSpell(state.getSpell(SpellChessLib::White, SpellChessLib::JumpSpell));
    blackSpellPanel.setSpell(state.getSpell(SpellChessLib::Black, SpellChessLib::FreezeSpell));
    blackSpellPanel.setSpell(state.getSpell(SpellChessLib::Black, SpellChessLib::JumpSpell));

    lintBoard(board->boardGridModel, linterInput);
    moveBuilder->reset();
    board->update();
}

void GameSession::GameSessionView::handleMoveListChanged(const std::vector<SpellChessLib::SpellChessMove> &moves){
    SpellChessLib::Annotation::AnnotatedSequence seq;
    for(const auto & m : moves)
        seq.pushBack(m);
    moveListModel->set(model->getInitialState(), seq);
}

GameSession::GameSessionView::GameSessionView(GameSessionModel *model, QWidget *parent) : QWidget(parent), model(model){
    interactionManager = std::make_shared<GameSession::UserInteractionManager>();
    moveBuilder = std::make_shared<GameSession::SpellChessMoveBuilder>();
    moveListModel = std::make_shared<MoveListModel>();
    interactionManager->setCurrentState(std::make_unique<States::NoFocus>(this, interactionManager));
    
    board = new Board::BoardWidget(this);
    layout = new QGridLayout(this);
    moveList = new MoveListView(moveListModel, this);

    SpellPanel * blPanel = new SpellPanel(blackSpellPanel);
    SpellPanel * whPanel = new SpellPanel(whiteSpellPanel);
    

    layout->addWidget(blPanel, 0, 6, 1, 2);
    layout->addWidget(whPanel, 9, 6, 1, 2);
    layout->addWidget(board, 1, 1, 8, 8);
    layout->addWidget(moveList, 1, 9, 8, 3);

    setLayout(layout);

    connectSlots();
}

GameSession::GameSessionView::~GameSessionView(){
    delete model;
}

void GameSession::GameSessionView::updateLinter() {
    auto linterInput = prepareInfoForLinter();
    lintBoard(board->boardGridModel, linterInput);
    update();
};

void GameSession::GameSessionView::handlePromotionActionSelected(Board::PawnPromotionAction action) {
    moveBuilder->setPromotion(typeFromAction(action));
    model->submitMove(moveBuilder->build());
    moveBuilder->reset();
    board->closePromotionContextMenu();
    updateLinter();
    board->update();
    interactionManager->setCurrentState(std::make_unique<States::NoFocus>(this, interactionManager));
}

void GameSession::GameSessionView::handleSpellSelect(SpellChessLib::SpellIdentifator spell) {
    interactionManager->setCurrentState(std::make_unique<States::SpellSquarePicker>(spell, this, interactionManager));
}

void GameSession::GameSessionView::handleSpellDiscarded() {
    interactionManager->setCurrentState(std::make_unique<States::NoFocus>(this, interactionManager));
};

bool GameSession::States::NoFocus::isPromotionMove(SpellChessLib::SquareIndex targetSquare) const {
    SpellChessLib::SpellChessMove moveSoFar = view->moveBuilder->build();
    SpellChessLib::SquareIndex from = moveSoFar.from;
    SpellChessLib::GameState state = view->model->getCurrentState();

    SpellChessLib::Piece movedPiece = (from != SpellChessLib::NoSquare) ? 
            state.getSquare(from).piece :
            SpellChessLib::Piece({SpellChessLib::None, SpellChessLib::Nobody});

    bool isPawnMove = movedPiece.type == SpellChessLib::Pawn;
    bool inPromotionZone = targetSquare != SpellChessLib::NoSquare && (targetSquare < 8 || targetSquare >= 56);
    return isPawnMove && inPromotionZone;
};


void GameSession::States::NoFocus::onSpellSelected(SpellChessLib::SpellIdentifator spellId) {
    transistTo(std::make_unique<SpellSquarePicker>(spellId, view, manager));
};

void GameSession::States::NoFocus::onSpellDiscarded() {
    // ignored
}

void GameSession::States::NoFocus::onEmptySquareClicked(SpellChessLib::SquareIndex squareIndex) {
    view->moveBuilder->reset();
    view->updateLinter();
}

void GameSession::States::NoFocus::onPieceClicked(SpellChessLib::SquareIndex pieceSquare, SpellChessLib::Piece piece) {
    bool canMove =  view->model->userFunction == GameSessionModel::UserFunction::SelfPlay ||
                    (piece.owner == SpellChessLib::White && view->model->userFunction == GameSessionModel::UserFunction::WhitePlayer) ||
                    (piece.owner == SpellChessLib::Black && view->model->userFunction == GameSessionModel::UserFunction::BlackPlayer);

    canMove &= (piece.owner == view->model->getCurrentState().getSideToMove());

    if (canMove) {
        view->moveBuilder->setFromSquare(pieceSquare);
        view->moveBuilder->setDestSquare(SpellChessLib::NoSquare);
        view->updateLinter();
        view->board->update();
    } else 
        onEmptySquareClicked(pieceSquare);
}

void GameSession::States::NoFocus::onPieceTargetSquareClicked(SpellChessLib::SquareIndex targetSquare) {
    if (isPromotionMove(targetSquare)) {
        transistTo(std::make_unique<PromotionPicker>(targetSquare, view, manager));
    } else {
        view->moveBuilder->setDestSquare(targetSquare);
        view->model->submitMove(view->moveBuilder->build());
        view->moveBuilder->reset();
        view->updateLinter();
    };
}

void GameSession::States::NoFocus::onPieceDrag(Board::DragHandle<Board::BoardWidget> & drag) {
    SpellChessLib::SquareIndex fromSq = view->board->squareIndexFromBoardPos(drag.getStartingPosition());
    SpellChessLib::SquareIndex toSq = view->board->squareIndexFromBoardPos(drag.getFinalPosition());


    if (fromSq > 63 || toSq > 63) {
        view->moveBuilder->reset();
        view->updateLinter();
        drag.reject();
        view->board->update();
        return;
    };

    view->moveBuilder->setFromSquare(fromSq);
    view->moveBuilder->setDestSquare(toSq);
    

    SpellChessLib::MoveGenerator gen(view->model->getCurrentState());

    if (isPromotionMove(toSq)) {
        drag.reject();
        transistTo(std::make_unique<PromotionPicker>(toSq, view, manager));
    } else {
        auto move = view->moveBuilder->build();
        bool isLegal = gen.isMoveLegal(move);

        if(!isLegal){
            drag.reject();
            view->board->update();
            return;
        }
        view->model->submitMove(move);
        view->moveBuilder->reset();
        view->updateLinter();
    };
};

GameSession::States::PromotionPicker::PromotionPicker(SpellChessLib::SquareIndex anchorIndex, GameSessionView * view, std::weak_ptr<UserInteractionManager> manager) :
view(view), UserInteractionState(manager) {
    auto & menu = view->board->openPromotionContextMenu();
    int x = anchorIndex % 8;
    int y = anchorIndex / 8;

    if (y  + 4 >= 8)
        y = 4;

    view->moveBuilder->setDestSquare(anchorIndex);

    menu.setPosition(QPointF(x, y));
    menu.setPiecesColor(anchorIndex >= 56 ? Board::Black : Board::White);
    auto viewPtr = view;

    menu.connect(&menu, &Board::PromotionContextMenuOverlay::onActionSelected, view, &GameSessionView::handlePromotionActionSelected);
    view->board->update();
};

void GameSession::States::PromotionPicker::discard() {
    view->moveBuilder->reset();
    view->board->closePromotionContextMenu();
    view->updateLinter();
    view->board->update();
    transistTo(std::make_unique<NoFocus>(view, manager));
};

void GameSession::States::SpellSquarePicker::discard(){
    view->moveBuilder->reset();
    view->updateLinter();
    view->board->update();
    transistTo(std::make_unique<NoFocus>(view, manager));
}

void GameSession::States::SpellSquarePicker::onSpellSelected(SpellChessLib::SpellIdentifator spellId)
{
    if (spellId !=  this->spellId ) {
        transistTo(std::make_unique<SpellSquarePicker>(spellId, view, manager));
    };
}

void GameSession::States::SpellSquarePicker::onEmptySquareClicked(SpellChessLib::SquareIndex squareIndex)
{
    if(spellId == SpellChessLib::FreezeSpell) {
        view->moveBuilder->setFreezeSquare(squareIndex);
    } else {
        view->moveBuilder->setJumpSquare(squareIndex);
    }
    view->updateLinter();
    view->board->update();
    transistTo(std::make_unique<NoFocus>(view, manager));
}

GameSession::States::SpellSquarePicker::SpellSquarePicker(SpellChessLib::SpellIdentifator spell, GameSessionView *view, std::weak_ptr<UserInteractionManager> manager) :
    UserInteractionState(manager), view(view), spellId(spell) {
}

#include <moc_GameSession.cpp>


