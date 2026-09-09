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


void Board::PromotionContextMenuOverlay::draw(QPainter & painter, BoardWidgetSize boardSize, BoardOrientation orientation) {

    if (style == nullptr) {
        qDebug() << "Can not render a promotiom context menu overlay without any style";
        return;
    };
    
    BoardWidgetRect boundingRect = toWidgetCoordinates(LogicalBoardRect{overlayPosition, overlaySize}, boardSize, orientation);

    style->drawOutsideContainer(painter, QRectF(boundingRect.left() , boundingRect.top(), boundingRect.width(), boundingRect.height()));

    const qreal containerHeight = boundingRect.height() / 4.f;
    for (unsigned int actionIndex = 0; actionIndex < 4; ++actionIndex) {
        QRectF currentContainer(boundingRect.left(), boundingRect.top()  + actionIndex * containerHeight, boundingRect.width(), containerHeight);
        
        style->drawPieceTargetContainer(painter, 
            currentContainer,
            actionIndex == highlightedActionIndex);
    
        style->drawPieceIcon(painter, actions[actionIndex], pieceColor, currentContainer);
    };

};

void Board::PromotionContextMenuOverlay::onMouseMoved(LogicalBoardPoint position) {

    LogicalBoardRect boundingRect = LogicalBoardRect(overlayPosition, overlaySize); 

    unsigned int oldIndex = highlightedActionIndex;

    if (    position.x() < boundingRect.left() ||
            position.y() < boundingRect.top()  ||
            position.x() > boundingRect.right()  ||
            position.y() > boundingRect.down()
        ) {
            highlightedActionIndex = (unsigned int) -1;
            if (oldIndex != highlightedActionIndex)
                onOverlayUpdate();
            return;
    };

    int selectedIndex = qFloor((position.y() - boundingRect.top()) / boundingRect.height() * 4); 

    if ( selectedIndex < 4 && selectedIndex >= 0 ) {
        highlightedActionIndex = (unsigned int) selectedIndex;
    } else {
        highlightedActionIndex = (unsigned int) -1;
    }

    exit:

    if (oldIndex != highlightedActionIndex)
        onOverlayUpdate();
};

void Board::PromotionContextMenuOverlay::onMousePressed(LogicalBoardPoint position) {
    LogicalBoardRect boundingRect{overlayPosition, overlaySize};

    if (    position.x() < boundingRect.left() ||
            position.y() < boundingRect.top()  ||
            position.x() > boundingRect.right()  ||
            position.y() > boundingRect.down()
        )
        return;

   int selectedIndex = qFloor((position.y() - boundingRect.top()) / boundingRect.height() * 4); 
    if ( selectedIndex < 4 && selectedIndex >= 0 )
        onActionSelected(actions[selectedIndex]);
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