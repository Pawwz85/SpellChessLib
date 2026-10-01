#include <EngineRolesService.hpp>
#include <QtTranslation>

namespace EngineTools {
    
    EngineRolesService::EngineRolesService(){
        for (size_t i = 0; i < EngineRoleEnum_Size; ++i)
            roleToEngineIdMap[i] = -1;
    }

     QString engineRoleToQString(EngineRole role) {
        switch (role)
        {
        case LiveAnalisisEngine:
            return QObject::tr("Live Analisis", "Engine role");
        case WhiteSideEngine:
            return QObject::tr("White", "Engine role");
        case BlackSideEngine:
            return QObject::tr("Black", "Engine role");
        default:
            break;
        }

        return "";
     }


    void EngineRolesService::assignEngineToRole(EngineRole role, EngineServiceEntry entry) {
        roleToEngineIdMap[role] = entry.id;
        roleChanged(role);
    }

    void EngineRolesService::clearRole(EngineRole role) {
        roleToEngineIdMap[role] = -1;
        roleChanged(role);
    }

    std::optional<EngineServiceEntry> EngineRolesService::getEngineByRole(EngineRole role) {
        int engineId = roleToEngineIdMap[role];

        if (engineId == -1)
            return std::nullopt;

        return RunningEnginesService::getInstance().getById(engineId);
    }

    EngineRolesService & EngineRolesService::getInstance() {
        static EngineRolesService instance;
        return instance;
    };

}

#include <moc_EngineRolesService.cpp>