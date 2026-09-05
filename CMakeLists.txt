cmake_minimum_required(VERSION 3.16)

project("SpellChessLib" LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(source_files 
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
add_library(spellchess STATIC ${source_files})

include(FetchContent)

FetchContent_Declare(
		UCILoader
		URL https://github.com/Pawwz85/UCILoader/archive/refs/tags/v.1.2.0.zip
)

FetchContent_MakeAvailable(UCILoader)

# find_package(UCILoader REQUIRED)

enable_testing()

target_include_directories( spellchess PUBLIC include )
target_include_directories( spellchess PRIVATE ThirdParty )
target_link_libraries(spellchess PUBLIC uciloader)


add_subdirectory("Examples/TournamentRunner")
add_subdirectory("Examples/SpellChessArena")
add_subdirectory("Examples/Grimoire")
add_subdirectory("Examples/Pgn4Generator")
add_subdirectory("UnitTests")
