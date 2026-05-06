cmake_minimum_required(VERSION 3.16)

project("SpellChessLib" LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(header_files
	"include/SpellChess/Board.h"
	"include/SpellChess/ControlFlowManager.h"
	"include/SpellChess/Factory.h"
	"include/SpellChess/GameAnnotations.h"
	"include/SpellChess/GameMonitor.h"
	"include/SpellChess/GameRunner.h"
	"include/SpellChess/GameRunnerConfiguration.h"
	"include/SpellChess/GameRunnerPool.h"
	"include/SpellChess/MatchStatistics.h"
	"include/SpellChess/MoveGen.h"
	"include/SpellChess/MoveLegality.h"
	"include/SpellChess/Observer.h"
	"include/SpellChess/PGNExporter.h"
	"include/SpellChess/PGN4Exporter.h"
	"include/SpellChess/Player.h"
	"include/SpellChess/PlayerEntry.h"
	"include/SpellChess/PlayerRegistry.h"
	"include/SpellChess/RandomPlayer.h"
	"include/SpellChess/SpellChessGame.h"
	"include/SpellChess/SpellChessUciLoaderPort.h"
	"include/SpellChess/SPRTTournament.h"
	"include/SpellChess/StringTokenizer.h"
	"include/SpellChess/TimerService.h"
	"include/SpellChess/UCIEnginePlayer.h"
	"include/SpellChess/Scheduler/GameScheduler.h"
	"include/SpellChess/Scheduler/ObjectAllocator.h"
	"include/SpellChess/Scheduler/ObjectAllocatorRepository.h"
	"include/SpellChess/Scheduler/PoolObjectAllocator.h"
	"include/SpellChess/ThreadPool/ThreadPool.h"
	"include/SpellChess/ThreadPool/WorkerThread.h"
)

set(source_files 
	"src/Board.cpp"
	"src/ControlFlowManager.cpp"
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
	"src/OpeningBookPositionSupplier.cpp"
	"src/PGNExporter.cpp"
	"src/PGN4Exporter.cpp"
	"src/Player.cpp"
	"src/PlayerRegistry.cpp"
	"src/PositionSupplier.cpp"
	"src/RandomPlayer.cpp"
	"src/SpellChessGame.cpp"
	"src/SpellChessUciLoaderPort.cpp"
	"src/SPRTTournament.cpp"
	"src/StringTokenizer.cpp"
	"src/ThreadPool.cpp"
	"src/TimerService.cpp"
	"src/UCIEnginePlayer.cpp"
	"src/ZobristHash.cpp"
)

set(all_files
	${header_files}
	${source_files}
)
add_library(spellchess STATIC ${all_files})

include(FetchContent)

FetchContent_Declare(
		UCILoader
		URL https://github.com/Pawwz85/UCILoader/archive/refs/heads/master.zip
)

FetchContent_MakeAvailable(UCILoader)

# find_package(UCILoader REQUIRED)

target_include_directories( spellchess PUBLIC include )
target_link_libraries(spellchess PUBLIC uciloader)

add_subdirectory("Examples/TournamentRunner")
add_subdirectory("Examples/SpellChessArena")
add_subdirectory("Examples/Grimoire")
add_subdirectory("UnitTests")
