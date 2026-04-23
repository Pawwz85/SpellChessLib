#include <SpellChess/GameRunnerPool.h>
#include <SpellChess/RandomPlayer.h>
#include <SpellChess/UCIEnginePlayer.h>
#include <SpellChess/Scheduler/GameScheduler.h>
#include <SpellChess/PGNExporter.h>
#include <SpellChess/PGN4Exporter.h>
#include <SpellChess/MatchStatistics.h>
#include <SpellChess/SPRTTournament.h>

#include <iostream>
#include <vector>
#include <fstream>
using namespace SpellChessLib;

PlayerEntry makeRandomPlayerEntry() {
	return {
		0,
		PlayerEntry::SIMULTANEOUS_GAMES_SUPPORTED,
		"Random Player",
		std::make_shared<RandomPlayer::FactoryClass>()
	};
}
PlayerEntry makeSpellChessEntry() {
	return {
		1,
		PlayerEntry::SIMULTANEOUS_GAMES_SUPPORTED,
		"SpellChess",
		std::make_shared<UCIPlayerFactory>("C:\\Users\\Pawwz\\source\\repos\\SpellChessEngine\\x64\\Release\\EngineExecutable.exe")
	};
}

PlayerEntry makeStockfishEntry() {
	return {
		2,
		PlayerEntry::SIMULTANEOUS_GAMES_SUPPORTED,
		"Stockfish",
		std::make_shared<UCIPlayerFactory>("D:\\Programs\\stockfish\\stockfish-windows-x86-64-avx2.exe")
	};
}

PlayerEntry makeEngineEntry(const std::string& path) {
	return {
		0,
		PlayerEntry::SIMULTANEOUS_GAMES_SUPPORTED,
		path,
		std::make_shared<UCIPlayerFactory>(path)
	};
}

const SpellChessMove& chooseNoSpellMove(const std::vector<MoveCandidate> & moves) {
	size_t index;

	do {
		index = rand() % moves.size();
	} while (moves[index].spellPolicy != UserCanAddSpell);
	return moves[index].move;
}

std::string makeRandomSpellChessOpening() {
	FenParser parser;
	GameState s = parser.fromSpellChessFen(SPELL_START_POS);
	MoveGenerator g1(s);
	auto moves = g1.generateMoves();
	s.makeMove(chooseNoSpellMove(moves));
	MoveGenerator g2(s);
	moves = g2.generateMoves();
	s.makeMove(chooseNoSpellMove(moves));
	return s.toFen();
}

GameRunnerPool::GameRunRequest makeRequest(std::function<void(const Game&, const GameRunnerPool::GameRunRequest & req)> clb, 
	std::function<void(const  GameRunnerPool::GameRunRequest& req, const std::exception&)> crash_callback,
	std::shared_ptr<GameMonitor> monitor) {
	GameRunnerConfigurationBuilder builder;
	builder.withEvent("Test Tournament");

	return  {
		makeSpellChessEntry(),
		makeStockfishEntry(),
		//builder.withPosition(START_POS).withMoveTimeLimit(1000).build(),
		builder.withPosition(makeRandomSpellChessOpening()).withMoveTimeLimit(300).build(),
		//builder.withPosition("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNB1KBNR b KQkq - 0 1").withMoveTimeLimit(500).build(),
		monitor,
		false,
		clb,
		crash_callback
	};
}

class GameLogger {
	std::string filename;
	std::shared_ptr<GameMonitor> monitor;
	std::ofstream file;

	class MoveConsumer : public Observer<std::vector<SpellChessMove>> {
		GameLogger* parent;
	public:
		MoveConsumer(GameLogger* parent) : parent(parent) {};

		// Odziedziczono za poœrednictwem elementu Observer
		void onObservedStateUpdate(const std::vector<SpellChessMove>& state) override {
			if(!state.empty())
				parent->file << spellChessMoveToString(state.back()) << "\n";
		}
	};

	class StateConsumer : public Observer<GameState*> {
		GameLogger* parent;
	public:
		StateConsumer(GameLogger* parent) : parent(parent) {};

		// Odziedziczono za poœrednictwem elementu Observer
	
		void onObservedStateUpdate(GameState* const& state) override {
			if (state)
				parent->file <<  "Fen: '"<< state->toFen() << "'\n";
		}

	};

	std::shared_ptr<MoveConsumer> mvCons;
	std::shared_ptr<StateConsumer> stCons;
public:

	GameLogger(std::shared_ptr<GameMonitor> monitor, std::string file) : filename(file), monitor(monitor), 
		mvCons(std::make_shared<MoveConsumer>(this)), stCons(std::make_shared<StateConsumer>(this)) {
		this->file.open(file);
		this->file << "" << "\n";
		monitor->getObservableMoves().addObserver(mvCons);
		monitor->getObservableGame().addObserver(stCons);
	};

	~GameLogger() {
		file.close();
		monitor->getObservableMoves().removeObserver(mvCons);
		monitor->getObservableGame().removeObserver(stCons);
	}

	std::shared_ptr<GameMonitor> getMonitor() {
		return monitor;
	}
};


class TournamentStatus {
	size_t gameCompleted = 0;
	size_t maxGames;
	mutable std::mutex lock;
	std::condition_variable con_var;

	int endingReason[11] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

	std::vector<std::shared_ptr<GameMonitor>> monitors;
	std::string score(const PlayerColor& c) {
		switch (c)
		{
		case Nobody:
			return "0 - 0";
		case White:
			return "1 - 0";
		case Black:
			return "0 - 1";
		default:
			break;
		}
		return "N/A";
	}
public:

	TournamentStatus(size_t max_games) : maxGames(max_games) {
		
	};

	void onGameCompletion(const Game & g, const GameRunnerPool::GameRunRequest & req) {
		std::unique_lock<std::mutex> guard(lock);
		++gameCompleted;
		endingReason[g.getStatusCode()]++;
		std::cout << req.whPlayer.name << " vs " << req.blPlayer.name << " " << score(g.getWinner()) <<'\n';
		std::cout << "Games completed: " << gameCompleted << "/" << maxGames << "\n";
		con_var.notify_one();

		auto annotations = g.getAnnotations();
		annotations.tagSection["Round"] = std::to_string(gameCompleted);
		annotations.tagSection["Result"] = Annotation::makeResultTag(g.getAnnotations().terminationReason);
	
		std::string saveLocation = "Games/game_" + std::to_string(gameCompleted);
		Annotation::exportAnnotatedGame(saveLocation, annotations);
	}

	void onCrash(const GameRunnerPool::GameRunRequest& req, const std::exception& exc) {
		std::unique_lock<std::mutex> guard(lock);
		++gameCompleted;
		endingReason[PlayerCrashed]++;
		std::cout << "Games nr "<< gameCompleted << " crashed because of the following exception " << exc.what()<<"\n";
	}

	void wait() {
		std::unique_lock<std::mutex> guard(lock);
		con_var.wait(guard, [this]() {return this->gameCompleted == maxGames; });
	}

	void displayEndingReasons() {
#define SHOW_REASON(reason) std::cout << #reason << ": "<< endingReason[reason] <<"\n"
		SHOW_REASON(StillOngoing);
		SHOW_REASON(IllegalMovePlayed);
		SHOW_REASON(CheckMate);
		SHOW_REASON(Stalemate);
		SHOW_REASON(InsufficientMaterial);
		SHOW_REASON(OpponentTimedOutWithoutSufficientMatingMaterial);
		SHOW_REASON(OpponentTimedOut);
		SHOW_REASON(DrawBy50MoveRule);
		SHOW_REASON(OpponentDisconnected);
		SHOW_REASON(KingCaptured);
		SHOW_REASON(PlayerCrashed);
	}
};

int main() {
	srand(time(0));
	std::shared_ptr<TimerService> timerService = std::make_shared<TimerService>();
	std::shared_ptr<Scheduler::GameScheduler> scheduler = std::make_shared<Scheduler::GameScheduler>();
	PlayerEntry entry = makeRandomPlayerEntry();
	PlayerRegistry->registerEntry(entry);

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
	


	SPRTPTournament tournament(gameRunnerPool, elo0, elo1);
	tournament.startTest(new_.id, old.id);
	auto result =	tournament.waitForCompletion();
	
	std::cout << "LLR - final = " << Stats::calculateLLR(result, elo0, elo1);

}