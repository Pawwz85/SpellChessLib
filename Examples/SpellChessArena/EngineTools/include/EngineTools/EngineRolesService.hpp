#pragma once

#include <optional>

#include <QObject>
#include <QString>

#include "RunningEnginesService.hpp"

namespace EngineTools {

 enum EngineRole {
    LiveAnalisisEngine,
    WhiteSideEngine,
    BlackSideEngine,
    EngineRoleEnum_Size
 }; 

 QString engineRoleToQString(EngineRole role);

 class EngineRolesService : public QObject {
    Q_OBJECT
    int roleToEngineIdMap [EngineRoleEnum_Size];
    EngineRolesService();

    public:
    void assignEngineToRole(EngineRole role, EngineServiceEntry entry);
    void clearRole(EngineRole role);
    std::optional<EngineServiceEntry> getEngineByRole(EngineRole role);

    static EngineRolesService & getInstance();

    signals:
    void roleChanged(EngineTools::EngineRole role);
 };
}