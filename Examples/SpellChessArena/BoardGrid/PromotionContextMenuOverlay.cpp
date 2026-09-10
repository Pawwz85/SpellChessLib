#include <PromotionContextMenuOverlay.hpp>
#include <PieceSpriteSet.hpp>
#include <QDebug>
#include <map>

void Board::PromotionContextMenuOverlay::setStyle(const PromotionContextMenuStyle * style) {
    this->style = style;
}

void Board::PromotionContextMenuOverlay::setPosition(LogicalBoardPoint location) {
    this->overlayPosition = location;
};

void Board::PromotionContextMenuOverlay::setSize(LogicalBoardSize size) {
    this->overlaySize = size;
}

void  Board::PromotionContextMenuOverlay::setPiecesColor(PieceColor color) {
    this->pieceColor = color;
};

Board::BoardWidgetRect Board::PromotionContextMenuOverlay::widgetBoundingRect(
    BoardWidgetSize boardSize,
    BoardOrientation orientation) const {
    return toWidgetCoordinates(
        LogicalBoardRect{overlayPosition, overlaySize},
        boardSize,
        orientation);
}

Board::BoardWidgetRect Board::PromotionContextMenuOverlay::actionBoundingRect(
    PromotionActionIndex actionIndex,
    BoardWidgetSize boardSize,
    BoardOrientation orientation) const {
    const BoardWidgetRect boundingRect = widgetBoundingRect(boardSize, orientation);
    const BoardWidgetSize actionSize(boundingRect.width(), boundingRect.height() / 4.f);
    const auto actionNumber = static_cast<unsigned int>(actionIndex);
    const BoardWidgetPoint actionPosition(
        boundingRect.x(),
        boundingRect.top() + (static_cast<float>(actionNumber) + 0.5f) * actionSize.height());

    return BoardWidgetRect(actionPosition, actionSize);
}

std::optional<Board::PromotionActionIndex> Board::PromotionContextMenuOverlay::actionIndexFromWidgetPoint(
    BoardWidgetPoint point,
    BoardWidgetSize boardSize,
    BoardOrientation orientation) const {
    const BoardWidgetRect boundingRect = widgetBoundingRect(boardSize, orientation);
    if (boundingRect.width() <= 0.f || boundingRect.height() <= 0.f ||
        point.x() < boundingRect.left() || point.x() >= boundingRect.right() ||
        point.y() < boundingRect.top() || point.y() >= boundingRect.down()) {
        return std::nullopt;
    }

    const auto row = static_cast<unsigned int>(
        (point.y() - boundingRect.top()) / boundingRect.height() * 4.f);
    if (row >= 4)
        return std::nullopt;

    return static_cast<PromotionActionIndex>(row);
}

std::optional<Board::PromotionActionIndex> Board::PromotionContextMenuOverlay::actionIndexFromLogicalPoint(
    LogicalBoardPoint point,
    BoardWidgetSize boardSize,
    BoardOrientation orientation) const {
    return actionIndexFromWidgetPoint(
        toWidgetCoordinates(point, boardSize, orientation),
        boardSize,
        orientation);
}

void Board::PromotionContextMenuOverlay::draw(QPainter & painter, BoardWidgetSize boardSize, BoardOrientation orientation) {

    if (style == nullptr) {
        qDebug() << "Can not render a promotiom context menu overlay without any style";
        return;
    };
    
    const BoardWidgetRect boundingRect = widgetBoundingRect(boardSize, orientation);

    style->drawOutsideContainer(painter, QRectF(boundingRect.left() , boundingRect.top(), boundingRect.width(), boundingRect.height()));

    for (const auto actionIndex : {
             PromotionActionIndex::Queen,
             PromotionActionIndex::Rook,
             PromotionActionIndex::Knight,
             PromotionActionIndex::Bishop }) {
        const BoardWidgetRect currentRect = actionBoundingRect(actionIndex, boardSize, orientation);
        const QRectF currentContainer(currentRect.left(), currentRect.top(), currentRect.width(), currentRect.height());
        
        style->drawPieceTargetContainer(painter, 
            currentContainer,
            highlightedActionIndex == actionIndex);
    
        style->drawPieceIcon(painter, actions[static_cast<unsigned int>(actionIndex)], pieceColor, currentContainer);
    };

};

void Board::PromotionContextMenuOverlay::onMouseMoved(
    BoardWidgetPoint position,
    BoardWidgetSize boardSize,
    BoardOrientation orientation) {
    const auto oldIndex = highlightedActionIndex;
    const auto selectedIndex = actionIndexFromWidgetPoint(position, boardSize, orientation);
    highlightedActionIndex = selectedIndex;

    if (oldIndex != highlightedActionIndex)
        onOverlayUpdate();
};

void Board::PromotionContextMenuOverlay::onMousePressed(
    BoardWidgetPoint position,
    BoardWidgetSize boardSize,
    BoardOrientation orientation) {
    const auto selectedIndex = actionIndexFromWidgetPoint(position, boardSize, orientation);
    if (selectedIndex.has_value())
        onActionSelected(actions[static_cast<unsigned int>(*selectedIndex)]);
};

class DefaultPromotionContextMenuStyle : public Board::PromotionContextMenuStyle {
    public:
        void drawOutsideContainer(QPainter & painter, QRectF where) const override {
            painter.save();
            painter.setBrush(Qt::GlobalColor::white);
            painter.drawRect(where);
            painter.restore();
        };
        void drawPieceTargetContainer(QPainter & painter, QRectF where, bool higlighted) const override {
            if (higlighted) {
                painter.save();
                painter.setBrush(QColor(QRgb(0xD7FF61)));
                painter.drawRect(where);
                painter.restore();
            }
        };
        void drawPieceIcon(QPainter & painter, Board::PawnPromotionAction action, Board::PieceColor color, QRectF where) const override {
            static std::map<Board::PawnPromotionAction, const char *> actionToPiece = {
                {Board::PromoteToQueen, "queen"},
                {Board::PromoteToRook,  "rook"},
                {Board::PromoteToBishop, "bishop"},
                {Board::PromoteToKnight, "knight"},
            };

            QString spriteId = QString(color == Board::White ? "white_" : "black_") + actionToPiece[action];
            painter.drawPixmap(where.toRect(), *Board::PieceSpriteSet::getInstance().getSprite(spriteId));
        };      
};

DefaultPromotionContextMenuStyle defaultPromotionContextMenuStyleInstance;
const Board::PromotionContextMenuStyle * Board::defaultPromotionContextMenuStyle = &defaultPromotionContextMenuStyleInstance;

#include <moc_PromotionContextMenuOverlay.cpp>