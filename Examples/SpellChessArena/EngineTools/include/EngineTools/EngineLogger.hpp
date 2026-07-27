#pragma once

#include <QString>
#include <QObject>
#include <QTextEdit>

#include <vector>

/*
    Custom logger implementation that stores engine logs inside a circular buffer.
*/
class EngineLogger : public QObject {
    Q_OBJECT
    size_t maximumLineCount;
    size_t currentLineCount = 0;
    size_t headIndex = 0; // index of first element of the buffer
    std::vector<QString> lines;

    bool isFull() const noexcept;
    void shiftHeadPosition() noexcept;
    void appendLine(const QString & line);

    public:
    EngineLogger(size_t maximumLineCount = 1024);
    void log(const std::string & msg);
    QString getContent() const;

    signals:
    void contentChanged(const QString & content);
};

class EngineLogsWidget : public QWidget {
    Q_OBJECT
    QTextEdit * textEdit;

    private slots:
    void onLogsContentChanged(const QString & content);

    public:
    EngineLogsWidget(EngineLogger & logger, QWidget* parent = nullptr );
};