#pragma once

#include <QEvent>
#include <QCoreApplication>
#include <QObject>
#include <SpellChess/GameMonitor.h>
#include <SpellChess/Observer.h>
#include <SpellChess/Player.h>
#include <SpellChess/GameAnnotations.h>
#include <SpellChess/Scheduler/ObjectAllocator.h> // for resource pointer
#include <SpellChess/Factory.h> // for resource pointer

#include <memory>
#include <vector>
#include <future>

namespace GameSession {


    namespace Internal {
    
        class GameRunnerEvents {
        public:
        GameRunnerEvents();
        const int GameStateChanged;
        const int MovesSinceRootChanged;
        const int GameRunnerIdChanged;
        };

        class GameStateChangedEvent : public QEvent {
            std::unique_ptr<SpellChessLib::GameState> state; 

            public:
            GameStateChangedEvent(const SpellChessLib::GameState * newState);
            bool isEmpty() const;
            const SpellChessLib::GameState * get() const;
            const SpellChessLib::GameState & operator*() const;
        };

        class MoveListEvent : public QEvent {
            std::vector<SpellChessLib::SpellChessMove> moves; 

            public:
            MoveListEvent(const std::vector<SpellChessLib::SpellChessMove> & moves);
            const std::vector<SpellChessLib::SpellChessMove> & movesSinceRoot() const;
        };

        class GameRunnerEvent : public QEvent {
            int gameRunnerId;

            public:
            GameRunnerEvent(int id);
            int getGameRunnerId() const;
            bool connected() const;
        };

        template <class Event, typename State>
        class EventEmmiter : public SpellChessLib::Observer<State> {
            QCoreApplication * app;
            QObject * object;
            public:
            EventEmmiter(QCoreApplication * app, QObject * obj) : app(app), object(obj) {};
            void onObservedStateUpdate(const State & state) override {
                Event * event = new Event(state);
                app->postEvent(object, event);
            };
        };
    }

    extern Internal::GameRunnerEvents gameSessionEvents;

    #define EVENT_EMITTER_DECL(EVENT, STATE) std::shared_ptr<Internal::EventEmmiter<Internal::EVENT, STATE>> emitter##EVENT

    class GuiPlayer;
    class GameRunnerBridge {
        friend GuiPlayer; 

        QCoreApplication * app;

        EVENT_EMITTER_DECL(GameStateChangedEvent, SpellChessLib::GameState *);
        EVENT_EMITTER_DECL(MoveListEvent, std::vector<SpellChessLib::SpellChessMove>);
        EVENT_EMITTER_DECL(GameRunnerEvent, int);

        std::shared_ptr<SpellChessLib::GameMonitor> gameMonitor;
        std::unique_ptr<std::promise<SpellChessLib::SpellChessMove>> movePromise;
        
        std::shared_ptr<SpellChessLib::Factory<SpellChessLib::Player>> playerFactory; 

        public:
        GameRunnerBridge(QCoreApplication * app, QObject * owner);

        std::shared_ptr<SpellChessLib::GameMonitor> getGameMonitor();  
        SpellChessLib::Scheduler::ResourcePointer<SpellChessLib::Player> makePlayerFromSession();
        void submitMove(SpellChessLib::SpellChessMove move);

    };


    class GuiPlayer : public SpellChessLib::Player {

        GameRunnerBridge * session;

        // inherited from SpellChessLib::Player
        Result makeResult(const SpellChessLib::Game& game, const std::chrono::milliseconds & timeOut) override;
		Result handlePonderHit() override;
		void startPondering(const SpellChessLib::Game& game, const SpellChessLib::SpellChessMove & ponderMove) override;
		void cancelPondering() override;
        bool supportsPondering() const override;
		void onNewGame() override;
		void interrupt() override; 
		void kill() override; 

        public:

        GuiPlayer(GameRunnerBridge * session);         
    };

    #undef EVENT_EMITTER_DECL

};