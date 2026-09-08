#include <BoardViewerWidget.hpp>
#include <BoardLinter.hpp>
#include <SingleItemSquareLayout.hpp>
#include <BoardGrid/EntityInitializers.hpp>
#include <SpellChess/Board.h>

#include <QGridLayout>
#include <QHBoxLayout>

#include <map>
#include <QKeyEvent>
class BoardViewer::BoardViewerWidget::NoFocus : public BoardViewer::UserInteractionState {
    BoardViewerWidget * widget;

    void onSpellSelected(SpellChessLib::SpellIdentificator spellId) override;
    void onSpellDiscarded() override;
    void onEmptySquareClicked(SpellChessLib::SquareIndex squareIndex) override;
    void onPieceClicked(SpellChessLib::SquareIndex pieceSquare, SpellChessLib::Piece piece) override;
    void onPieceTargetSquareClicked(SpellChessLib::SquareIndex targetSquare) override;
    void onPieceDrag(Board::DragHandle<Board::BoardWidget> & drag) override;
    bool isPromotionMove(SpellChessLib::SquareIndex targetSquare) const;
    
    public:
    NoFocus(BoardViewerWidget * view, std::weak_ptr<UserInteractionManager> manager) : widget(view), UserInteractionState(manager) {};
        
};

class BoardViewer::BoardViewerWidget::PromotionPicker : public UserInteractionState {
    BoardViewerWidget * widget;

    void discard();
    void onSpellSelected(SpellChessLib::SpellIdentificator spellId) override;
    void onSpellDiscarded() override;
    void onEmptySquareClicked(SpellChessLib::SquareIndex squareIndex) override ;
    void onPieceClicked(SpellChessLib::SquareIndex pieceSquare, SpellChessLib::Piece piece) override;
    void onPieceTargetSquareClicked(SpellChessLib::SquareIndex targetSquare) override;
    void onPieceDrag(Board::DragHandle<Board::BoardWidget> & drag) override;

    public:
    PromotionPicker(SpellChessLib::SquareIndex anchorIndex, BoardViewerWidget * widget, std::weak_ptr<UserInteractionManager> manager);
};

class BoardViewer::BoardViewerWidget::SpellSquarePicker : public UserInteractionState {
    BoardViewerWidget * widget;
    SpellChessLib::SpellIdentificator spellId;

    void discard();
    void onSpellSelected(SpellChessLib::SpellIdentificator spellId) override;
    void onSpellDiscarded() override;
    void onEmptySquareClicked(SpellChessLib::SquareIndex squareIndex) override;
    void onPieceClicked(SpellChessLib::SquareIndex pieceSquare, SpellChessLib::Piece piece) override;
    void onPieceTargetSquareClicked(SpellChessLib::SquareIndex targetSquare) override;
    void onPieceDrag(Board::DragHandle<Board::BoardWidget> & drag) override;
    std::optional<SpellChessLib::SpellIdentificator> spellPickerRenderTip() const override;
    
    public:
    SpellSquarePicker(SpellChessLib::SpellIdentificator spell, BoardViewerWidget * widget, std::weak_ptr<UserInteractionManager> manager): 
    UserInteractionState(manager), widget(widget), spellId(spell) {
        widget->moveBuilder->reset(); // reset move creation
    }
};

void BoardViewer::BoardViewerWidget::connectSlots() {
    connect(board, &Board::BoardWidget::onDrag, this, &BoardViewerWidget::handleDrag);
    connect(board, &Board::BoardWidget::onClick, this, &BoardViewerWidget::handleClick);
    connect(board, &Board::BoardWidget::onDragStarted, this, &BoardViewerWidget::handleClick);

    connect(model, &Model::changed, this, &BoardViewerWidget::handleModelChanged);    
    connect(model, &Model::onTimerChanged, this, &BoardViewerWidget::handleTimePairChanged);
    connect(model, &Model::documentChanged, this, &BoardViewerWidget::handleDocumentChanged);

    connect(&whiteSpellPanel, &SpellPanelModel::spellSelected, this, &BoardViewerWidget::handleSpellSelect);
    connect(&whiteSpellPanel, &SpellPanelModel::spellDiscarded, this, &BoardViewerWidget::handleSpellDiscarded);
    connect(&blackSpellPanel, &SpellPanelModel::spellSelected, this, &BoardViewerWidget::handleSpellSelect);
    connect(&blackSpellPanel, &SpellPanelModel::spellDiscarded, this, &BoardViewerWidget::handleSpellDiscarded);

    connect(moveList, &MoveListView::nodeClicked, this, &BoardViewerWidget::handleNodeClicked);
    connect(moveList, &MoveListView::navButtonClicked, this, &BoardViewerWidget::handleMoveListNav);
    connect(moveList, &MoveListView::currentCommentEdited, this, &BoardViewerWidget::handleCommentEdit);
}

BoardViewer::LinterInput BoardViewer::BoardViewerWidget::prepareInfoForLinter() const {
    LinterInput result;
    auto doc = model->getDocument();
    result.state = doc->getGameState(model->getCurrentPosition());
    
    result.playerColor = model->canUserSubmitMove() ?
                        result.state.getSideToMove() :
                        SpellChessLib::Nobody;

    auto builderState = moveBuilder->build();
    result.selectedSquare = builderState.from;
    result.previewedFreezeSquare = builderState.freezeSquare;
    result.previewedJumpableSquare = builderState.jumpSquare;
    result.selectedSpell = interactionManager->spellPickerRenderTip();
    result.isDragging = board->isDragging();
    return result;
}

QPointF BoardViewer::BoardViewerWidget::pointFromSquareIndex(unsigned int sqId) const
{
    if(sqId >= 64)
        return QPointF();
    
    unsigned int x = sqId & 7;
    unsigned int y = sqId >> 3;
    return QPointF(x, y);
}

void BoardViewer::BoardViewerWidget::handleDrag(Board::DragHandle<Board::BoardWidget> & drag) {
    interactionManager->onPieceDrag(drag);
};

void BoardViewer::BoardViewerWidget::handleClick(Board::BoardWidget::ClickHandle &click){
    unsigned int squareIndex = board->squareIndexFromBoardPos(click.getPosition());

    if (squareIndex > 63) {
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

void BoardViewer::BoardViewerWidget::handleGamestateChange(const SpellChessLib::GameState &state){
    board->entities.clear();

    static std::map<SpellChessLib::PieceType, Board::PieceType> libTypeToBoardTypeMap = {
        {SpellChessLib::Pawn,   Board::Pawn},
        {SpellChessLib::Knight, Board::Knight},
        {SpellChessLib::Bishop, Board::Bishop},
        {SpellChessLib::Rook,   Board::Rook},
        {SpellChessLib::Queen,  Board::Queen},
        {SpellChessLib::King,   Board::King}
    };

    moveBuilder->reset();
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
    board->update();
}

void BoardViewer::BoardViewerWidget::handleMoveListChanged(const std::vector<SpellChessLib::SpellChessMove> &moves){
    // ignored
}
void BoardViewer::BoardViewerWidget::handleTimePairChanged(const SpellChessLib::TimerPairState & state) {
    whiteTimer->setState(state.whiteTimer);
    blackTimer->setState(state.blackTimer);
}

void BoardViewer::BoardViewerWidget::handleModelChanged(const Model & model) {
    handleGamestateChange(model.getCurrentState());
    moveList->currentPosChanged(model.getCurrentPosition());
};

void BoardViewer::BoardViewerWidget::handleDocumentChanged(const SpellChessLib::SpellChessDocument & document) {
    moveList->documentChanged(document, model->getCurrentPosition());
}


BoardViewer::BoardViewerWidget::BoardViewerWidget(Model *model, QWidget *parent) : QWidget(parent), model(model){
    moveBuilder = std::make_shared<SpellChessMoveBuilder>();
    
    board = new Board::BoardWidget(this);
    layout = new QHBoxLayout(this);
    moveList = new MoveListView(this);

    whiteTimer = new PlayerTimer(this);
    blackTimer = new PlayerTimer(this); 
    
    QGridLayout * gridLayout = new QGridLayout(this);
    SingleItemSquareLayout * boardContainer = new SingleItemSquareLayout();
    boardContainer->addWidget(board);
    
    gridLayout->addLayout(boardContainer, 1, 1, 8, 8);
    gridLayout->addWidget(blackTimer, 0, 1, 1, 2);
    gridLayout->addWidget(whiteTimer, 9, 1, 1, 2);

    layout->addLayout(gridLayout, 3);
    layout->addWidget(moveList, 1);
    moveList->setMaximumWidth(300);

    if (model->getCurrentState().isSpellChessGame()) {
        SpellPanel * blPanel = new SpellPanel(blackSpellPanel);
        SpellPanel * whPanel = new SpellPanel(whiteSpellPanel);
        gridLayout->addWidget(blPanel, 0, 6, 1, 2);
        gridLayout->addWidget(whPanel, 9, 6, 1, 2);
    }

    setLayout(layout);

    connectSlots();
    setFocusPolicy(Qt::FocusPolicy::StrongFocus);

    interactionManager = std::make_shared<UserInteractionManager>([this]() {
        // When state changes, update linter in case if render tips had changed
        this->updateLinter();
    });
    interactionManager->setCurrentState(std::make_unique<NoFocus>(this, interactionManager));

    handleDocumentChanged(*model->getDocument()); // neccessary for document viewer to display correctly
    handleModelChanged(*model);
}

BoardViewer::BoardViewerWidget::~BoardViewerWidget(){
    delete model;
}

void BoardViewer::BoardViewerWidget::updateLinter() {
    auto linterInput = prepareInfoForLinter();
    lintBoard(board->boardGridModel, linterInput);
    update();
};

void BoardViewer::BoardViewerWidget::keyPressEvent(QKeyEvent * event) {
    switch (event->key())
    {
        case Qt::Key_Left:
            model->goBackward(1);
            break;
        
        case Qt::Key_Up:
            model->goBackward(2);
            break;

         case Qt::Key_Right:
            model->goForward(1);
            break;

        case Qt::Key_Down:
            model->goForward(2);
            break;

        case Qt::Key_Home:
            model->goToDocumentRoot();
            break;

        case Qt::Key_End:
            model->goToDocumentTail();
            break;
        default:
            QWidget::keyPressEvent(event);
            break;
    }
   
};


#define PROMOTION_ACTION(type) Board::PromoteTo##type
#define SPELLCHESSLIB_PIECE(type) SpellChessLib:: type
#define PROMOTION_TARGET(type) {PROMOTION_ACTION(type), SPELLCHESSLIB_PIECE(type)}

void BoardViewer::BoardViewerWidget::handlePromotionActionSelected(Board::PawnPromotionAction action) {
    static std::map<Board::PawnPromotionAction, SpellChessLib::PieceType> actionToPieceTypeMap = {
        PROMOTION_TARGET(Bishop),
        PROMOTION_TARGET(Knight),
        PROMOTION_TARGET(Rook),
        PROMOTION_TARGET(Queen)
    };
  
    moveBuilder->setPromotion(actionToPieceTypeMap[action]);
    model->makeMove(moveBuilder->build());
    board->closePromotionContextMenu();
    updateLinter();
    board->update();
    interactionManager->setCurrentState(std::make_unique<NoFocus>(this, interactionManager));
}

void BoardViewer::BoardViewerWidget::handleSpellSelect(SpellChessLib::SpellIdentificator spell) {
    interactionManager->setCurrentState(std::make_unique<SpellSquarePicker>(spell, this, interactionManager));
}

void BoardViewer::BoardViewerWidget::handleSpellDiscarded() {
    moveBuilder->clearSpells();
    interactionManager->setCurrentState(std::make_unique<NoFocus>(this, interactionManager));
    updateLinter();
};

void BoardViewer::BoardViewerWidget::handleNodeClicked(int nodeId) {
    auto newPos = model->getDocument()->getNodeById(nodeId);
    model->setCurrentPosition(newPos);
}

void  BoardViewer::BoardViewerWidget::handleMoveListNav(MoveListView::NavigationDirective directive) {
    switch (directive)
    {
        case MoveListView::NavigationDirective::GoToRoot:
            model->goToDocumentRoot();
            break;
        case MoveListView::NavigationDirective::GoToPreviousState:
            model->goBackward(1);
            break;
        case MoveListView::NavigationDirective::GoToNextState:
            model->goForward(1);
            break;
        case MoveListView::NavigationDirective::GoToTail:
            model->goToDocumentTail();
            break;
        default:
            break;
    }
};

void BoardViewer::BoardViewerWidget::handleCommentEdit(const QString & newComment) {
    model->setComment(newComment.toStdString());
}



void BoardViewer::BoardViewerWidget::NoFocus::onSpellSelected(SpellChessLib::SpellIdentificator spellId)  {
    transistTo(std::make_unique<SpellSquarePicker>(spellId, widget, manager));
};

void BoardViewer::BoardViewerWidget::NoFocus::onSpellDiscarded() {
        // ignored    
};

void BoardViewer::BoardViewerWidget::NoFocus::onEmptySquareClicked(SpellChessLib::SquareIndex squareIndex) {
    widget->moveBuilder->clearSquares();
    widget->updateLinter();
}

void BoardViewer::BoardViewerWidget::NoFocus::onPieceClicked(SpellChessLib::SquareIndex pieceSquare, SpellChessLib::Piece piece) {
    bool canMove = widget->model->canUserSubmitMove();

    canMove &= (piece.owner == widget->model->getCurrentState().getSideToMove());

    if (canMove) {
        widget->moveBuilder->setFromSquare(pieceSquare);
        widget->moveBuilder->setDestSquare(SpellChessLib::NoSquare);
        widget->updateLinter();
        widget->board->update();
    } else 
        onEmptySquareClicked(pieceSquare);
};

void BoardViewer::BoardViewerWidget::NoFocus::onPieceTargetSquareClicked(SpellChessLib::SquareIndex targetSquare) {
        if (isPromotionMove(targetSquare)) {
        transistTo(std::make_unique<PromotionPicker>(targetSquare, widget, manager));
    } else {
        widget->moveBuilder->setDestSquare(targetSquare);
        widget->model->makeMove(widget->moveBuilder->build());
    };
};

void BoardViewer::BoardViewerWidget::NoFocus::onPieceDrag(Board::DragHandle<Board::BoardWidget> & drag) {
    SpellChessLib::SquareIndex fromSq = widget->board->squareIndexFromBoardPos(drag.getStartingPosition());
    SpellChessLib::SquareIndex toSq = widget->board->squareIndexFromBoardPos(drag.getFinalPosition());


    if (fromSq > 63 || toSq > 63) {
        widget->moveBuilder->clearSquares();
        widget->updateLinter();
        drag.reject();
        widget->board->update();
        return;
    };

    widget->moveBuilder->setFromSquare(fromSq);
    widget->moveBuilder->setDestSquare(toSq);
    

    SpellChessLib::MoveGenerator gen(widget->model->getCurrentState());

    if (isPromotionMove(toSq)) {
        drag.reject();
        transistTo(std::make_unique<PromotionPicker>(toSq, widget, manager));
    } else {
        auto move = widget->moveBuilder->build();
        bool isLegal = gen.isMoveLegal(move);

        if(!isLegal){
            drag.reject();
            widget->board->update();
            return;
        }
        widget->model->makeMove(move);
    };
};

bool BoardViewer::BoardViewerWidget::NoFocus::isPromotionMove(SpellChessLib::SquareIndex targetSquare) const {
    SpellChessLib::SpellChessMove moveSoFar = widget->moveBuilder->build();
    SpellChessLib::SquareIndex from = moveSoFar.from;

    SpellChessLib::GameState state = widget->model->getCurrentState();

    SpellChessLib::Piece movedPiece = (from != SpellChessLib::NoSquare) ? 
            state.getSquare(from).piece :
            SpellChessLib::Piece({SpellChessLib::None, SpellChessLib::Nobody});

    bool isPawnMove = movedPiece.type == SpellChessLib::Pawn;
    bool inPromotionZone = targetSquare != SpellChessLib::NoSquare && (targetSquare < 8 || targetSquare >= 56);
    return isPawnMove && inPromotionZone;
};

void BoardViewer::BoardViewerWidget::PromotionPicker::discard() {
        widget->moveBuilder->clearSquares();
        widget->board->closePromotionContextMenu();
        widget->updateLinter();
        widget->board->update();
        transistTo(std::make_unique<NoFocus>(widget, manager));
}

void BoardViewer::BoardViewerWidget::PromotionPicker::onSpellSelected(SpellChessLib::SpellIdentificator spellId) {
    discard();
}

void BoardViewer::BoardViewerWidget::PromotionPicker::onSpellDiscarded() {
    discard();
}

void BoardViewer::BoardViewerWidget::PromotionPicker::onEmptySquareClicked(SpellChessLib::SquareIndex squareIndex) {
    discard();
}

void BoardViewer::BoardViewerWidget::PromotionPicker::onPieceClicked(SpellChessLib::SquareIndex pieceSquare, SpellChessLib::Piece piece) {
    discard();
}

void BoardViewer::BoardViewerWidget::PromotionPicker::onPieceTargetSquareClicked(SpellChessLib::SquareIndex targetSquare) {
    discard();
}

void BoardViewer::BoardViewerWidget::PromotionPicker::onPieceDrag(Board::DragHandle<Board::BoardWidget> & drag) {
    discard();
}

BoardViewer::BoardViewerWidget::PromotionPicker::PromotionPicker(SpellChessLib::SquareIndex anchorIndex, BoardViewerWidget * widget, std::weak_ptr<UserInteractionManager> manager):
    widget(widget), UserInteractionState(manager) {
    auto & menu = widget->board->openPromotionContextMenu();
    int x = anchorIndex % 8;
    int y = anchorIndex / 8;

    if (y  + 4 >= 8)
        y = 4;

    widget->moveBuilder->setDestSquare(anchorIndex);

    menu.setPosition(QPointF(x, y));
    menu.setPiecesColor(anchorIndex >= 56 ? Board::Black : Board::White);
    menu.connect(&menu, &Board::PromotionContextMenuOverlay::onActionSelected, widget, &BoardViewerWidget::handlePromotionActionSelected);
    
    widget->board->update();
}

void BoardViewer::BoardViewerWidget::SpellSquarePicker::discard() {
    widget->moveBuilder->clearSquares();
    widget->updateLinter();
    widget->board->update();
    transistTo(std::make_unique<NoFocus>(widget, manager));
}

void BoardViewer::BoardViewerWidget::SpellSquarePicker::onSpellSelected(SpellChessLib::SpellIdentificator spellId) {
    if (spellId !=  this->spellId ) 
        transistTo(std::make_unique<SpellSquarePicker>(spellId, widget, manager));
}

void BoardViewer::BoardViewerWidget::SpellSquarePicker::onSpellDiscarded() {
    discard();
};   

void BoardViewer::BoardViewerWidget::SpellSquarePicker::onEmptySquareClicked(SpellChessLib::SquareIndex squareIndex) {

    if(spellId == SpellChessLib::FreezeSpell)
        widget->moveBuilder->setFreezeSquare(squareIndex);
    else
        widget->moveBuilder->setJumpSquare(squareIndex);

    transistTo(std::make_unique<NoFocus>(widget, manager));
}

void BoardViewer::BoardViewerWidget::SpellSquarePicker::onPieceClicked(SpellChessLib::SquareIndex pieceSquare, SpellChessLib::Piece piece) {
    onEmptySquareClicked(pieceSquare);
}

void BoardViewer::BoardViewerWidget::SpellSquarePicker::onPieceTargetSquareClicked(SpellChessLib::SquareIndex targetSquare) {
    discard();
}

void BoardViewer::BoardViewerWidget::SpellSquarePicker::onPieceDrag(Board::DragHandle<Board::BoardWidget> & drag) {
    discard();
}

std::optional<SpellChessLib::SpellIdentificator> BoardViewer::BoardViewerWidget::SpellSquarePicker::spellPickerRenderTip() const {
    return spellId;
}


#include <moc_BoardViewerWidget.cpp>