#pragma once

#include <QDockWidget>

#include "EngineRolesService.hpp"
#include "EmbeddedToolRegistry.hpp"

class DockToolHost : public QDockWidget {
    Q_OBJECT
    EngineTools::ToolInstanceKey instanceKey;

protected:
    void closeEvent(QCloseEvent *event) override;

public:
    DockToolHost(EngineTools::ToolTypeId type, EngineTools::EngineRole role, const QString &title, QWidget *parent = nullptr);

    EngineTools::ToolInstanceKey key() const { return instanceKey; }

signals:
    void toolClosed(EngineTools::ToolTypeId type, EngineTools::EngineRole role);
};
