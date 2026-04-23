cmake_minimum_required(VERSION 3.16) 
project(BoardGrid 
        LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17) 
set(CMAKE_CXX_STANDARD_REQUIRED True)

set ( sources 
        "SpellCounterModel.cpp"
        "SpellCounterWidget.cpp"
        "SpellBar.cpp"
)


qt_add_library(SpellButton STATIC ${sources})

target_link_libraries(SpellButton PRIVATE Qt6::Widgets Qt6::Gui)
target_include_directories( SpellButton INTERFACE include )
target_include_directories( SpellButton PRIVATE include/SpellButton )

