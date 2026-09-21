#include <DockToolHost.hpp>

#include <QCloseEvent>

DockToolHost::DockToolHost(EngineTools::ToolTypeId type, unsigned int engineId, const QString &title, QWidget *parent)
    : QDockWidget(title, parent), instanceKey{type, engineId} {
    setObjectName(QStringLiteral("engineToolDock_") + EngineTools::descriptorFor(type)->stableId
                 + QStringLiteral("_") + QString::number(engineId));
    setAttribute(Qt::WA_DeleteOnClose);
    setAllowedAreas(Qt::AllDockWidgetAreas);
}

void DockToolHost::closeEvent(QCloseEvent *event) {
    Q_EMIT toolClosed(instanceKey.type, instanceKey.engineId);
    QDockWidget::closeEvent(event);
}
