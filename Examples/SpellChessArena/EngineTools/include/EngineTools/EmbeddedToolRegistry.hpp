#pragma once

#include <functional>
#include <optional>
#include <vector>

#include <QSize>
#include <QString>

#include "RunningEnginesService.hpp"

class QWidget;

namespace EngineTools {

enum class ToolTypeId {
    PrincipalVariations,
    EngineLogs,
    EngineControlPanel
};

struct ToolDescriptor {
    ToolTypeId id;
    QString stableId;
    QString displayName;
    QString defaultTitle;
    QSize defaultSize;
    std::function<QWidget *(const EngineServiceEntry &, QWidget *)> createWidget;
};

const std::vector<ToolDescriptor> &toolDescriptors();
const ToolDescriptor *descriptorFor(ToolTypeId id);
std::optional<ToolTypeId> toolTypeFromStableId(const QString &stableId);

} // namespace EngineTools
