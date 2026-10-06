#pragma once

#include <map>
#include <memory>

#include <QObject>
#include <QMainWindow>
#include <QPointer>

#include <SpellChess/Board.h>

#include "DockToolHost.hpp"

class ToolHostController : public QObject {
    Q_OBJECT
    QMainWindow *mainWindow;
    std::map<EngineTools::ToolInstanceKey, QPointer<DockToolHost>> embedded;
    bool settingsLoaded = false;
    bool restoringSettings = false;
    bool settingsCanBeSaved = false;

    void saveSettings() const;
    bool addEmbedded(EngineTools::ToolInstanceKey key, Qt::DockWidgetArea area);
    bool addEmbeddedWithoutSaving(EngineTools::ToolInstanceKey key, Qt::DockWidgetArea area);

private slots:
    void handleDockClosed(EngineTools::ToolTypeId type, EngineTools::EngineRole role);

public:
    explicit ToolHostController(QMainWindow *mainWindow, QObject *parent = nullptr);
    ~ToolHostController() override;

    bool addEmbedded(EngineTools::ToolTypeId type, EngineTools::EngineRole role);
    bool addEmbedded(const EngineTools::ToolInstanceKey &key);
    bool removeEmbedded(const EngineTools::ToolInstanceKey &key);
    bool removeEmbedded(EngineTools::ToolTypeId type, EngineTools::EngineRole role);
    bool hasEmbedded(const EngineTools::ToolInstanceKey &key) const;
    bool hasEmbedded(EngineTools::ToolTypeId type, EngineTools::EngineRole role) const;
    void loadSettings();
    void resetLayout();
    void forwardAnalysisContext(const SpellChessLib::GameState &context);

signals:
    void embeddedToolsChanged();
};
