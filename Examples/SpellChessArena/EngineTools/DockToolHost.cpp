#include <DockToolHost.hpp>

#include <QCloseEvent>

DockToolHost::DockToolHost(EngineTools::ToolTypeId type, const QString &title, QWidget *parent)
    : QDockWidget(title, parent), type(type) {
    setObjectName(QStringLiteral("engineToolDock_") + EngineTools::descriptorFor(type)->stableId);
    setAttribute(Qt::WA_DeleteOnClose);
    setAllowedAreas(Qt::AllDockWidgetAreas);
}

void DockToolHost::closeEvent(QCloseEvent *event) {
    Q_EMIT toolClosed(type);
    QDockWidget::closeEvent(event);
}
