#include <ToolHostController.hpp>

#include <PrincipalVariationsPanel.hpp>

#include <QSettings>
#include <QTimer>
#include <QSignalBlocker>

#include <vector>

namespace {
constexpr int settingsSchemaVersion = 1;
constexpr auto settingsPrefix = "ui/engineTools/";
}

ToolHostController::ToolHostController(QMainWindow *mainWindow, QObject *parent)
    : QObject(parent), mainWindow(mainWindow) {
    mainWindow->setDockNestingEnabled(true);
}

ToolHostController::~ToolHostController() {
    saveSettings();
}

bool ToolHostController::hasEmbedded(const EngineTools::ToolInstanceKey &key) const {
    const auto found = embedded.find(key);
    return found != embedded.end() && !found->second.isNull();
}

bool ToolHostController::hasEmbedded(EngineTools::ToolTypeId type, EngineTools::EngineRole role) const {
    return hasEmbedded(EngineTools::ToolInstanceKey{type, role});
}

bool ToolHostController::addEmbedded(EngineTools::ToolTypeId type, EngineTools::EngineRole role) {
    return addEmbedded(EngineTools::ToolInstanceKey{type, role});
}

bool ToolHostController::addEmbedded(const EngineTools::ToolInstanceKey &key) {
    const auto *descriptor = EngineTools::descriptorFor(key.type);
    Qt::DockWidgetArea area = (descriptor != nullptr) ? descriptor->defaultDockArea :  Qt::RightDockWidgetArea;
    return addEmbedded(key, area);
}

bool ToolHostController::addEmbedded(EngineTools::ToolInstanceKey key,
                                     Qt::DockWidgetArea area) {
    if (addEmbeddedWithoutSaving(key, area)) {
        saveSettings();
        return true;        
    }
    return false;
}

bool ToolHostController::addEmbeddedWithoutSaving(EngineTools::ToolInstanceKey key, Qt::DockWidgetArea area) {
    if (hasEmbedded(key))
        return false;

    const auto *descriptor = EngineTools::descriptorFor(key.type);
    if (!descriptor)
        return false;

    auto *dock = new DockToolHost(key.type, key.engineRole, descriptor->defaultTitle, mainWindow);
    dock->setWidget(descriptor->createWidget(key.engineRole, dock));
    dock->resize(descriptor->defaultSize);
    dock->setWindowTitle(QString("%1: %2").arg(descriptor->defaultTitle, EngineTools::engineRoleToQString(key.engineRole)));
    connect(dock, &DockToolHost::toolClosed, this, &ToolHostController::handleDockClosed);
    dock->installEventFilter(this);

    DockToolHost *previousInstance = nullptr;
    for (const auto &[otherKey, existingDock] : embedded) {
        if (otherKey.type == key.type && !existingDock.isNull()
            && mainWindow->dockWidgetArea(existingDock.data()) == area) {
            previousInstance = existingDock.data();
            break;
        }
    }
    if (previousInstance != nullptr)
        mainWindow->splitDockWidget(previousInstance, dock, Qt::Horizontal);
    else
        mainWindow->addDockWidget(area, dock);

    embedded[key] = dock;
    Q_EMIT embeddedToolsChanged();
    return true;
}


void ToolHostController::handleDockClosed(EngineTools::ToolTypeId type, EngineTools::EngineRole role) {
    embedded.erase(EngineTools::ToolInstanceKey{type, role});
    Q_EMIT embeddedToolsChanged();
    saveSettings();
}

bool ToolHostController::eventFilter(QObject *watched, QEvent *event) {
   if (qobject_cast<DockToolHost *>(watched) &&
        (event->type() == QEvent::Move || event->type() == QEvent::Resize ||
         event->type() == QEvent::Show || event->type() == QEvent::Hide))
        saveSettings();
   return QObject::eventFilter(watched, event);
}

bool ToolHostController::removeEmbedded(const EngineTools::ToolInstanceKey &key) {
    const auto found = embedded.find(key);
    if (found == embedded.end() || found->second.isNull())
        return false;

    auto *dock = found->second.data();
    embedded.erase(found);
    dock->deleteLater();
    Q_EMIT embeddedToolsChanged();
    saveSettings();
    return true;
}

bool ToolHostController::removeEmbedded(EngineTools::ToolTypeId type, EngineTools::EngineRole role) {
    return removeEmbedded(EngineTools::ToolInstanceKey{type, role});
}

void ToolHostController::loadSettings() {
    QSettings settings;
    if (settings.value(QString::fromLatin1(settingsPrefix) + QStringLiteral("schemaVersion"), 0).toInt() != settingsSchemaVersion)
        return;

    const auto state = settings.value(QString::fromLatin1(settingsPrefix) + QStringLiteral("hostState")).toByteArray();
    const auto geometry = settings.value(QString::fromLatin1(settingsPrefix) + QStringLiteral("geometry")).toByteArray();
    const auto instances = settings.value(QString::fromLatin1(settingsPrefix) + QStringLiteral("embeddedInstances")).toStringList();

    for (const auto &instanceId : instances) {
        const auto instancePrefix = QString::fromLatin1(settingsPrefix) + QStringLiteral("instances/") + instanceId + QStringLiteral("/");
        const auto stableId = settings.value(instancePrefix + QStringLiteral("toolId")).toString();
        const auto type = EngineTools::toolTypeFromStableId(stableId);
        if (!type.has_value())
            continue;
        bool validEngineId = false;
        const auto roleId = settings.value(instancePrefix + QStringLiteral("roleId"), -1).toUInt(&validEngineId);
        validEngineId = validEngineId && roleId < EngineTools::EngineRoleEnum_Size;
        if (!validEngineId)
            continue;

        EngineTools::EngineRole engineRole = static_cast<EngineTools::EngineRole>(roleId);

        const auto area = static_cast<Qt::DockWidgetArea>(settings.value(instancePrefix + QStringLiteral("dockArea"), static_cast<int>(Qt::RightDockWidgetArea)).toInt());
        EngineTools::ToolInstanceKey key{type.value(), engineRole};
        if (addEmbeddedWithoutSaving(key, area)) {
            const auto found = embedded.find(key);
        }
    }

    if (!geometry.isEmpty())
        mainWindow->restoreGeometry(geometry);
    if (!state.isEmpty())
        QTimer::singleShot(0, this, [this, state](){
        mainWindow->restoreState(state);
    });
}

void ToolHostController::saveSettings() const {
    QSettings settings;
    settings.setValue(QString::fromLatin1(settingsPrefix) + QStringLiteral("schemaVersion"), settingsSchemaVersion);

    settings.remove(QString::fromLatin1(settingsPrefix) + QStringLiteral("instances"));
    QStringList instances;
    for (const auto &[key, dock] : embedded) {
        if (dock.isNull())
            continue;
        const auto *descriptor = EngineTools::descriptorFor(key.type);
        if (!descriptor)
            continue;
        const auto instanceId = descriptor->stableId + QStringLiteral("_") + QString::number(key.engineRole);
        const auto instancePrefix = QString::fromLatin1(settingsPrefix) + QStringLiteral("instances/") + instanceId + QStringLiteral("/");
        instances.append(instanceId);
        settings.setValue(instancePrefix + QStringLiteral("toolId"), descriptor->stableId);
        settings.setValue(instancePrefix + QStringLiteral("roleId"), key.engineRole);
        settings.setValue(instancePrefix + QStringLiteral("dockArea"), static_cast<int>(mainWindow->dockWidgetArea(dock.data())));
    }
    settings.setValue(QString::fromLatin1(settingsPrefix) + QStringLiteral("embeddedInstances"), instances);
    settings.setValue(QString::fromLatin1(settingsPrefix) + QStringLiteral("hostState"), mainWindow->saveState());
    settings.setValue(QString::fromLatin1(settingsPrefix) + QStringLiteral("geometry"), mainWindow->saveGeometry());
    settings.sync();
}

void ToolHostController::resetLayout() {
    const auto activeInstances = std::vector<EngineTools::ToolInstanceKey>([this] {
        std::vector<EngineTools::ToolInstanceKey> result;
        for (const auto &[key, dock] : embedded)
            if (!dock.isNull())
                result.push_back(key);
        return result;
    }());

    for (const auto key : activeInstances) {
        removeEmbedded(key);
        const auto *descriptor = EngineTools::descriptorFor(key.type);
        Qt::DockWidgetArea area = (descriptor != nullptr) ? descriptor->defaultDockArea :  Qt::RightDockWidgetArea;
 
        addEmbedded(key, area);
    }
    saveSettings();
}

void ToolHostController::forwardAnalysisContext(const SpellChessLib::GameState &context) {
    for (const auto &[key, dock] : embedded) {
        if (dock.isNull())
            continue;
        if (auto *panel = qobject_cast<PrincipalVariationPanel *>(dock->widget()))
            panel->contextChanged(context);
    }
}