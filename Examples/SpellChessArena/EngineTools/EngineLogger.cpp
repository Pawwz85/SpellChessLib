#include <EngineLogger.hpp>
#include <QVBoxLayout>

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
    newLineLogged(QString::fromStdString(msg));
};

QString EngineLogger::getContent() const {
    QString result;

    for (size_t i = headIndex; i < currentLineCount; ++i)
        result += lines[i];

    for (size_t i = 0; i < headIndex; ++i)
        result += lines[i];        

    return result;
};

void EngineLogsWidget::onNewLineLogged(QString line) {
    line.chop(1); // remove the trailing newline character
    textEdit->append(line);
}

EngineLogsWidget::EngineLogsWidget(EngineLogger & logger, QWidget * parent) : QWidget(parent) {
    textEdit = new QTextEdit(this);
    auto layout = new QVBoxLayout(this);
    layout->addWidget(textEdit);
    textEdit->setPlainText(logger.getContent());
    textEdit->setReadOnly(true);
    connect(&logger, &EngineLogger::newLineLogged, this, &EngineLogsWidget::onNewLineLogged);
};

#include <moc_EngineLogger.cpp>