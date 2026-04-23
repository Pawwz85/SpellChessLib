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
        "TestAttackers.cpp"
        "TestBoard.cpp"
        "TestGameRunner.cpp"
        "TestGameRunnerConfiguration.cpp"
        "TestMoveGen.cpp"
        "TestObserver.cpp"
        "TestPlayerTimerService.cpp"
        "TestSchedulerModule.cpp"
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


add_executable(SpellChessLibtTests pch.h pch.cpp ${all_files})

target_link_libraries(SpellChessLibtTests  GTest::gtest_main GTest::gmock uciloader spellchess)
include(GoogleTest)
gtest_discover_tests(SpellChessLibtTests)
