add_executable(pgn4_position_insights_exporter main.cpp)

target_link_libraries(pgn4_position_insights_exporter PRIVATE spellchess)
target_include_directories(pgn4_position_insights_exporter PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../ThirdParty
)
