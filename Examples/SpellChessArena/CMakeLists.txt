cmake_minimum_required(VERSION 3.16)

project(SpellChessArena LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set( headears
)

set ( src_files 
    "src/App.cpp"
    "src/EmptyBoardWidget.cpp"
    "src/EngineAnalisysConfigDialog.cpp"
    "src/EngineRepository.cpp"
    "src/Form.cpp"
    "src/GameRunnerConfigurationDialog.cpp"
    "src/LiveAnalisis.cpp"
    "src/LiveAnalisisEngineDialog.cpp"
    "src/main.cpp"
    "src/PastePGN4Dialog.cpp"
)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets Gui Concurrent)

qt_standard_project_setup()

set (all_files
    ${headers}
    ${src_files}
)

add_compile_definitions(QT_NO_EMIT)

add_subdirectory(Misc)
add_subdirectory(SpriteSet)
add_subdirectory(BoardGrid)
add_subdirectory(SpellButton)
add_subdirectory(EngineTools)
add_subdirectory(BoardViewer)

qt_add_executable( arena ${src_files} )

target_compile_definitions( arena PRIVATE QT_NO_EMIT) # Disable QT's emit keyword to avoid name conflict with UCI Loader
target_link_libraries( arena PRIVATE Qt6::Core Qt6::Widgets Qt6::Gui Qt6::Concurrent)
target_link_libraries( arena PRIVATE BoardGrid SpellButton SpriteSet BoardViewer spellchess Misc)
target_include_directories( arena PRIVATE include )

enable_testing()
# add_subdirectory(tests)

set_target_properties( arena PROPERTIES
    WIN32_EXECUTABLE ON
)
