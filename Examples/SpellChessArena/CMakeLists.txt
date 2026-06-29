cmake_minimum_required(VERSION 3.16)

project(SpellChessArena LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set( headears
)

set ( src_files 
    "src/BoardViewer/BoardLinter.cpp"
    "src/BoardViewer/SpellChessMoveBuilder.cpp"
    "src/BoardViewer/UserInteractionManager.cpp"
    "src/GameSession/OpenSessionRequest.cpp"
    "src/GameSession/GameRunnerBridge.cpp"
    "src/ChessGui.cpp"
    "src/EngineRepository.cpp"
    "src/Form.cpp"
    "src/GameRunnerConfigurationDialog.cpp"
    "src/GameSession.cpp"
    "src/EmptyBoardWidget.cpp"
    "src/App.cpp"
    "src/main.cpp"
    "src/MoveList.cpp"
    "src/PlayerTimer.cpp"
    "src/SpellPanel.cpp"
)

find_package(Qt6 REQUIRED COMPONENTS Widgets Gui  Concurrent)

qt_standard_project_setup()

set (all_files
    ${headers}
    ${src_files}
)

add_subdirectory(Misc)
add_subdirectory(SpriteSet)
add_subdirectory(BoardGrid)
add_subdirectory(SpellButton)

qt_add_executable( arena ${src_files} )

include(FetchContent)
set(JUST_INSTALL_CEREAL true) # we are not interested in performing unit test and performance comparison
FetchContent_Declare(
  cereal
  GIT_REPOSITORY https://github.com/USCiLab/cereal.git
  GIT_TAG        v1.3.2 # Latest stable as of 23.03.2026
)
FetchContent_MakeAvailable(cereal)

target_compile_definitions( arena PRIVATE QT_NO_EMIT) # Disable QT's emit keyword to avoid name conflict with UCI Loader
target_link_libraries( arena PRIVATE Qt6::Widgets Qt6::Gui Qt6::Concurrent)
target_link_libraries( arena PRIVATE BoardGrid SpellButton SpriteSet spellchess cereal::cereal Misc)
target_include_directories( arena PRIVATE include )

enable_testing()
# add_subdirectory(tests)

set_target_properties( arena PROPERTIES
    WIN32_EXECUTABLE ON
)
