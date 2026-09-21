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
    EngineControlPanel,
    EvalThermomether
};

struct ToolInstanceKey {
    ToolTypeId type;
    unsigned int engineId;

    friend constexpr bool operator<(const ToolInstanceKey &left, const ToolInstanceKey &right) {
        if (left.type != right.type)
            return static_cast<int>(left.type) < static_cast<int>(right.type);
        return left.engineId < right.engineId;
    }

    friend constexpr bool operator==(const ToolInstanceKey &left, const ToolInstanceKey &right) {
        return left.type == right.type && left.engineId == right.engineId;
    }
};

struct ToolDescriptor {
    ToolTypeId id;
    QString stableId;
    QString displayName;
    QString defaultTitle;
    QSize defaultSize;
    std::function<QWidget *(const EngineServiceEntry &, QWidget *)> createWidget;
    Qt::DockWidgetArea defaultDockArea;

};

const std::vector<ToolDescriptor> &toolDescriptors();
const ToolDescriptor *descriptorFor(ToolTypeId id);
std::optional<ToolTypeId> toolTypeFromStableId(const QString &stableId);

} // namespace EngineTools
