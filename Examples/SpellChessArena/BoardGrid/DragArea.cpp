#include <DragArea.hpp>
#include <limits>
#include <QDebug>

Board::Internal::MouseFollowAnimation::MouseFollowAnimation(QObject * parent, std::shared_ptr<DragInfo> info) 
    : QPropertyAnimation(parent), info(info) {
    setDuration(INT_MAX);
    
};

void Board::Internal::MouseFollowAnimation::updateCurrentTime(int currentTime) {
    if(info == nullptr) return;

    std::lock_guard<std::mutex> guard(info->lock);

    if(info->closed) {
        finished();
        stop();
    } else {
        QPointF value = {info->currentLocation.x(), info->currentLocation.y()};
        updateCurrentValue(value);
        QPropertyAnimation::updateCurrentTime(currentTime);
    }    
};
