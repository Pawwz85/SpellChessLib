#pragma once
#include <QWidget>
#include <QColor>
#include <vector>
#include <memory>
#include <functional>
#include <QPainter>
#include "BoardGridModel.h"
#include "BoardEntity.h"
#include "DragArea.hpp"
#include "MouseDragDetector.hpp"
#include "PromotionContextMenuOverlay.hpp"

namespace Board {

    class BoardGridWidget : public QWidget {
        Q_OBJECT
        QColor pickColorForSquare(unsigned int index);
        void resetHiglight();

        protected:
        virtual void drawPieces(QPainter & painter) {}; 
        virtual void drawExtras(QPainter & painter) {};

        QRect getSquareBoundingRect(int col, int row);

        public:
        BoardGridModel boardGridModel;
        BoardGridWidget(QWidget * parent = nullptr);
        ~BoardGridWidget() = default;

        void paintEvent(QPaintEvent * event) override;
        void mouseMoveEvent(QMouseEvent* event) override;
        QSize sizeHint() const override;
        BoardOrientation orientation() const;

        public slots:
        void onModelChanged();
    };

    class BoardWidget : public BoardGridWidget, public DragArea<BoardWidget> {
        Q_OBJECT

        public:

        class ClickHandle {
            BoardWidget * context;
            LogicalBoardPoint where;
            public:
            ClickHandle(BoardWidget * cntx, LogicalBoardPoint point) : context(cntx), where(point) {};
            BoardWidget & getContext() {return *context;};
            const BoardWidget & getContext() const {return *context;};
            const LogicalBoardPoint getPosition() const {return where;};
        };

        private:    
        MouseDragDetector mouseDragDetector;
        
        // rendering
        std::vector<const BoardEntity *> renderQueueLower;
        std::vector<const BoardEntity *> renderQueueUpper;
        void boardEntityDrawingRoutine( QPainter & painter,
                                const std::vector<const BoardEntity *> & renderQueue);

        class DragHandleImpl : public DragHandle<BoardWidget> {
            void reject() override;
            void accept() override;
            BoardEntity * getEntity() override;
            public:
            DragHandleImpl(BoardWidget & context, std::shared_ptr<Internal::DragInfo> dragInfo);
        };
        DragHandle<BoardWidget> * startDrag(unsigned int EntityId) override;
        BoardEntity * trySelectDraggedEntity(BoardWidgetPoint rawMousePoint);

        PromotionContextMenuOverlay * promotionContextMenu = nullptr;

        private slots: 
        void tryStartDragAction(BoardWidgetPoint rawMousePoint);
        void tryHandleDragActionEnd(BoardWidgetPoint rawMousePoint);
        void onClickAction(BoardWidgetPoint rawMousePoint);

        protected:
        // inherited from BoardGridWidget
        void drawPieces(QPainter & painter) override;
        void drawExtras(QPainter & painter) override;

        public:
        BoardWidget(QWidget * parent = nullptr) : BoardGridWidget(parent) {
            connect(&mouseDragDetector, &MouseDragDetector::onDragStarted, this, &BoardWidget::tryStartDragAction);
            connect(&mouseDragDetector, &MouseDragDetector::onDragEnded, this, &BoardWidget::tryHandleDragActionEnd);
            connect(&mouseDragDetector, &MouseDragDetector::onClick, this, &BoardWidget::onClickAction); 
        };
        virtual ~BoardWidget() = default;

        void mousePressEvent(QMouseEvent * event) override;
        void mouseReleaseEvent(QMouseEvent * event) override;
        void mouseMoveEvent(QMouseEvent * event) override;
        void paintEvent(QPaintEvent * event) override;
        
        unsigned int squareIndexFromBoardPos(LogicalBoardPoint pos) const;
        bool setOrientation(BoardOrientation orientation);
        bool toggleOrientation();

        BoardWidgetSize getBoardSize() const;

        BoardEntityList entities;

        void closePromotionContextMenu();
        PromotionContextMenuOverlay & openPromotionContextMenu();

        signals:
        void onDrag(Board::DragHandle<BoardWidget> & drag);
        void onDragStarted(Board::BoardWidget::ClickHandle & click);
        void onClick(Board::BoardWidget::ClickHandle & click);
    };
};