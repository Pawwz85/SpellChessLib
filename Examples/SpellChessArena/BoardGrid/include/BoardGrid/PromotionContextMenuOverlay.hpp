#pragma once
#include "BoardTypes.hpp"
#include "CoordinateSystem.h"

#include <QObject>
#include <QPainter>
#include <QRectF>

#include <array>
#include <optional>

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

    enum class PromotionActionIndex : unsigned int {
        Queen = 0,
        Rook = 1,
        Knight = 2,
        Bishop = 3
    };

    class PromotionContextMenuOverlay : public QObject {
        Q_OBJECT

        const PromotionContextMenuStyle * style; 
        LogicalBoardPoint overlayPosition;  
        LogicalBoardSize overlaySize; 
        Board::PieceColor pieceColor;

        std::optional<PromotionActionIndex> highlightedActionIndex;

        const std::array<PawnPromotionAction, 4> actions = {
            PawnPromotionAction::PromoteToQueen,
            PawnPromotionAction::PromoteToRook,
            PawnPromotionAction::PromoteToKnight,
            PawnPromotionAction::PromoteToBishop
        };

        BoardWidgetRect widgetBoundingRect(BoardWidgetSize boardSize, BoardOrientation orientation) const;
        BoardWidgetRect actionBoundingRect(PromotionActionIndex actionIndex,
                                            BoardWidgetSize boardSize,
                                            BoardOrientation orientation) const;

        public:
        PromotionContextMenuOverlay()   : style(defaultPromotionContextMenuStyle), overlayPosition(0, 0), overlaySize(1.f, 4.f), pieceColor(Board::White) {};
        
        void setStyle(const PromotionContextMenuStyle * style);
        void setPosition(LogicalBoardPoint location);
        void setSize(LogicalBoardSize size);
        void setPiecesColor(PieceColor color);

        std::optional<PromotionActionIndex> actionIndexFromWidgetPoint(
            BoardWidgetPoint point,
            BoardWidgetSize boardSize,
            BoardOrientation orientation) const;
        std::optional<PromotionActionIndex> actionIndexFromLogicalPoint(
            LogicalBoardPoint point,
            BoardWidgetSize boardSize,
            BoardOrientation orientation) const;

        void draw(QPainter & painter, BoardWidgetSize boardSize, BoardOrientation orientation);
        void onMouseMoved(BoardWidgetPoint position, BoardWidgetSize boardSize, BoardOrientation orientation);
        void onMousePressed(BoardWidgetPoint position, BoardWidgetSize boardSize, BoardOrientation orientation);

        signals:
        void onActionSelected(Board::PawnPromotionAction action); 
        void onOverlayUpdate();
    };

}