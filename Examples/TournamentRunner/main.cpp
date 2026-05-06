#include <SpellChess/GameRunnerPool.h>
#include <SpellChess/RandomPlayer.h>
#include <SpellChess/UCIEnginePlayer.h>
#include <SpellChess/Scheduler/GameScheduler.h>
#include <SpellChess/PGNExporter.h>
#include <SpellChess/PGN4Exporter.h>
#include <SpellChess/MatchStatistics.h>
#include <SpellChess/SPRTTournament.h>
#include <SpellChess/GrimoireOpeningBook.h>
#include <SpellChess/OpeningBookPositionSupplier.h>

#include <iostream>
#include <vector>
#include <fstream>
using namespace SpellChessLib;

PlayerEntry makeEngineEntry(const std::string& path) {
	return {
		0,
		PlayerEntry::SIMULTANEOUS_GAMES_SUPPORTED,
		path,
		std::make_shared<UCIPlayerFactory>(path)
	};
}

int main() {
	srand(time(0));
	std::shared_ptr<TimerService> timerService = std::make_shared<TimerService>();
	std::shared_ptr<Scheduler::GameScheduler> scheduler = std::make_shared<Scheduler::GameScheduler>();

	std::string oldEngine;
	std::string newEngine;

	std::cout << "Enter the path to old engine: ";
	std::cin >> oldEngine;
	auto old = makeEngineEntry(oldEngine);
	std::cout << std::endl;
	std::cout << "Enter the path to new engine: ";
	std::cin >> newEngine;
	auto new_ = makeEngineEntry(newEngine);
	std::cout << std::endl;

	PlayerRegistry->registerEntry(old);
	PlayerRegistry->registerEntry(new_);

	double elo0 = -10, elo1 = 0;
	std::cout << "Enter elo0: ";
	std::cin >> elo0;
	std::cout << std::endl;
	std::cout << "Enter elo1: ";
	std::cin >> elo1;
	std::cout << std::endl;


	std::shared_ptr<GameRunnerPool> gameRunnerPool = std::make_shared<GameRunnerPool>(timerService, scheduler);

	gameRunnerPool->setConcurrentGamesValue(10);

	GrimoireOpeningBookLoader loader;
	SPRTPTournament tournament(gameRunnerPool, elo0, elo1);
	tournament.setPositionSupplier(std::make_shared<OpeningBookPositionSupplier>(SPELL_START_POS, loader.loadOpeningBook("book.spell")));
	tournament.startTest(new_.id, old.id);
	auto result =	tournament.waitForCompletion();
	
	std::cout << "LLR - final = " << Stats::calculateLLR(result, elo0, elo1);

}