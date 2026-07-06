#include <PlayerTimer.hpp>
#include <QTime>
#include <ctime>
#include <QHBoxLayout>

using namespace BoardViewer;

QString PlayerTimer::formatTime() const {
    if (!state.showValue)
        return "--:--.--";

    QString format = "s";
    unsigned int timeLeft = state.timeLeft.count();
    timeLeft -= timeLeft % 10;

    if (timeLeft <= 60 * 1000) // if less than minute is left, show milliseconds 
        format += ".zzz";

    if (timeLeft >= 60 * 1000)
        format = "m:s" + format;

    if (timeLeft >= 3600 * 10000)
        format = "h:m" + format;
        
    unsigned int ms     = timeLeft % 1000;
    unsigned int s      = (timeLeft/ 1000) % 60;
    unsigned int min    = (timeLeft / 60000) % 60;
    unsigned int h      = timeLeft / 3600000;

    QTime time(h, min, s, ms);
    return padWithSpaces(time.toString(format));
};

QString PlayerTimer::padWithSpaces(const QString & string) const {
    size_t desiredLength    = getDesiredTextLength();
    size_t actualLength     = string.length();
    size_t leftPadSize      = (desiredLength - actualLength) / 2;
    size_t rightPadSize     = desiredLength - actualLength - leftPadSize;

    QString result = string;

    for(size_t i = 0; i < leftPadSize; ++i)
        result.prepend(' ');
   
    for(size_t i = 0; i < rightPadSize; ++i)
        result.append(' ');

     return result;   
};

PlayerTimer::PlayerTimer(QWidget * parent ) : QWidget(parent) {
    state.running = state.showValue = false;
    state.timeLeft = std::chrono::milliseconds(0);

    content = new QLabel(this);
    content->setText(formatTime());
    content->setFrameStyle(QFrame::Shape::Box);
    content->setAlignment(Qt::AlignCenter);

    QHBoxLayout *l = new QHBoxLayout(this);
    l->addWidget(content);
    l->setAlignment(Qt::AlignCenter);
    setLayout(l);

    timer = new QTimer(this);
    timer->setInterval(std::chrono::milliseconds(10));    
    connect(timer, &QTimer::timeout, this, &PlayerTimer::onTick);
    timer->start();
};

void PlayerTimer::setState(const SpellChessLib::PlayerTimerState & newState) {
    this->state = newState;
    content->setText(formatTime());
    update();
}

void PlayerTimer::onTick() {
    if(state.running) {
        long long newValue = std::max(0LL, state.timeLeft.count() - 10); 
        state.timeLeft = std::chrono::milliseconds(newValue);
        content->setText(formatTime());
        update(); 
    };
};


#include <moc_PlayerTimer.cpp>