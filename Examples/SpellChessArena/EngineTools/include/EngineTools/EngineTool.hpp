#pragma once 

#include <memory>
#include <QObject>
#include <QPointer>

#include "RunningEnginesService.hpp"
#include "EngineRolesService.hpp"


namespace EngineTools {

    class EngineTool {
        std::shared_ptr<ChessEngine> currentEngine = nullptr;
        
        // Current engine connected to this tool was disconnected
        virtual void disconnectEngine() = 0;

        // Connect new engine
        virtual void connectEngine(const EngineServiceEntry & engineEntry) = 0;

        protected slots:
        
        void syncWithRole(EngineRole role) {
            auto entry = EngineRolesService::getInstance().getEngineByRole(role);
            if (entry.has_value())
                setEngine(entry.value());
        }

        public:

        EngineTool(EngineRole role, QPointer<QObject> self) {
            auto & service = EngineRolesService::getInstance();
            
            self->connect(&service, &EngineRolesService::roleChanged, self, [this, role](EngineRole role2){
                if (role2 == role)
                    this->syncWithRole(role);
            });
        };

        virtual ~EngineTool() = default;

        inline void setEngine(const EngineServiceEntry & newEngineEntry) {
            if (currentEngine)
                disconnectEngine();
            if ((currentEngine = newEngineEntry.engine))
                connectEngine(newEngineEntry);
        };
    };
}