#include <ToolHostController.hpp>

#include <PrincipalVariationsPanel.hpp>

#include <QSettings>
#include <QTimer>
#include <QSignalBlocker>

#include <vector>

namespace {
constexpr int settingsSchemaVersion = 2;
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

bool ToolHostController::hasEmbedded(EngineTools::ToolTypeId type, unsigned int engineId) const {
    return hasEmbedded(EngineTools::ToolInstanceKey{type, engineId});
}

bool ToolHostController::addEmbedded(EngineTools::ToolTypeId type, const EngineServiceEntry &engine) {
    const auto *descriptor = EngineTools::descriptorFor(type);
    Qt::DockWidgetArea area = (descriptor != nullptr) ? descriptor->defaultDockArea :  Qt::RightDockWidgetArea;
    return addEmbedded(type, engine, area);
}

bool ToolHostController::addEmbedded(EngineTools::ToolTypeId type, const EngineServiceEntry &engine,
                                     Qt::DockWidgetArea area) {
    if (addEmbeddedWithoutSaving(type, engine, area)) {
        saveSettings();
        return true;        
    }
    return false;
}

bool ToolHostController::addEmbeddedWithoutSaving(EngineTools::ToolTypeId type, const EngineServiceEntry &engine, Qt::DockWidgetArea area) {
    if (hasEmbedded(type, engine.id))
        return false;

    const auto *descriptor = EngineTools::descriptorFor(type);
    if (!descriptor || !engine.engine || !engine.logger)
        return false;

    auto *dock = new DockToolHost(type, engine.id, descriptor->defaultTitle, mainWindow);
    dock->setWidget(descriptor->createWidget(engine, dock));
    dock->resize(descriptor->defaultSize);
    dock->setWindowTitle(descriptor->defaultTitle + QStringLiteral(": ") + engine.name);
    connect(dock, &DockToolHost::toolClosed, this, &ToolHostController::handleDockClosed);
    dock->installEventFilter(this);

    DockToolHost *previousInstance = nullptr;
    for (const auto &[key, existingDock] : embedded) {
        if (key.type == type && !existingDock.isNull()
            && mainWindow->dockWidgetArea(existingDock.data()) == area) {
            previousInstance = existingDock.data();
            break;
        }
    }
    if (previousInstance != nullptr)
        mainWindow->splitDockWidget(previousInstance, dock, Qt::Horizontal);
    else
        mainWindow->addDockWidget(area, dock);

    embedded[EngineTools::ToolInstanceKey{type, engine.id}] = dock;
    Q_EMIT embeddedToolsChanged();
    return true;
}


void ToolHostController::handleDockClosed(EngineTools::ToolTypeId type, unsigned int engineId) {
    embedded.erase(EngineTools::ToolInstanceKey{type, engineId});
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

bool ToolHostController::removeEmbedded(EngineTools::ToolTypeId type, unsigned int engineId) {
    return removeEmbedded(EngineTools::ToolInstanceKey{type, engineId});
}

void ToolHostController::openStandalone(EngineTools::ToolTypeId type, const EngineServiceEntry &engine) {
    const auto *descriptor = EngineTools::descriptorFor(type);
    if (!descriptor || !engine.engine || !engine.logger)
        return;

    auto *widget = descriptor->createWidget(engine, nullptr);
    widget->setAttribute(Qt::WA_DeleteOnClose);
    widget->setWindowTitle(descriptor->defaultTitle + QStringLiteral(": ") + engine.name);
    widget->resize(descriptor->defaultSize);
    widget->show();
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
        const auto engineId = settings.value(instancePrefix + QStringLiteral("engineId"), -1).toUInt(&validEngineId);
        if (!validEngineId)
            continue;
        const auto engine = RunningEnginesService::getInstance().getById(engineId);
        if (!engine.has_value())
            continue;
        const auto area = static_cast<Qt::DockWidgetArea>(settings.value(instancePrefix + QStringLiteral("dockArea"), static_cast<int>(Qt::RightDockWidgetArea)).toInt());
        if (addEmbeddedWithoutSaving(type.value(), engine.value(), area)) {
            const auto found = embedded.find(EngineTools::ToolInstanceKey{type.value(), engineId});
            if (found != embedded.end() && !found->second.isNull())
                found->second->setVisible(settings.value(instancePrefix + QStringLiteral("visible"), true).toBool());
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
        const auto instanceId = descriptor->stableId + QStringLiteral("_") + QString::number(key.engineId);
        const auto instancePrefix = QString::fromLatin1(settingsPrefix) + QStringLiteral("instances/") + instanceId + QStringLiteral("/");
        instances.append(instanceId);
        settings.setValue(instancePrefix + QStringLiteral("toolId"), descriptor->stableId);
        settings.setValue(instancePrefix + QStringLiteral("engineId"), key.engineId);
        settings.setValue(instancePrefix + QStringLiteral("dockArea"), static_cast<int>(mainWindow->dockWidgetArea(dock.data())));
        settings.setValue(instancePrefix + QStringLiteral("visible"), dock->isVisible());
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
        const auto engine = RunningEnginesService::getInstance().getById(key.engineId);
        const auto *descriptor = EngineTools::descriptorFor(key.type);
        Qt::DockWidgetArea area = (descriptor != nullptr) ? descriptor->defaultDockArea :  Qt::RightDockWidgetArea;
        if (engine.has_value())
            addEmbedded(key.type, engine.value(), area);
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

void ToolHostController::rebindEmbedded(unsigned int oldEngineId, const EngineServiceEntry &newEngine) {
    if (!newEngine.engine || !newEngine.logger)
        return;

    std::vector<std::pair<EngineTools::ToolInstanceKey, QPointer<DockToolHost>>> toRebind;
    for (const auto &[key, dock] : embedded) {
        if (key.engineId == oldEngineId && !dock.isNull())
            toRebind.emplace_back(key, dock);
    }

    for (const auto &[oldKey, oldDock] : toRebind) {
        if (oldDock.isNull())
            continue;

        const auto area = mainWindow->dockWidgetArea(oldDock.data());
        const auto visible = oldDock->isVisible();
        const auto floating = oldDock->isFloating();
        const auto geometry = oldDock->geometry();

        // Create through the normal registry factory so every tool gets the
        // same engine/logger wiring as a newly opened tool.
        if (!addEmbeddedWithoutSaving(oldKey.type, newEngine, area))
            continue;

        const auto newKey = EngineTools::ToolInstanceKey{oldKey.type, newEngine.id};
        const auto newDockIt = embedded.find(newKey);
        if (newDockIt != embedded.end() && !newDockIt->second.isNull()) {
            auto *newDock = newDockIt->second.data();
            newDock->setFloating(floating);
            if (floating)
                newDock->setGeometry(geometry);
            newDock->setVisible(visible);
        }

        embedded.erase(oldKey);
        oldDock->deleteLater();
    }

    if (!toRebind.empty()) {
        Q_EMIT embeddedToolsChanged();
        saveSettings();
    }
}
