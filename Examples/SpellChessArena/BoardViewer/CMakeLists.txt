cmake_minimum_required(VERSION 3.16) 
project( BoardViewer 
        LANGUAGES CXX)

set ( sources 
    "BoardLinter.cpp"
    "BoardViewerModel.cpp"
    "BoardViewerWidget.cpp"
    "MoveList.cpp"
    "GameRunnerBridge.cpp"
    "GameViewerModel.cpp"
    "OpenSessionRequest.cpp"
    "PlayerTimer.cpp"
    "SpellChessMoveBuilder.cpp"
    "SpellPanel.cpp"
    "UserInteractionManager.cpp"
)


qt_add_library(BoardViewer STATIC ${sources})

target_link_libraries(BoardViewer PUBLIC Misc Qt6::Widgets Qt6::Gui spellchess BoardGrid SpriteSet EngineTools)
target_include_directories( BoardViewer INTERFACE include )
target_include_directories( BoardViewer PRIVATE include/BoardViewer )

