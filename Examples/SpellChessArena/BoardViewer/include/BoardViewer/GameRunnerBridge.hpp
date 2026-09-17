#pragma once

#include <QEvent>
#include <QCoreApplication>
#include <QObject>
#include <QPointer>
#include <QTimer>
#include <SpellChess/GameMonitor.h>
#include <SpellChess/Observer.h>
#include <SpellChess/Player.h>
#include <SpellChess/GameAnnotations.h>

#include <algorithm>
#include <chrono>
#include <memory>
#include <mutex>
#include <optional>
#include <type_traits>
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
            class EventEmmiter : public SpellChessLib::Observer<State>, public std::enable_shared_from_this<EventEmmiter<Event, State>> {
                using Snapshot = std::conditional_t<std::is_pointer_v<State>,
                    std::shared_ptr<std::remove_pointer_t<State>>, State>;

                QPointer<QObject> object;
                std::mutex mutex;
                std::optional<Snapshot> pending;
                bool deliveryScheduled = false;
                std::chrono::milliseconds deliveryInterval;

                static Snapshot copyState(const State & state);

                void scheduleDelivery();

            public:
                EventEmmiter(QObject * obj, std::chrono::milliseconds interval) :
                    object(obj), deliveryInterval(std::max(interval, std::chrono::milliseconds(1))) {};

                void setDeliveryInterval(std::chrono::milliseconds interval) {
                    std::lock_guard<std::mutex> guard(mutex);
                    deliveryInterval = std::max(interval, std::chrono::milliseconds(1));
                }

                void onObservedStateUpdate(const State & state) override;

                void deliverPending();
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
        GameRunnerBridge(QCoreApplication * app, QObject * owner,
                 std::chrono::milliseconds eventDeliveryInterval = std::chrono::milliseconds(16));

        void setEventDeliveryInterval(std::chrono::milliseconds interval);

        std::shared_ptr<SpellChessLib::GameMonitor> getGameMonitor();  

    };

    #undef EVENT_EMITTER_DECL

    template <class Event, typename State>
    typename Internal::EventEmmiter<Event, State>::Snapshot 
    Internal::EventEmmiter<Event, State>::copyState(const State & state) { 
        if constexpr (std::is_pointer_v<State>) {
            return state == nullptr   ? nullptr : std::make_shared<std::remove_pointer_t<State>>(*state);
        } else
            return state;
    }

    template <class Event, typename State>
    void Internal::EventEmmiter<Event, State>::scheduleDelivery() {
        if (object.isNull())
            return;

        deliveryScheduled = true;
        const auto weakEmitter = this->weak_from_this();
        QTimer::singleShot(deliveryInterval, object, [weakEmitter]() {
            if (auto emitter = weakEmitter.lock())
                emitter->deliverPending();
        });
    }

    template <class Event, typename State>
    void Internal::EventEmmiter<Event, State>::onObservedStateUpdate(const State & state) {
        std::lock_guard<std::mutex> guard(mutex);
        pending = copyState(state);
        if (!deliveryScheduled) 
            scheduleDelivery();
    };

    template <class Event, typename State>
    void Internal::EventEmmiter<Event, State>::deliverPending() {
        std::optional<Snapshot> snapshot;
        std::lock_guard<std::mutex> guard(mutex);

        snapshot.swap(pending);

        if (snapshot.has_value() && !object.isNull()) {
            if constexpr (std::is_pointer_v<State>) {
                Event event(snapshot->get());
                QCoreApplication::sendEvent(object, &event);
            } else {
                Event event(*snapshot);
                QCoreApplication::sendEvent(object, &event);
            }
        }
        
        if (pending.has_value()) 
            scheduleDelivery();
        else 
            deliveryScheduled = false;
    }
};

