#include <ToolHostController.hpp>

#include <PrincipalVariationsPanel.hpp>

#include <QSettings>
#include <QSignalBlocker>
#include <QTimer>

#include <set>
#include <vector>

namespace {
constexpr int settingsSchemaVersion = 1;
constexpr auto settingsPrefix = "ui/engineTools/";

bool isValidDockArea(int area) {
    return area == Qt::TopDockWidgetArea || area == Qt::BottomDockWidgetArea
        || area == Qt::LeftDockWidgetArea || area == Qt::RightDockWidgetArea;
}
}

ToolHostController::ToolHostController(QMainWindow *mainWindow, QObject *parent)
    : QObject(parent), mainWindow(mainWindow) {
    mainWindow->setDockNestingEnabled(true);
}

ToolHostController::~ToolHostController() {
    if (settingsLoaded)
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
    if (settingsLoaded)
        return;

    settingsLoaded = true;
    restoringSettings = true;

    QSettings settings;;
    const auto prefix = QString::fromLatin1(settingsPrefix);
    const auto schemaKey = prefix + QStringLiteral("schemaVersion");
    const auto instancesKey = prefix + QStringLiteral("embeddedInstances");
    const bool hasAnySettings = settings.contains(schemaKey) || settings.contains(instancesKey)
        || settings.contains(prefix + QStringLiteral("hostState"))
        || settings.contains(prefix + QStringLiteral("geometry"));

    bool validSettings = !hasAnySettings;
    if (settings.contains(schemaKey)) {
        bool schemaIsValid = false;
        const int schema = settings.value(schemaKey).toInt(&schemaIsValid);
        validSettings = schemaIsValid && schema == settingsSchemaVersion
            && settings.contains(instancesKey);
    }

    if (!validSettings) {
        settingsCanBeSaved = false;
        restoringSettings = false;
        return;
    }

    const auto state = settings.value(prefix + QStringLiteral("hostState")).toByteArray();
    const auto geometry = settings.value(prefix + QStringLiteral("geometry")).toByteArray();
    const auto instances = settings.value(instancesKey).toStringList();
    std::set<EngineTools::ToolInstanceKey> restoredKeys;
    bool containsMalformedInstance = false;

    for (const auto &instanceId : instances) {
        const auto instancePrefix = prefix + QStringLiteral("instances/") + instanceId + QStringLiteral("/");
        const auto stableId = settings.value(instancePrefix + QStringLiteral("toolId")).toString();
        const auto type = EngineTools::toolTypeFromStableId(stableId);
        if (instanceId.isEmpty() || !type.has_value()) {
            containsMalformedInstance = true;
            continue;
        }

        bool validEngineId = false;
        const int roleId = settings.value(instancePrefix + QStringLiteral("roleId"), -1).toInt(&validEngineId);
        validEngineId = validEngineId && roleId >= 0 && roleId < EngineTools::EngineRoleEnum_Size;
        bool validArea = false;
        const int areaId = settings.value(instancePrefix + QStringLiteral("dockArea"), -1).toInt(&validArea);
        const auto expectedInstanceId = stableId + QStringLiteral("_") + QString::number(roleId);
        if (!validEngineId || !validArea || !isValidDockArea(areaId) || instanceId != expectedInstanceId) {
            containsMalformedInstance = true;
            continue;
        }

        const auto engineRole = static_cast<EngineTools::EngineRole>(roleId);
        const EngineTools::ToolInstanceKey key{type.value(), engineRole};
        if (!restoredKeys.insert(key).second
            || !addEmbeddedWithoutSaving(key, static_cast<Qt::DockWidgetArea>(areaId)))
            containsMalformedInstance = true;
    }

    QTimer::singleShot(0, this, [this, geometry, state, containsMalformedInstance] {
        if (!geometry.isEmpty())
            mainWindow->restoreGeometry(geometry);
        if (!state.isEmpty())
            mainWindow->restoreState(state);

        // Do not save here. Layout events generated by restoration may be
        // delivered after this callback, and are not user changes.
        restoringSettings = false;
        settingsCanBeSaved = !containsMalformedInstance;
    });
}

void ToolHostController::saveSettings() const {
    if (!settingsLoaded || restoringSettings || !settingsCanBeSaved)
        return;

    QSettings settings;
    const auto prefix = QString::fromLatin1(settingsPrefix);
    settings.setValue(prefix + QStringLiteral("schemaVersion"), settingsSchemaVersion);

    settings.remove(prefix + QStringLiteral("instances"));
    QStringList instances;
    for (const auto &[key, dock] : embedded) {
        if (dock.isNull())
            continue;
        const auto *descriptor = EngineTools::descriptorFor(key.type);
        if (!descriptor)
            continue;
        const auto instanceId = descriptor->stableId + QStringLiteral("_") + QString::number(key.engineRole);
        const auto instancePrefix = prefix + QStringLiteral("instances/") + instanceId + QStringLiteral("/");
        instances.append(instanceId);
        settings.setValue(instancePrefix + QStringLiteral("toolId"), descriptor->stableId);
        settings.setValue(instancePrefix + QStringLiteral("roleId"), key.engineRole);
        settings.setValue(instancePrefix + QStringLiteral("dockArea"), static_cast<int>(mainWindow->dockWidgetArea(dock.data())));
    }
    settings.setValue(prefix + QStringLiteral("embeddedInstances"), instances);
    settings.setValue(prefix + QStringLiteral("hostState"), mainWindow->saveState());
    settings.setValue(prefix + QStringLiteral("geometry"), mainWindow->saveGeometry());
    settings.sync();
}

void ToolHostController::resetLayout() {
    if (!settingsLoaded)
        return;

    restoringSettings = true;
    const auto activeInstances = std::vector<EngineTools::ToolInstanceKey>([this] {
        std::vector<EngineTools::ToolInstanceKey> result;
        for (const auto &[key, dock] : embedded)
            if (!dock.isNull())
                result.push_back(key);
        return result;
    }());

    for (const auto key : activeInstances) {
        const auto found = embedded.find(key);
        if (found == embedded.end() || found->second.isNull())
            continue;
        auto *dock = found->second.data();
        embedded.erase(found);
        delete dock;
        Q_EMIT embeddedToolsChanged();
        const auto *descriptor = EngineTools::descriptorFor(key.type);
        Qt::DockWidgetArea area = (descriptor != nullptr) ? descriptor->defaultDockArea :  Qt::RightDockWidgetArea;
        addEmbeddedWithoutSaving(key, area);
    }
    restoringSettings = false;
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