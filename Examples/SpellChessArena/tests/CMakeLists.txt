cmake_minimum_required(VERSION 3.16) 
project(UnitTests 
        LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17) # gtest requirement
set(CMAKE_CXX_STANDARD_REQUIRED True)

set ( test_files 
        "Core/TestObserver.cpp"
        "Core/TestMVC.cpp"
)

set ( all_files 
    ${test_files}
)

find_package(GTest)

add_executable(UnitTests  ${all_files})
target_include_directories( UnitTests PUBLIC ../include )

target_link_libraries(UnitTests  GTest::gtest_main)
include(GoogleTest)

gtest_discover_tests(UnitTests)
