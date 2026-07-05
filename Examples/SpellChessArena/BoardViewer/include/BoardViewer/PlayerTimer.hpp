#pragma once

#include <SpellChess/TimerService.h>
#include <QWidget>
#include <QLabel>
#include <QTimer>

namespace BoardViewer {
    class PlayerTimer : public QWidget {
        Q_OBJECT
        SpellChessLib::PlayerTimerState state;
        QTimer * timer;
        QLabel * content;
        
        QString formatTime() const;
        QString padWithSpaces(const QString & string) const;
        size_t getDesiredTextLength() const {return 12;};

        public:
        PlayerTimer(QWidget * parent = nullptr);
        void setState(const SpellChessLib::PlayerTimerState & newState);

        public slots:
        void onTick();
    };
}
