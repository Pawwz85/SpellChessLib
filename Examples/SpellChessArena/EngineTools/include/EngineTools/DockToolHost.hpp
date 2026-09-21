#pragma once

#include <QDockWidget>

#include "EmbeddedToolRegistry.hpp"

class DockToolHost : public QDockWidget {
    Q_OBJECT
    EngineTools::ToolInstanceKey instanceKey;

protected:
    void closeEvent(QCloseEvent *event) override;

public:
    DockToolHost(EngineTools::ToolTypeId type, unsigned int engineId, const QString &title, QWidget *parent = nullptr);

    EngineTools::ToolInstanceKey key() const { return instanceKey; }

signals:
    void toolClosed(EngineTools::ToolTypeId type, unsigned int engineId);
};
