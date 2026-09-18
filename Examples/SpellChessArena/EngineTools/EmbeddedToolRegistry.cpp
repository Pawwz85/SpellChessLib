#include <EmbeddedToolRegistry.hpp>

#include <EngineControlPanel.hpp>
#include <EngineLogger.hpp>
#include <PrincipalVariationsPanel.hpp>
#include <EvalThermometer.hpp>

namespace EngineTools {

const std::vector<ToolDescriptor> &toolDescriptors() {
    static const std::vector<ToolDescriptor> descriptors = {
        {ToolTypeId::PrincipalVariations, QStringLiteral("principalVariations"),
         QStringLiteral("Principal Variations"), QStringLiteral("Engine variations"), QSize(400, 300),
         [](const EngineServiceEntry &entry, QWidget *parent) { return new PrincipalVariationPanel(entry.engine, 3, parent); },
         Qt::DockWidgetArea::TopDockWidgetArea},
        {ToolTypeId::EngineLogs, QStringLiteral("engineLogs"),
         QStringLiteral("Engine Logs"), QStringLiteral("Engine logs"), QSize(800, 600),
         [](const EngineServiceEntry &entry, QWidget *parent) { return new EngineLogsWidget(*entry.logger, parent); },
         Qt::DockWidgetArea::RightDockWidgetArea},
        {ToolTypeId::EngineControlPanel, QStringLiteral("engineControlPanel"),
         QStringLiteral("Engine Options"), QStringLiteral("Engine options"), QSize(800, 600),
         [](const EngineServiceEntry &entry, QWidget *parent) { return new EngineControlPanel(entry.engine, parent); },
        Qt::DockWidgetArea::RightDockWidgetArea},
         {ToolTypeId::EvalThermomether, QStringLiteral("evalThermometer"),
         QStringLiteral("Evaluation Thermometer"), QStringLiteral("Evaluation thermometer"), QSize(120, 420),
         [](const EngineServiceEntry &entry, QWidget *parent) { return new EvalThermometer(entry.engine, parent); },
        Qt::DockWidgetArea::LeftDockWidgetArea}

    };
    return descriptors;
}

const ToolDescriptor *descriptorFor(ToolTypeId id) {
    for (const auto &descriptor : toolDescriptors())
        if (descriptor.id == id)
            return &descriptor;
    return nullptr;
}

std::optional<ToolTypeId> toolTypeFromStableId(const QString &stableId) {
    for (const auto &descriptor : toolDescriptors())
        if (descriptor.stableId == stableId)
            return descriptor.id;
    return std::nullopt;
}

} // namespace EngineTools
