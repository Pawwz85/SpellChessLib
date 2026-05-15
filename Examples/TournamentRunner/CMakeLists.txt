cmake_minimum_required(VERSION 3.16)

project(TournamentRunner LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(sources
    "Configuration.cpp"
    "Report.cpp"
    "main.cpp"
)

add_executable( runner ${sources} )

target_link_libraries( runner spellchess)
target_include_directories( runner PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../../ThirdParty) # for cxxopts.hpp

# install(FILES book.spell DESTINATION "/Examples/TournamentRunner")