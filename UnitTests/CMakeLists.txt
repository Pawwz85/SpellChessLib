cmake_minimum_required(VERSION 3.16) 
project(UnitTests 
        LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17) # gtest requirement
set(CMAKE_CXX_STANDARD_REQUIRED True)

set ( header_files
    "ThreadPoolUtils.h"
    "utils.h"
)

set ( test_files 
        "test.cpp"
        "TestAnnotations.cpp"
        "TestArbiter.cpp"
        "TestAsynchronousPlayer.cpp"
        "TestAttackers.cpp"
        "TestBoard.cpp"
        "TestGameRunner.cpp"
        "TestGameRunnerConfiguration.cpp"
        "TestMoveGen.cpp"
        "TestObserver.cpp"
        "TestPGN4Parser.cpp"
        "TestPgnExporter.cpp"
        "TestPromotionContextMenuOverlay.cpp"
        "TestPlayerTimerService.cpp"
        "TestSchedulerModule.cpp"
        "TestSpellChessDocument.cpp"
        "TestThreadPool.cpp"
        "TestWorkerTHread.cpp"
)

set ( all_files 
    ${header_files}
    ${test_files}
)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.17.0.zip
)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)


add_executable(SpellChessLibTests pch.h pch.cpp ${all_files})

add_custom_command(TARGET SpellChessLibTests POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_CURRENT_SOURCE_DIR}/assets/ $<TARGET_FILE_DIR:SpellChessLibTests>)

find_package(Qt6 REQUIRED COMPONENTS Widgets)
target_link_libraries(SpellChessLibTests  GTest::gtest_main GTest::gmock uciloader spellchess BoardGrid Qt6::Widgets)
include(GoogleTest)
gtest_discover_tests(SpellChessLibTests)
