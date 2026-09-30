#pragma once

#include <QString>
#include <QObject>
#include <QTextEdit>

#include <vector>

#include "EngineTool.hpp"

class EngineLogsWidget : public QWidget, public EngineTools::EngineTool {
    Q_OBJECT
    QTextEdit * textEdit;

    int engineId;
    QMetaObject::Connection loggerConnection;
    void disconnectEngine() override;
    void connectEngine(const EngineServiceEntry & engineEntry) override;

    private slots:
    void onNewLineLogged(QString line);

    public:
    EngineLogsWidget(const EngineServiceEntry & engineEntry, QWidget* parent = nullptr );
};