#pragma once

#include <vector>
#include <memory>
#include <thread>

#include <QCoreApplication>
#include <QWidget>
#include <QGridLayout>


#include <SpellChess/Board.h>
#include <SpellChess/GameRunner.h>
#include <SpellChess/GameAnnotations.h>

#include <BoardGrid/BoardGridWidget.h>


#include <GameSession/GameRunnerBridge.hpp>
#include <GameSession/SpellChessMoveBuilder.hpp>
#include <GameSession/UserInteractionManager.hpp>
#include <GameSession/BoardLinter.hpp>
#include <GameSession/OpenSessionRequest.hpp>

#include <SpellPanel.hpp> 
#include <MoveList.hpp>
#include <PlayerTimer.hpp>

namespace GameSession {

    class GameSessionModel : public QObject {
        Q_OBJECT
        public: 
        
        // this enum dictates how board component and linter should work
        enum UserFunction {
            Observing,
            WhitePlayer,
            BlackPlayer,
            SelfPlay
        };

        const UserFunction userFunction;

        private:

        using PlayerPointer = typename SpellChessLib::Scheduler::ResourcePointer<SpellChessLib::Player>;

        QCoreApplication * qApplication;
        GameRunnerBridge runnerBridge;
        
        SpellChessLib::GameState gameState;
        SpellChessLib::GameState rootState;
        SpellChessLib::TimerPairState pairState;

        std::vector<SpellChessLib::SpellChessMove> moveList;
        
        std::shared_ptr<SpellChessLib::GameRunner> gameRunner;

        std::thread* gameThread;

        int gameRunnerId;


        bool event(QEvent * e) override;

        static UserFunction userFunctionFromRequest(const OpenSessionRequest & request);
        PlayerPointer playerFromDef(const GameSession::PlayerDefinition & def, const OpenSessionRequest & request);

        public:
        GameSessionModel(std::shared_ptr<SpellChessLib::TimerService> timeServ, const OpenSessionRequest & request,  QCoreApplication * app);
        ~GameSessionModel();
        
        bool submitMove(const SpellChessLib::SpellChessMove & move);

        const SpellChessLib::GameState & getCurrentState() const;
        const SpellChessLib::GameState & getInitialState() const;
        const SpellChessLib::TimerPairState & getTimerPairState() const;

        const std::vector<SpellChessLib::SpellChessMove> & getMoveList() const;        
        int getGameRunnerId() const;

        

        signals:
        void onGameStateChanged(const SpellChessLib::GameState & state);
        void onMoveListChanged(const std::vector<SpellChessLib::SpellChessMove> & moves);
        void onGameRunnerIdChanged(int id);
        void onTimerPairChanged(const SpellChessLib::TimerPairState & state);

    };

    namespace States {
        class NoFocus;
        class PromotionPicker;
        class SpellSquarePicker;
    };

    class GameSessionView : public QWidget {
        friend States::NoFocus;
        friend States::PromotionPicker;
        friend States::SpellSquarePicker;

        Q_OBJECT
        std::shared_ptr<GameSession::UserInteractionManager> interactionManager;
        std::shared_ptr<GameSession::SpellChessMoveBuilder> moveBuilder;
        std::shared_ptr<MoveListModel> moveListModel;

        Board::BoardWidget * board;
        GameSessionModel * model;
        MoveListView * moveList;

        PlayerTimer * whiteTimer;
        PlayerTimer * blackTimer;


        QGridLayout * layout;

        SpellPanelModel whiteSpellPanel;
        SpellPanelModel blackSpellPanel;


        void connectSlots();
        GameSession::LinterInput prepareInfoForLinter() const;

        QPointF pointFromSquareIndex(unsigned int sqId) const;

        void updateLinter();

        protected slots:
        void handleDrag(Board::DragHandle<Board::BoardWidget> & drag);
        void handleClick(Board::BoardWidget::ClickHandle & click);
        void handleGamestateChange(const SpellChessLib::GameState & state);
        void handleMoveListChanged(const std::vector<SpellChessLib::SpellChessMove> & moves);
        void handleTimePairChanged(const SpellChessLib::TimerPairState & state);
        void handleGameRunnerIdChanged(int id) {};
        void handlePromotionActionSelected(Board::PawnPromotionAction action);
        void handleSpellSelect(SpellChessLib::SpellIdentifator spell);
        void handleSpellDiscarded();


        public:

        GameSessionView(GameSessionModel * model, QWidget * parent = nullptr);
        ~GameSessionView();
    };

    namespace States {

        class NoFocus : public UserInteractionState {
            GameSessionView * view;

            void onSpellSelected(SpellChessLib::SpellIdentifator spellId) override;
            void onSpellDiscarded() override;
            void onEmptySquareClicked(SpellChessLib::SquareIndex squareIndex) override;
            void onPieceClicked(SpellChessLib::SquareIndex pieceSquare, SpellChessLib::Piece piece) override;
            void onPieceTargetSquareClicked(SpellChessLib::SquareIndex targetSquare) override;
            void onPieceDrag(Board::DragHandle<Board::BoardWidget> & drag) override;

            bool isPromotionMove(SpellChessLib::SquareIndex targetSquare) const;
            void openPromotionContextMenu(SpellChessLib::SquareIndex anchorSquare);
            public:

            NoFocus(GameSessionView * view, std::weak_ptr<UserInteractionManager> manager) : view(view), UserInteractionState(manager) {};
        };

        class PromotionPicker : public UserInteractionState {
            GameSessionView * view;

            void discard();

            void onSpellSelected(SpellChessLib::SpellIdentifator spellId) override {discard();};
            void onSpellDiscarded() override {discard();};   
            void onEmptySquareClicked(SpellChessLib::SquareIndex squareIndex) override {discard();};
            void onPieceClicked(SpellChessLib::SquareIndex pieceSquare, SpellChessLib::Piece piece) override {discard();}
            void onPieceTargetSquareClicked(SpellChessLib::SquareIndex targetSquare) override {discard();};
            void onPieceDrag(Board::DragHandle<Board::BoardWidget> & drag) override {discard();};

            public:
            PromotionPicker(SpellChessLib::SquareIndex anchorIndex, GameSessionView * view, std::weak_ptr<UserInteractionManager> manager);
        };

        class SpellSquarePicker : public UserInteractionState {
            GameSessionView * view;
            SpellChessLib::SpellIdentifator spellId;

            void discard();

            void onSpellSelected(SpellChessLib::SpellIdentifator spellId) override;
            void onSpellDiscarded() override {discard();};   
            void onEmptySquareClicked(SpellChessLib::SquareIndex squareIndex) override;
            void onPieceClicked(SpellChessLib::SquareIndex pieceSquare, SpellChessLib::Piece piece) override {onEmptySquareClicked(pieceSquare);}
            void onPieceTargetSquareClicked(SpellChessLib::SquareIndex targetSquare) override {discard();};
            void onPieceDrag(Board::DragHandle<Board::BoardWidget> & drag) override {discard();};

            public:
            SpellSquarePicker(SpellChessLib::SpellIdentifator spell, GameSessionView * view, std::weak_ptr<UserInteractionManager> manager);

        };
    };

} // namespace GameSession
