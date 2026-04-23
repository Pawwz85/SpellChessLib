#pragma once
#include "BoardTypes.hpp"

#include <QObject>
#include <QPainter>
#include <QPointF>
#include <QRectF>

    namespace Board {
    
    /*!
        PromotionContextMenuStyle class is responsible for defining how promotion context menu overlay is being drawn.
        In order to customise how Promotion Context menu overlay is being drawn simply derive from this class. 
    */
    class PromotionContextMenuStyle {
        public:
        virtual ~PromotionContextMenuStyle() = default;    
        virtual void drawOutsideContainer(QPainter & painter, QRectF where) const = 0;
        virtual void drawPieceTargetContainer(QPainter & painter, QRectF where, bool higlighted) const = 0;
        virtual void drawPieceIcon(QPainter & painter, Board::PawnPromotionAction action, Board::PieceColor color, QRectF where) const = 0; 
    };

    extern const PromotionContextMenuStyle * defaultPromotionContextMenuStyle;

    class PromotionContextMenuOverlay : public QObject {
        Q_OBJECT

        const PromotionContextMenuStyle * style; 
        QPointF overlayPosition; // relative to size of a board. Value of 1.f is distant of 1 board square
        QSizeF overlaySize; // relative size to board. Value of (1.f, 1.f) is size of 1 board square
        Board::PieceColor pieceColor;

        unsigned int highlightedActionIndex = (unsigned int) -1;

        const PawnPromotionAction actions[4] = {
            PawnPromotionAction::PromoteToQueen,
            PawnPromotionAction::PromoteToRook,
            PawnPromotionAction::PromoteToKnight,
            PawnPromotionAction::PromoteToBishop
        };

        public:
        PromotionContextMenuOverlay()   : style(defaultPromotionContextMenuStyle), overlayPosition(0, 0), overlaySize(1.f, 4.f), pieceColor(Board::White) {};
        
        void setStyle(const PromotionContextMenuStyle * style);
        void setPosition(QPointF location);
        void setSize(QSizeF size);
        void setPiecesColor(PieceColor color);

        void draw(QPainter & painter, QSizeF boardSize);
        void onMouseMoved(QPointF position, QSizeF boardSize);
        void onMousePressed(QPointF position, QSizeF boardSize);

        signals:
        void onActionSelected(Board::PawnPromotionAction action); 
        void onOverlayUpdate();
    };

}