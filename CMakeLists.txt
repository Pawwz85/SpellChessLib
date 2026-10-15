cmake_minimum_required(VERSION 3.28)

project("SpellChessLib" LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(source_files 
	"src/Arbiter.cpp"
	"src/AsynchronousPlayer.cpp"
	"src/Board.cpp"
	"src/ControlFlowManager.cpp"
	"src/DocumentAnnotator.cpp"
	"src/GameAnnotations.cpp"
	"src/GameMonitor.cpp"
	"src/GameRunner.cpp"
	"src/GameRunnerConfiguration.cpp"
	"src/GameRunnerPool.cpp"
	"src/GameScheduler.cpp"
	"src/GrimoireOpeningBook.cpp"
	"src/MatchStatistics.cpp"
	"src/MoveGen.cpp"
	"src/MoveLegality.cpp"
	"src/MoveNotation.cpp"
	"src/OpeningBookPlayerWrapper.cpp"
	"src/OpeningBookPositionSupplier.cpp"
	"src/PGNExporter.cpp"
	"src/PGN4Exporter.cpp"
	"src/PGN4Parser.cpp"
	"src/Player.cpp"
	"src/PlayerRegistry.cpp"
	"src/PositionSupplier.cpp"
	"src/RandomPlayer.cpp"
	"src/SpellChessDocument.cpp"
	"src/SpellChessDocumentHTMLRenderer.cpp"
	"src/SpellChessGame.cpp"
	"src/SpellChessUciLoaderPort.cpp"
	"src/SPRTTournament.cpp"
	"src/StringTokenizer.cpp"
	"src/ThreadPool.cpp"
	"src/TimerService.cpp"
	"src/UCIEnginePlayer.cpp"
	"src/UCIHelpers.cpp"
	"src/ZobristHash.cpp"
)

set(all_files
	${header_files}
	${source_files}
)

option (ENABLE_IWYU "Enable include-what-you-use" ON)

if(ENABLE_IWYU)
    find_program(IWYU_EXECUTABLE NAMES include-what-you-use)
    if(NOT IWYU_EXECUTABLE)
        message(FATAL_ERROR
            "ENABLE_IWYU is ON, but include-what-you-use was not found.")
    endif()

    set(CMAKE_CXX_INCLUDE_WHAT_YOU_USE
        "${IWYU_EXECUTABLE}")
endif()

add_library(spellchess STATIC ${source_files})

include(FetchContent)

FetchContent_Declare(
		UCILoader
		URL https://github.com/Pawwz85/UCILoader/archive/refs/tags/v1.2.3.zip
)

# UCILoader v1.2.1 installs UCILoaderConfig.cmake using a relative source
# path, although configure_package_config_file() generates it in the build
# directory. Patch the fetched project before adding it so its install rule
# references the generated file.
FetchContent_GetProperties(UCILoader)
if(NOT uciloader_POPULATED)
	FetchContent_Populate(UCILoader)
endif()

file(READ "${uciloader_SOURCE_DIR}/CMakeLists.txt" _uciloader_cmake_lists)
string(REPLACE
	"install( FILES \"UCILoaderConfig.cmake\" \"\${CMAKE_CURRENT_BINARY_DIR}/UCILoaderConfigVersion.cmake\""
	"install( FILES \"\${CMAKE_CURRENT_BINARY_DIR}/UCILoaderConfig.cmake\" \"\${CMAKE_CURRENT_BINARY_DIR}/UCILoaderConfigVersion.cmake\""
	_uciloader_cmake_lists
	"${_uciloader_cmake_lists}"
)
file(WRITE "${uciloader_SOURCE_DIR}/CMakeLists.txt" "${_uciloader_cmake_lists}")

add_subdirectory("${uciloader_SOURCE_DIR}" "${uciloader_BINARY_DIR}")
unset(_uciloader_cmake_lists)

# find_package(UCILoader REQUIRED)

enable_testing()

target_include_directories( spellchess PUBLIC include )
target_include_directories( spellchess PRIVATE ThirdParty )
target_link_libraries(spellchess PUBLIC uciloader)


add_subdirectory("Examples/TournamentRunner")
add_subdirectory("Examples/Grimoire")
add_subdirectory("Examples/Pgn4Generator")
add_subdirectory("Examples/Pgn4FenExtractor")
add_subdirectory("Examples/TransitionScoreExporter")
add_subdirectory("UnitTests")
