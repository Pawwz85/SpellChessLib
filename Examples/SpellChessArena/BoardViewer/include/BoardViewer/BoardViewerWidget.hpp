#pragma once

#include <QWidget>
#include <QGridLayout>

#include <BoardGrid/BoardGridWidget.h>

#include "BoardViewerModel.hpp"
#include "UserInteractionManager.hpp"
#include "SpellChessMoveBuilder.hpp"
#include "PlayerTimer.hpp"
#include "MoveList.hpp"
#include "BoardLinter.hpp"
#include "SpellPanel.hpp"

namespace BoardViewer {

    class BoardViewerWidget : public QWidget {

        class NoFocus;
        class PromotionPicker;
        class SpellSquarePicker;

        Q_OBJECT
        std::shared_ptr<UserInteractionManager> interactionManager;
        std::shared_ptr<SpellChessMoveBuilder> moveBuilder;

        Board::BoardWidget * board;
        Model * model;
        MoveListView * moveList;

        PlayerTimer * whiteTimer;
        PlayerTimer * blackTimer;


        QGridLayout * layout;

        SpellPanelModel whiteSpellPanel;
        SpellPanelModel blackSpellPanel;

        void connectSlots();
        LinterInput prepareInfoForLinter() const;

        QPointF pointFromSquareIndex(unsigned int sqId) const;

        void updateLinter();
        void keyPressEvent(QKeyEvent * event) override;

        protected slots:
        void handleDrag(Board::DragHandle<Board::BoardWidget> & drag);
        void handleClick(Board::BoardWidget::ClickHandle & click);

        void handleGamestateChange(const SpellChessLib::GameState & state);
        void handleMoveListChanged(const std::vector<SpellChessLib::SpellChessMove> & moves);
        void handleModelChanged(const Model & model);
        void handleDocumentChanged(const SpellChessLib::SpellChessDocument & document);
        void handleTimePairChanged(const SpellChessLib::TimerPairState & state);
        void handleGameRunnerIdChanged(int id) {};
        void handlePromotionActionSelected(Board::PawnPromotionAction action);
        void handleSpellSelect(SpellChessLib::SpellIdentificator spell);
        void handleSpellDiscarded();
        void handleNodeClicked(int nodeId);
        void handleMoveListNav(MoveListView::NavigationDirective directive);
        void handleCommentEdit(const QString & newComment);

        public:

        BoardViewerWidget(Model * model, QWidget * parent = nullptr);
        ~BoardViewerWidget();
    };

}

