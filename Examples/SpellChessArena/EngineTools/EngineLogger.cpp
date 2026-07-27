#include <EngineLogger.hpp>

EngineLogger::EngineLogger(size_t maximumLineCount) : maximumLineCount(maximumLineCount){
};


bool EngineLogger::isFull() const noexcept {
    return currentLineCount == maximumLineCount;
};

void EngineLogger::shiftHeadPosition() noexcept {
    headIndex++;
    headIndex = headIndex % maximumLineCount;
};

void EngineLogger::appendLine(const QString & line) {
    if (isFull()){
        lines[headIndex] = line;
        shiftHeadPosition();
    } else {
        lines.push_back(line);
        currentLineCount = lines.size();
    }
};

void EngineLogger::log(const std::string & msg){
    appendLine(QString::fromStdString(msg)); 
    contentChanged(getContent());
};

QString EngineLogger::getContent() const {
    QString result;

    for (size_t i = headIndex; i < currentLineCount; ++i)
        result += lines[i] + "\n";

    for (size_t i = 0; i < headIndex; ++i)
        result += lines[i] + "\n";        

    return result;
};

void EngineLogsWidget::onLogsContentChanged(const QString & content) {
    textEdit->setText(content);
}

EngineLogsWidget::EngineLogsWidget(EngineLogger & logger, QWidget * parent) : QWidget(parent) {
    textEdit = new QTextEdit();
    textEdit->setEnabled(false);
    connect(&logger, &EngineLogger::contentChanged, this, &EngineLogsWidget::onLogsContentChanged);
};

#include <moc_EngineLogger.cpp>