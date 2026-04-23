#include <PromotionContextMenuOverlay.hpp>
#include <PieceSpriteSet.hpp>
#include <QDebug>
#include <map>

void Board::PromotionContextMenuOverlay::setStyle(const PromotionContextMenuStyle * style) {
    this->style = style;
}

void Board::PromotionContextMenuOverlay::setPosition(QPointF location) {
    this->overlayPosition = location;
};

void Board::PromotionContextMenuOverlay::setSize(QSizeF size) {
    this->overlaySize = size;
}

void  Board::PromotionContextMenuOverlay::setPiecesColor(PieceColor color) {
    this->pieceColor = color;
};


void Board::PromotionContextMenuOverlay::draw(QPainter & painter, QSizeF boardSize) {

    if (style == nullptr) {
        qDebug() << "Can not render a promotiom context menu overlay without any style";
        return;
    };
    
    qreal overlayLeft   =  overlayPosition.x() * boardSize.width()  / 8.f; 
    qreal overlayTop    =  overlayPosition.y() * boardSize.height() / 8.f;  
    qreal overlayWidth  =  overlaySize.width() * boardSize.width()  / 8.f;
    qreal overlayHeight =  overlaySize.height()* boardSize.height() / 8.f;

    style->drawOutsideContainer(painter, QRectF(overlayLeft, overlayTop, overlayWidth, overlayHeight));

    const qreal containerHeight = overlayHeight / 4.f;
    for (unsigned int actionIndex = 0; actionIndex < 4; ++actionIndex) {
        QRectF currentContainer(overlayLeft, overlayTop + actionIndex * containerHeight, overlayWidth, containerHeight);
        
        style->drawPieceTargetContainer(painter, 
            currentContainer,
            actionIndex == highlightedActionIndex);
    
        style->drawPieceIcon(painter, actions[actionIndex], pieceColor, currentContainer);
    };

};

void Board::PromotionContextMenuOverlay::onMouseMoved(QPointF position, QSizeF boardSize) {

    qreal overlayLeft   =  overlayPosition.x() * boardSize.width()  / 8.f; 
    qreal overlayTop    =  overlayPosition.y() * boardSize.height() / 8.f;  
    qreal overlayWidth  =  overlaySize.width() * boardSize.width()  / 8.f;
    qreal overlayHeight =  overlaySize.height()* boardSize.height() / 8.f;

    unsigned int oldIndex = highlightedActionIndex;

    if (    position.x() < overlayLeft ||
            position.y() < overlayTop ||
            overlayLeft + overlayWidth < position.x() ||
            overlayTop + overlayHeight < position.y()
        ) {
            highlightedActionIndex = (unsigned int) -1;
            if (oldIndex != highlightedActionIndex)
                onOverlayUpdate();
            return;
    };

    int selectedIndex = qFloor((position.y() - overlayTop) / overlayHeight * 4); 

    if ( selectedIndex < 4 && selectedIndex >= 0 ) {
        highlightedActionIndex = (unsigned int) selectedIndex;
    } else {
        highlightedActionIndex = (unsigned int) -1;
    }

    exit:

    if (oldIndex != highlightedActionIndex)
        onOverlayUpdate();
};

void Board::PromotionContextMenuOverlay::onMousePressed(QPointF position, QSizeF boardSize) {
    qreal overlayLeft   =  overlayPosition.x() * boardSize.width()  / 8.f; 
    qreal overlayTop    =  overlayPosition.y() * boardSize.height() / 8.f;  
    qreal overlayWidth  =  overlaySize.width() * boardSize.width()  / 8.f;
    qreal overlayHeight =  overlaySize.height()* boardSize.height() / 8.f;

      if (  position.x() < overlayLeft ||
            position.y() < overlayTop ||
            overlayLeft + overlayWidth < position.x() ||
            overlayTop + overlayHeight < position.y()
        ) 
        return;

    int selectedIndex = qFloor((position.y() - overlayTop) / overlayHeight * 4); 
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