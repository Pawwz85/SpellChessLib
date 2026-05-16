cmake_minimum_required(VERSION 3.16)

project(Pgn4Generator LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(sources
    "main.cpp"
)

add_executable( Pgn4Generator ${sources})

target_link_libraries( Pgn4Generator spellchess)
target_include_directories( Pgn4Generator PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../../ThirdParty)