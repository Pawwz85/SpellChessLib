cmake_minimum_required(VERSION 3.16)

project(IcecreamMoveGeneratorTester LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable(icecream_move_generator_tester
    TestCase.cpp
    main.cpp
)

target_link_libraries(icecream_move_generator_tester PRIVATE spellchess)
target_include_directories(icecream_move_generator_tester PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../ThirdParty
)