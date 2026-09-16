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

bool ToolHostController::hasEmbedded(EngineTools::ToolTypeId type) const {
    const auto found = embedded.find(type);
    return found != embedded.end() && !found->second.isNull();
}

bool ToolHostController::addEmbedded(EngineTools::ToolTypeId type, const EngineServiceEntry &engine) {
    return addEmbedded(type, engine, Qt::RightDockWidgetArea);
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
        if (hasEmbedded(type))
        return false;

    const auto *descriptor = EngineTools::descriptorFor(type);
    if (!descriptor || !engine.engine || !engine.logger)
        return false;

    auto *dock = new DockToolHost(type, descriptor->defaultTitle, mainWindow);
    dock->setWidget(descriptor->createWidget(engine, dock));
    dock->resize(descriptor->defaultSize);
    dock->setWindowTitle(descriptor->defaultTitle + QStringLiteral(": ") + engine.name);
    dock->setProperty("engineId", static_cast<uint>(engine.id));
    connect(dock, &DockToolHost::toolClosed, this, &ToolHostController::handleDockClosed);
    dock->installEventFilter(this);
    mainWindow->addDockWidget(area, dock);
    embedded[type] = dock;
    Q_EMIT embeddedToolsChanged();
    return true;
}


void ToolHostController::handleDockClosed(EngineTools::ToolTypeId type) {
    embedded.erase(type);
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

bool ToolHostController::removeEmbedded(EngineTools::ToolTypeId type) {
    const auto found = embedded.find(type);
    if (found == embedded.end() || found->second.isNull())
        return false;

    auto *dock = found->second.data();
    embedded.erase(found);
    dock->deleteLater();
    Q_EMIT embeddedToolsChanged();
    saveSettings();
    return true;
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
    const auto types = settings.value(QString::fromLatin1(settingsPrefix) + QStringLiteral("embeddedTypes")).toStringList();

    for (const auto &stableId : types) {
        const auto type = EngineTools::toolTypeFromStableId(stableId);
        if (!type.has_value())
            continue;
        const auto engineId = settings.value(QString::fromLatin1(settingsPrefix) + QStringLiteral("tools/") + stableId + QStringLiteral("/engineId"), -1).toUInt();
        const auto engine = RunningEnginesService::getInstance().getById(engineId);
        if (!engine.has_value())
            continue;
        addEmbeddedWithoutSaving(type.value(), engine.value(), Qt::RightDockWidgetArea);
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

    QStringList types;
    for (const auto &[type, dock] : embedded) {
        if (dock.isNull())
            continue;
        const auto *descriptor = EngineTools::descriptorFor(type);
        if (!descriptor)
            continue;
        types.append(descriptor->stableId);
        settings.setValue(QString::fromLatin1(settingsPrefix) + QStringLiteral("tools/") + descriptor->stableId + QStringLiteral("/engineId"),
                          dock->property("engineId"));
        settings.setValue(QString::fromLatin1(settingsPrefix) + QStringLiteral("tools/") + descriptor->stableId + QStringLiteral("/visible"), dock->isVisible());
    }
    settings.setValue(QString::fromLatin1(settingsPrefix) + QStringLiteral("embeddedTypes"), types);
    settings.setValue(QString::fromLatin1(settingsPrefix) + QStringLiteral("hostState"), mainWindow->saveState());
    settings.setValue(QString::fromLatin1(settingsPrefix) + QStringLiteral("geometry"), mainWindow->saveGeometry());
    settings.sync();
}

void ToolHostController::resetLayout() {
    const auto activeTypes = std::vector<EngineTools::ToolTypeId>([this] {
        std::vector<EngineTools::ToolTypeId> result;
        for (const auto &[type, dock] : embedded)
            if (!dock.isNull())
                result.push_back(type);
        return result;
    }());

    for (const auto type : activeTypes) {
        const auto engineId = embedded[type]->property("engineId").toUInt();
        removeEmbedded(type);
        const auto engine = RunningEnginesService::getInstance().getById(engineId);
        if (engine.has_value())
            addEmbedded(type, engine.value(), Qt::RightDockWidgetArea);
    }
    saveSettings();
}

void ToolHostController::forwardAnalysisContext(const SpellChessLib::GameState &context) {
    for (const auto &[type, dock] : embedded) {
        if (dock.isNull())
            continue;
        if (auto *panel = qobject_cast<PrincipalVariationPanel *>(dock->widget()))
            panel->contextChanged(context);
    }
}
