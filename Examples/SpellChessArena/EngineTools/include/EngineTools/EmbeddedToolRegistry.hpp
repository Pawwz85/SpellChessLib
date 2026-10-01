#pragma once

#include <functional>
#include <optional>
#include <vector>

#include <QSize>
#include <QString>

#include "RunningEnginesService.hpp"
#include "EngineRolesService.hpp"

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
    EngineRole engineRole;

    friend constexpr bool operator<(const ToolInstanceKey &left, const ToolInstanceKey &right) {
        if (left.type != right.type)
            return static_cast<int>(left.type) < static_cast<int>(right.type);
        return static_cast<int>(left.engineRole) < static_cast<int>(right.engineRole);
    }

    friend constexpr bool operator==(const ToolInstanceKey &left, const ToolInstanceKey &right) {
        return left.type == right.type && left.engineRole == right.engineRole;
    }
};

struct ToolDescriptor {
    ToolTypeId id;
    QString stableId;
    QString displayName;
    QString defaultTitle;
    QSize defaultSize;
    std::function<QWidget *(EngineRole, QWidget *)> createWidget;
    Qt::DockWidgetArea defaultDockArea;

};

const std::vector<ToolDescriptor> &toolDescriptors();
const ToolDescriptor *descriptorFor(ToolTypeId id);
std::optional<ToolTypeId> toolTypeFromStableId(const QString &stableId);

} // namespace EngineTools
