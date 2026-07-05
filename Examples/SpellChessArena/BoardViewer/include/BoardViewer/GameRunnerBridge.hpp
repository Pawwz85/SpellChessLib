#pragma once

#include <QEvent>
#include <QCoreApplication>
#include <QObject>
#include <SpellChess/GameMonitor.h>
#include <SpellChess/Observer.h>
#include <SpellChess/Player.h>
#include <SpellChess/GameAnnotations.h>

#include <memory>
#include <vector>
#include <future>

namespace BoardViewer {


    namespace Internal {
    
        class GameRunnerEvents {
        public:
        GameRunnerEvents();
        const int GameStateChanged;
        const int MovesSinceRootChanged;
        const int GameRunnerIdChanged;
        const int TimerPairStateChanged;
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

        class TimerPairEvent : public QEvent {
            public:
            TimerPairEvent(const SpellChessLib::TimerPairState & state);
            const SpellChessLib::TimerPairState state;
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
    class GameRunnerBridge {

        QCoreApplication * app;

        EVENT_EMITTER_DECL(GameStateChangedEvent, SpellChessLib::GameState *);
        EVENT_EMITTER_DECL(MoveListEvent, std::vector<SpellChessLib::SpellChessMove>);
        EVENT_EMITTER_DECL(TimerPairEvent, SpellChessLib::TimerPairState);
        EVENT_EMITTER_DECL(GameRunnerEvent, int);

        std::shared_ptr<SpellChessLib::GameMonitor> gameMonitor;
        std::unique_ptr<std::promise<SpellChessLib::SpellChessMove>> movePromise;

        public:
        GameRunnerBridge(QCoreApplication * app, QObject * owner);

        std::shared_ptr<SpellChessLib::GameMonitor> getGameMonitor();  

    };

    #undef EVENT_EMITTER_DECL

};