#pragma once

#include <QDockWidget>

#include "EmbeddedToolRegistry.hpp"

class DockToolHost : public QDockWidget {
    Q_OBJECT
    EngineTools::ToolTypeId type;

protected:
    void closeEvent(QCloseEvent *event) override;

public:
    DockToolHost(EngineTools::ToolTypeId type, const QString &title, QWidget *parent = nullptr);

signals:
    void toolClosed(EngineTools::ToolTypeId type);
};
