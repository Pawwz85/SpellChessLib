#pragma once

#include <vector>
#include <memory>
#include <thread>

#include <QCoreApplication>
#include <QWidget>
#include <QGridLayout>


#include <SpellChess/Board.h>
#include <SpellChess/GameRunner.h>

#include <BoardGrid/BoardGridWidget.h>


#include <GameSession/GameRunnerBridge.hpp>
#include <GameSession/SpellChessMoveBuilder.hpp>
#include <GameSession/UserInteractionManager.hpp>
#include <GameSession/BoardLinter.hpp>
#include <GameSession/OpenSessionRequest.hpp>

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
        std::vector<SpellChessLib::SpellChessMove> moveList;
        
        std::shared_ptr<SpellChessLib::GameRunner> gameRunner;

        std::thread* gameThread;

        int gameRunnerId;


        bool event(QEvent * e) override;

        static UserFunction userFunctionFromRequest(const OpenSessionRequest & request);
        PlayerPointer playerFromDef(const GameSession::PlayerDefinition & def);

        public:
        GameSessionModel(std::shared_ptr<SpellChessLib::TimerService> timeServ, const OpenSessionRequest & request,  QCoreApplication * app);
        ~GameSessionModel();
        
        bool submitMove(const SpellChessLib::SpellChessMove & move);

        const SpellChessLib::GameState & getCurrentState() const;
        const std::vector<SpellChessLib::SpellChessMove> & getMoveList() const;        
        int getGameRunnerId() const;

        

        signals:
        void onGameStateChanged(const SpellChessLib::GameState & state);
        void onMoveListChanged(const std::vector<SpellChessLib::SpellChessMove> & moves);
        void onGameRunnerIdChanged(int id);
    };

    class GameSessionView : public QWidget {
        Q_OBJECT
        std::shared_ptr<GameSession::UserInteractionManager> interactionManager;
        std::shared_ptr<GameSession::SpellChessMoveBuilder> moveBuilder;

        Board::BoardWidget * board;
        GameSessionModel * model;

        QGridLayout * layout;

        void connectSlots();
        GameSession::LinterInput prepareInfoForLinter() const;

        QPointF pointFromSquareIndex(unsigned int sqId) const;

        protected slots:
        void handleDrag(Board::DragHandle<Board::BoardWidget> & drag);
        void handleClick(Board::BoardWidget::ClickHandle & click);
        void handleGamestateChange(const SpellChessLib::GameState & state);
        void handleMoveListChanged(const std::vector<SpellChessLib::SpellChessMove> & moves) {};
        void handleGameRunnerIdChanged(int id) {};

        public:

        GameSessionView(GameSessionModel * model, QWidget * parent = nullptr);
        ~GameSessionView();
    };

} // namespace GameSession
