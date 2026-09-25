cmake_minimum_required(VERSION 3.16)

project(PGN4Extract LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(core_sources
    "Feature.cpp"
    "PGN4Processor.cpp"
    "PGN4Supplier.cpp"
)

add_library(Pgn4ExtractorCore STATIC ${core_sources})
target_link_libraries(Pgn4ExtractorCore PUBLIC spellchess)
target_include_directories(Pgn4ExtractorCore PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../../ThirdParty)

add_executable(Pgn4Extract "main.cpp")
target_link_libraries(Pgn4Extract PRIVATE Pgn4ExtractorCore)
