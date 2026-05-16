#include <SpellChess/GameRunnerPool.h>
#include <SpellChess/UCIEnginePlayer.h>
#include <SpellChess/Scheduler/GameScheduler.h>
#include <SpellChess/MatchStatistics.h>
#include <SpellChess/SPRTTournament.h>
#include <SpellChess/GrimoireOpeningBook.h>
#include <SpellChess/OpeningBookPositionSupplier.h>

#include <iostream>
#include <vector>
#include <fstream>

#include <cxxopts.hpp>
#include "Configuration.h"
#include "Report.h"

using namespace SpellChessLib;

PlayerEntry makeEngineEntry(const std::string& path) {
	return {
		0,
		PlayerEntry::SIMULTANEOUS_GAMES_SUPPORTED,
		path,
		std::make_shared<UCIPlayerFactory>(path)
	};
}

int main(int argc, char ** argv) {
	srand(time(0));

	cxxopts::Options option("Runner", "A spellchess SPRT runner");

	option.add_options()
		("old", "Path to baseline engine", cxxopts::value<std::string>())
		("new", "Path to the engine being tested", cxxopts::value<std::string>())
		("O,opening_suite", "Path to grimoire opening book being used", cxxopts::value<std::string>())
		("o,output", "Path to output json file", cxxopts::value<std::string>())
		("a,alpha", "Desired probability of incorrently accepting null hypothesis", cxxopts::value<double>())
		("b,beta", "Desired probability of incorrently accepting alternative hypothesis", cxxopts::value<double>())
		("elo0", "Elo difference null hypothesis", cxxopts::value<int>())
		("elo1", "Elo difference alternative hypothesis", cxxopts::value<int>())
		("j,jobs", "Amount of games being run simoultanously", cxxopts::value<int>())
		("t,time", "base time allocated for whole game (in seconds)", cxxopts::value<double>()->default_value("10"))
		("i,increment", "increment to time added after every move (in seconds)", cxxopts::value<double>()->default_value("0.1"))
		("T,move_time", "time allocated for engine to move (in seconds)", cxxopts::value<double>())
	;
	option.parse_positional({"old", "new", "elo0", "elo1"});
	auto parsedOptions = option.parse(argc, argv);
	
	Runner::Configuration config = Runner::parseConfig(parsedOptions);
	auto validationResult = Runner::validateConfig(config);
	if (validationResult.has_value()) {
		std::cerr << validationResult.value();
		return EXIT_FAILURE;
	}

	std::shared_ptr<TimerService> timerService = std::make_shared<TimerService>();
	std::shared_ptr<Scheduler::GameScheduler> scheduler = std::make_shared<Scheduler::GameScheduler>();

	auto old = makeEngineEntry(config.oldEnginePath);
	auto new_ = makeEngineEntry(config.newEnginePath);

	PlayerRegistry->registerEntry(old);
	PlayerRegistry->registerEntry(new_);

	std::shared_ptr<GameRunnerPool> gameRunnerPool = std::make_shared<GameRunnerPool>(timerService, scheduler);
	gameRunnerPool->setConcurrentGamesValue(config.jobs);

	GrimoireOpeningBookLoader loader;
	SPRTPTournament tournament(gameRunnerPool, config.elo0, config.elo1, config.alpha, config.beta);

	if (!config.openingBook.empty()) {
		try {
			tournament.setPositionSupplier(std::make_shared<OpeningBookPositionSupplier>(SPELL_START_POS, loader.loadOpeningBook(config.openingBook)));
		} catch(std::exception e) {
			std::cerr << "Failed to set up opening suite";
			return EXIT_FAILURE;
		};
	}

	if(config.baseTime > 0)
		tournament.setTimeFormat(config.baseTime, config.increment);

	if (config.moveTime > 0)
		tournament.setMoveTime(config.moveTime);

	tournament.startTest(new_.id, old.id);
	auto result =	tournament.waitForCompletion();
	Runner::Report raport = Runner::makeReport(result, config);
	Runner::exportReport(raport, config.outputPath);
	return EXIT_SUCCESS;
}