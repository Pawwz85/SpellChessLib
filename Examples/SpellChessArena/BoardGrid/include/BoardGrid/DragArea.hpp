#pragma once
#include "BoardEntity.h"
#include <memory>
#include <functional>
#include <list>
#include <QPointF>
#include <QPropertyAnimation>
#include <mutex>

namespace Board {

    namespace Internal {
        struct DragInfo {
            mutable std::mutex lock;
            unsigned int entityId;
            int startingZLevel;
            QPointF startingLocation;
            QPointF currentLocation;
            bool closed;
            bool paused;
        };

        class MouseFollowAnimation : public QPropertyAnimation {
            std::shared_ptr<DragInfo> info;
            public:
            MouseFollowAnimation(QObject * parent, std::shared_ptr<DragInfo> info);

            // checks if drag has ended
            void updateCurrentTime(int currentTime) override;
        };

    };

    template <class Context>
    class DragHandle {
        protected:
            Context * context;
            std::shared_ptr<Internal::DragInfo> info;
            void close();
        public:
            DragHandle(Context & context, std::shared_ptr<Internal::DragInfo> info) : info(info), context(&context) {};
            virtual ~DragHandle();

            virtual void reject() = 0;
            virtual void accept() = 0;
            virtual BoardEntity * getEntity() = 0;

            Context & getContext();
            const Context & getContext() const;

            QPointF getStartingPosition() const;
            QPointF getFinalPosition() const;

            bool isClosed() const;
    };

    template <class Context>
    class DragArea {
        protected:

        std::shared_ptr<Internal::DragInfo> currentlyDraggedPiece;

        protected:
        void updateDragAnimation(const QPointF & currentMousePosition);
        std::unique_ptr<QPropertyAnimation> getDragAnimation(std::shared_ptr<Internal::DragInfo> info);

        public:
        virtual ~DragArea() = default;

        virtual DragHandle<Context> * startDrag(unsigned int EntityId) = 0;
    };
};

template <class Context>
bool Board::DragHandle<Context>::isClosed() const {
    if (info != nullptr) {
        std::lock_guard<std::mutex> guard(info->lock);
        return info->closed;
    }
    return true;   
}

template <class Context>
void Board::DragHandle<Context>::close() {
    if(info != nullptr) {
        std::lock_guard<std::mutex> guard(info->lock);
        info->closed = true;
    }
};

template <class Context>
Board::DragHandle<Context>::~DragHandle() {
   close();
};

template <class Context>
Context & Board::DragHandle<Context>::getContext() {
    return *context;
};

template <class Context>
const Context & Board::DragHandle<Context>::getContext() const {
    return *context;
}

template <class Context>
inline QPointF Board::DragHandle<Context>::getStartingPosition() const{
    QPointF result;
    if(info != nullptr) {
        std::lock_guard<std::mutex> guard(info->lock);
        result = info->startingLocation;
    }
    return result;
}

template <class Context>
inline QPointF Board::DragHandle<Context>::getFinalPosition() const{
    QPointF result;
    if(info != nullptr) {
        std::lock_guard<std::mutex> guard(info->lock);
        result = info->currentLocation;
    }
    return result;
};

template <class Context>
void Board::DragArea<Context>::updateDragAnimation(const QPointF & currentPosition) {
    if (currentlyDraggedPiece != nullptr) {
        std::lock_guard<std::mutex> guard(currentlyDraggedPiece->lock);
        currentlyDraggedPiece->currentLocation = currentPosition;
    };
};

template <class Context>
std::unique_ptr<QPropertyAnimation> Board::DragArea<Context>::getDragAnimation(std::shared_ptr<Internal::DragInfo> info) {
    return std::make_unique<Internal::MouseFollowAnimation>(nullptr, info);
};
