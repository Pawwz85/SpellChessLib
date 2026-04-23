cmake_minimum_required(VERSION 3.16) 
project(BoardGrid 
        LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17) 
set(CMAKE_CXX_STANDARD_REQUIRED True)

set ( sources 
        "BoardEntity.cpp"
        "BoardGridModel.cpp"
        "BoardGridWidget.cpp"
        "DragArea.cpp"
        "EntityInitializers.cpp"
        "MouseDragDetector.cpp"
        "PieceSpriteSet.cpp"
        "PromotionContextMenuOverlay.cpp"
)


qt_add_library(BoardGrid STATIC ${sources})

target_link_libraries(BoardGrid PRIVATE Misc Qt6::Widgets Qt6::Gui SpriteSet spellchess)
target_include_directories( BoardGrid INTERFACE include )
target_include_directories( BoardGrid PRIVATE include/BoardGrid )

