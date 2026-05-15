cmake_minimum_required(VERSION 3.16)

project(Grimoire LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(sources
    "Configuration.cpp"
    "GameNode.cpp"
    "OpeningBookEntry.cpp"
    "main.cpp"
)

add_executable( grimoire ${sources})

target_link_libraries( grimoire spellchess)
target_include_directories( grimoire PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../../ThirdParty)