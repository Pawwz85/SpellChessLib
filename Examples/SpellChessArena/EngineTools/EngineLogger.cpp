#include <EngineLogger.hpp>
#include <QVBoxLayout>
#include <QScrollBar>

void EngineLogsWidget::disconnectEngine() {
    auto entry = RunningEnginesService::getInstance().getById(engineId);
    if (entry.has_value())
        entry->logger->disconnect(loggerConnection);

    textEdit->clear();
}

void EngineLogsWidget::connectEngine(const EngineServiceEntry & engineEntry) {
    loggerConnection = connect(engineEntry.logger.get(), &EngineLogger::newLineLogged, this, &EngineLogsWidget::onNewLineLogged);
    textEdit->setPlainText(engineEntry.logger->getContent());
}


void EngineLogsWidget::onNewLineLogged(QString line) {
    line.chop(1); // remove the trailing newline character

    QString storedText = textEdit->toPlainText();

    if (storedText.count('\n') < 4096) // arbitrary limit to avoid excessive memory usage
        textEdit->append(line);
    else {
        // TODO: inform the user that the logs are being truncated
    }
}

EngineLogsWidget::EngineLogsWidget(const EngineServiceEntry & engineEntry, QWidget * parent) : QWidget(parent) {
    textEdit = new QTextEdit(this);
    auto layout = new QVBoxLayout(this);
    layout->addWidget(textEdit);
    textEdit->setReadOnly(true);
    // Ensure the logs panel remains usable at small sizes
    setMinimumSize(400, 300);

    setEngine(engineEntry);
};

#include <moc_EngineLogger.cpp>