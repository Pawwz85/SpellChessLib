#include <DockToolHost.hpp>

#include <QCloseEvent>

DockToolHost::DockToolHost(EngineTools::ToolTypeId type, EngineTools::EngineRole role, const QString &title, QWidget *parent)
    : QDockWidget(title, parent), instanceKey{type, role} {
    setObjectName(QStringLiteral("engineToolDock_") + EngineTools::descriptorFor(type)->stableId
                 + QStringLiteral("_") + QString::number(role));
    setAttribute(Qt::WA_DeleteOnClose);
    setAllowedAreas(Qt::AllDockWidgetAreas);
}

void DockToolHost::closeEvent(QCloseEvent *event) {
    Q_EMIT toolClosed(instanceKey.type, instanceKey.engineRole);
    QDockWidget::closeEvent(event);
}
