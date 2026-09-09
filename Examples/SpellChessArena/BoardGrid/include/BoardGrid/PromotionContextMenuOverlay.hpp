#pragma once
#include "BoardTypes.hpp"
#include "CoordinateSystem.h"

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
        LogicalBoardPoint overlayPosition;  
        LogicalBoardSize overlaySize; 
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
        void setPosition(LogicalBoardPoint location);
        void setSize(LogicalBoardSize size);
        void setPiecesColor(PieceColor color);

        void draw(QPainter & painter, BoardWidgetSize boardSize, BoardOrientation orientation);
        void onMouseMoved(LogicalBoardPoint position);
        void onMousePressed(LogicalBoardPoint position);

        signals:
        void onActionSelected(Board::PawnPromotionAction action); 
        void onOverlayUpdate();
    };

}