cmake_minimum_required(VERSION 3.16) 
project( EngineTools 
        LANGUAGES CXX)

set ( sources 
    "include/EngineTools/EmbeddedToolRegistry.hpp"
    "include/EngineTools/DockToolHost.hpp"
    "include/EngineTools/ToolHostController.hpp"
    "EngineControlPanel.cpp"
    "EngineLogger.cpp"
    "PrincipalVariationsPanel.cpp"
    "EvalThermometer.cpp"
    "RunningEnginesService.cpp"
    "EmbeddedToolRegistry.cpp"
    "DockToolHost.cpp"
    "ToolHostController.cpp"
)


qt_add_library(EngineTools STATIC ${sources})
target_compile_definitions( EngineTools PRIVATE QT_NO_EMIT) # Disable QT's emit keyword

target_link_libraries(EngineTools PUBLIC Misc Qt6::Widgets Qt6::Gui spellchess uciloader )
target_include_directories( EngineTools INTERFACE include )
target_include_directories( EngineTools PRIVATE include/EngineTools )

