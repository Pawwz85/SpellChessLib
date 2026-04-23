#include <BoardGridWidget.h>
#include <QPaintEvent>
#include <QPainter>
#include <map>
#include <vector>
#include <mutex>
#include <algorithm>
#include <PieceSpriteSet.hpp>

using namespace Board;

BoardGridWidget::BoardGridWidget(QWidget * parent): QWidget(parent) {
    setMouseTracking(true);
    QSizePolicy p = sizePolicy();
    p.setHeightForWidth(true);
    setSizePolicy(p);
    connect(&boardGridModel, &BoardGridModel::onConfigChanged, this, &BoardGridWidget::onModelChanged);
    connect(&boardGridModel, &BoardGridModel::onDataChanged, this, &BoardGridWidget::onModelChanged);
    connect(&boardGridModel, &BoardGridModel::onReversed, this, &BoardGridWidget::onModelChanged);
};


QColor BoardGridWidget::pickColorForSquare(unsigned int index) {
    BoardGridModel & model = boardGridModel;
    const BoardGridConfig & config = boardGridModel.getConfig();

    unsigned int x = index & 7, y = index >> 3;

    uint32_t baseColour = ((x + y) % 2 == 0) ? config.baseDarkColour : config.baseLightColour;
    if (model.getPropertyValue(index, Higlighted))
        baseColour = config.baseHighlightedColour;

    return QRgb(baseColour);
};

void BoardGridWidget::paintEvent(QPaintEvent * event) {
    const QRect & rect = event->rect();
    QWidget::paintEvent(event);
    QPainter painter(this);

    int step_y = height() / 8;
    int step_x = width() / 8;
    
    // step 1. Draw the base board
    for(unsigned int col = 0; col < 8; ++col) 
        for (unsigned int row = 0; row < 8; ++row) {
            unsigned int squareIndex = boardGridModel.getIndex(col, row);
            if(squareIndex == (unsigned int)-1) continue;

            painter.setBrush(pickColorForSquare(squareIndex)); 
            int x = col * step_x;
            int y = row * step_y;
            painter.drawRect(x, y, step_x, step_y);
            if (boardGridModel.getPropertyValue(squareIndex, HasMoveDot)) {
                painter.setBrush(QColorConstants::DarkGray);
                painter.drawEllipse(QPoint(x + step_x / 2, y + step_y/2), step_x/8, step_y/8);
            }
        }

    // step 2. Draw pieces or other stuff suppossed to be affected visually by spell (noop unless defined by subclass)
    drawPieces(painter);    
    
    // step 3. Draw spell effects
    for(unsigned int col = 0; col < 8; ++col) 
    for (unsigned int row = 0; row < 8; ++row) {
        unsigned int squareIndex = boardGridModel.getIndex(col, row);
        if(squareIndex == (unsigned int)-1) continue;

        int x = col * step_x;
        int y = row * step_y;
        
        if (boardGridModel.getPropertyValue(squareIndex, Frozen)){
            painter.setBrush(QColor::fromRgba(boardGridModel.getConfig().freezeAreaColour));
            painter.drawRect(x, y, step_x, step_y);
        }

        if (boardGridModel.getPropertyValue(squareIndex, Jumpable)){
            painter.setBrush(QColor::fromRgba(boardGridModel.getConfig().jumpSquareColour));
            painter.drawRect(x, y, step_x, step_y);
        } 
    }

    // step 4. Allow subclasses draw things unaffected by spell effects
    drawExtras(painter);
}

void BoardGridWidget::resetHiglight() {
     boardGridModel.startEdit().setPropertyBitmask(Board::Higlighted, 0).commit();
};


void BoardGridWidget::mouseMoveEvent(QMouseEvent* event) {
    QPointF pos = event->position();
    int col = qFloor(8 * pos.x() / width());
    int row = qFloor(8 * pos.y() / height());

    if(col < 0 || row < 0) {
        resetHiglight();
        return;
    }
    unsigned int hoveredSquareIndex = boardGridModel.getIndex(col, row);

    if (hoveredSquareIndex == (unsigned int) -1 || !boardGridModel.getPropertyValue(hoveredSquareIndex, Board::Interactable)) {
        resetHiglight();
        return;
    }

    boardGridModel.startEdit().setPropertyBitmask(Board::Higlighted, 1ull << hoveredSquareIndex).commit();
};


QSize BoardGridWidget::sizeHint() const {
    return QSize(640, 640);
}


void BoardGridWidget::onModelChanged(){
    update();
};

void zLevelSplit (  const BoardEntityList & list, 
                    int threshold,
                    std::vector<const BoardEntity * > & lower,
                    std::vector<const BoardEntity *>  & upper) {
    for (const auto & entity : list)
        (entity.property("z_level").toInt() > threshold ? upper : lower).push_back(&entity);
};

class BoardEntityZLevelComparator {
    public:
    bool operator()(const BoardEntity * o1, const BoardEntity * o2) {
        return o1->property("z_level").toInt() < o2->property("z_level").toInt();
    };
};

void BoardWidget::paintEvent(QPaintEvent * event) {
    renderQueueLower.clear();
    renderQueueUpper.clear();
    zLevelSplit(entities, 1, renderQueueLower, renderQueueUpper);

    std::sort(renderQueueLower.begin(), renderQueueLower.end(), BoardEntityZLevelComparator());
    std::sort(renderQueueUpper.begin(), renderQueueUpper.end(), BoardEntityZLevelComparator());

    BoardGridWidget::paintEvent(event);

}

unsigned int Board::BoardWidget::squareIndexFromMousePos(QPointF pos){
    int x = qFloor(pos.x());
    int y = qFloor(pos.y());

    if (boardGridModel.property("reversed").toBool())
        y = 7 - y;

    return 8 * y + x;
}

void BoardWidget::boardEntityDrawingRoutine( QPainter & painter,
                                const std::vector<const BoardEntity *> & renderQueue,
                                float scalingFactorX,
                                float scalingFactorY) {
    const QPixmap * sprite;
    for (const auto & piece : renderQueue) {
        if ((sprite = piece->getSpritePointer())) {
            QSizeF size = piece->property("size").toSizeF();
            QPointF where  = piece->property("location").toPointF();

            QRect target(where.x() * scalingFactorX, where.y() * scalingFactorY,
              size.width() * scalingFactorX,
              size.height() * scalingFactorY);
            
              painter.drawPixmap(target, *sprite);

        };
    };
}

void BoardWidget::drawPieces(QPainter & painter) {
    float scalingFactorX = width() / 8.f;
    float scalingFactorY = height() / 8.f; 
    boardEntityDrawingRoutine(painter, renderQueueLower, scalingFactorX, scalingFactorY);
};

void BoardWidget::drawExtras(QPainter & painter) {
    float scalingFactorX = width() / 8.f;
    float scalingFactorY = height() / 8.f; 
    boardEntityDrawingRoutine(painter, renderQueueUpper, scalingFactorX, scalingFactorY);

    if (promotionContextMenu)
        promotionContextMenu->draw(painter, this->size());
};

void BoardWidget::mousePressEvent(QMouseEvent * event) {
    QPointF pos = event->position();
    mouseDragDetector.buttonPressed(pos);
    if (promotionContextMenu)
        promotionContextMenu->onMousePressed(event->position(), this->size());
};

void BoardWidget::mouseReleaseEvent(QMouseEvent * event) {
    QPointF pos = event->position();
    mouseDragDetector.buttonReleased(pos);
};

Board::BoardWidget::DragHandleImpl::DragHandleImpl(BoardWidget & context, std::shared_ptr<Internal::DragInfo> dragInfo) :
    DragHandle<BoardWidget>(context, dragInfo){};

void Board::BoardWidget::DragHandleImpl::reject(){
    BoardEntity * e = getEntity();
    std::lock_guard<std::mutex> guard(info->lock);
    e->setProperty("location", info->startingLocation);
    info->closed = true;
};

void Board::BoardWidget::DragHandleImpl::accept() {
    BoardEntity * e = getEntity();
    close();
};

BoardEntity *  Board::BoardWidget::DragHandleImpl::getEntity() {
    std::lock_guard<std::mutex> guard(info->lock);
    return context->entities.findByIdentifier(info->entityId);
}

QPointF BoardWidget::calculateDraggedPieceNormalisedLocation(QPointF rawMousePoint) {
    qreal x = 8.f * rawMousePoint.x() / width();
    qreal y = 8.f * rawMousePoint.y() / height();

    if (currentlyDraggedPiece == nullptr)
        return QPointF(x, y);
    
    std::lock_guard<std::mutex> guard(currentlyDraggedPiece->lock);

    BoardEntity * entity  = entities.findByIdentifier(currentlyDraggedPiece->entityId);

    if (entity == nullptr)
        return QPointF(x, y);

    QSizeF entitySize = entity->property("size").toSizeF();

    x -= entitySize.width() / 2;
    y -= entitySize.height() / 2;
    
    return QPointF(x, y);
};

BoardEntity * BoardWidget::trySelectDraggedEntity(QPointF rawMousePoint) {
    qreal x = 8.f * rawMousePoint.x() / width();
    qreal y = 8.f * rawMousePoint.y() / height();

    for (Board::BoardEntity & e : entities) {
        qreal eX = e.property("location").toPointF().x();
        qreal eY = e.property("location").toPointF().y();
        qreal eW = e.property("size").toSizeF().width();
        qreal eH = e.property("size").toSizeF().height();
        if (e.property("draggable").toBool() && eX <= x && x < eX + eW && eY <= y && y < eY + eH)
            return &e;
    }   

    return nullptr;
};

void BoardWidget::tryStartDragAction(QPointF rawMousePoint) {
    BoardEntity * draggedEntity = trySelectDraggedEntity(rawMousePoint);

    if(draggedEntity == nullptr)
        return;

    (void)startDrag(draggedEntity->getIdentifier());
    draggedEntity->setProperty("z_level", 100);
};

 void BoardWidget::tryHandleDragActionEnd(QPointF rawMousePoint) {
    if (currentlyDraggedPiece == nullptr) 
        return;
    DragHandle<BoardWidget> * dragHandle = new DragHandleImpl(*this,currentlyDraggedPiece);
    BoardEntity * draggedEntity = dragHandle->getEntity(); 
    if (draggedEntity ) 
        draggedEntity->setProperty("z_level", currentlyDraggedPiece->startingZLevel);
    if(dragHandle->isClosed()) 
        goto cleanup;
    onDrag(*dragHandle);
    cleanup:
    delete dragHandle;
 };

void BoardWidget::onClickAction(QPointF rawMousePoint) {
    ClickHandle click(this, rawMousePoint);
    onClick(click);
};


void BoardWidget::mouseMoveEvent(QMouseEvent * event) {
    BoardGridWidget::mouseMoveEvent(event);
    mouseDragDetector.mouseMoved(event->position());
    DragArea::updateDragAnimation(calculateDraggedPieceNormalisedLocation(event->position()));

    if (promotionContextMenu)
        promotionContextMenu->onMouseMoved(event->position(), this->size());

    if(currentlyDraggedPiece == nullptr)
        return;
    std::lock_guard<std::mutex> guard(currentlyDraggedPiece->lock);
    if(!currentlyDraggedPiece->closed)
        update();
};  

DragHandle<BoardWidget> * BoardWidget::startDrag(unsigned int EntityId) {
    auto resultInfo = std::make_shared<Internal::DragInfo>();
    BoardEntity * e = entities.findByIdentifier(EntityId);

    if (e == nullptr) return nullptr;

    resultInfo->closed = resultInfo->paused = false;
    resultInfo->currentLocation = resultInfo->startingLocation = e->property("location").toPointF();
    resultInfo->startingZLevel = e->property("z_level").toInt();
    resultInfo->entityId = EntityId;
    
    DragArea::currentlyDraggedPiece = resultInfo;
    e->setLocationAnimation(std::move(DragArea::getDragAnimation(resultInfo)));
    return new DragHandleImpl(*this, resultInfo);
};

PromotionContextMenuOverlay & BoardWidget::openPromotionContextMenu() {
    closePromotionContextMenu();

    promotionContextMenu = new PromotionContextMenuOverlay();
    connect(promotionContextMenu, &PromotionContextMenuOverlay::onOverlayUpdate, this, [this](){this->update();});
    return *promotionContextMenu;
};

void BoardWidget::closePromotionContextMenu() {
    if (promotionContextMenu)
        delete promotionContextMenu;
};


#include <moc_BoardGridWidget.cpp>