#pragma once

#include <map>
#include <memory>

#include <QObject>
#include <QMainWindow>
#include <QPointer>
#include <QEvent>

#include <SpellChess/Board.h>

#include "DockToolHost.hpp"

class ToolHostController : public QObject {
    Q_OBJECT
    QMainWindow *mainWindow;
    std::map<EngineTools::ToolTypeId, QPointer<DockToolHost>> embedded;

    void saveSettings() const;
    bool addEmbedded(EngineTools::ToolTypeId type, const EngineServiceEntry &engine, Qt::DockWidgetArea area);
    bool addEmbeddedWithoutSaving(EngineTools::ToolTypeId type, const EngineServiceEntry &engine, Qt::DockWidgetArea area);
    bool eventFilter(QObject *watched, QEvent *event) override;

private slots:
    void handleDockClosed(EngineTools::ToolTypeId type);

public:
    explicit ToolHostController(QMainWindow *mainWindow, QObject *parent = nullptr);
    ~ToolHostController() override;

    bool addEmbedded(EngineTools::ToolTypeId type, const EngineServiceEntry &engine);
    bool removeEmbedded(EngineTools::ToolTypeId type);
    bool hasEmbedded(EngineTools::ToolTypeId type) const;
    void openStandalone(EngineTools::ToolTypeId type, const EngineServiceEntry &engine);
    void loadSettings();
    void resetLayout();
    void forwardAnalysisContext(const SpellChessLib::GameState &context);

signals:
    void embeddedToolsChanged();
};
