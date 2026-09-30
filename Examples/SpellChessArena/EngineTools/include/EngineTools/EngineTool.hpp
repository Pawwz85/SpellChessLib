#pragma once 

#include <memory>

#include "RunningEnginesService.hpp"

namespace EngineTools {

    class EngineTool {
        std::shared_ptr<ChessEngine> currentEngine = nullptr;
        
        // Current engine connected to this tool was disconnected
        virtual void disconnectEngine() = 0;

        // Connect new engine
        virtual void connectEngine(const EngineServiceEntry & engineEntry) = 0;

        public:
        virtual ~EngineTool() = default;

        inline void setEngine(const EngineServiceEntry & newEngineEntry) {
            if (currentEngine)
                disconnectEngine();
            if ((currentEngine = newEngineEntry.engine))
                connectEngine(newEngineEntry);
        };
    };
}