#pragma once

#include "GameRunnerPool.h"
#include "MatchStatistics.h"
#include "PositionSupplier.h"
#include "Board.h"
#include "GameRunnerConfiguration.h"
#include <mutex>
#include <condition_variable>
#include <cstddef>
#include <memory>

namespace SpellChessLib {
	
	class Game;
	
	class SPRTPTournament {
	private:
		Stats::TrinomialModel tournamentModel;
		Stats::MatchOutcomeTables outcomeTables;

		size_t gamesIssued, gameCounter;
		std::shared_ptr<GameRunnerPool> runners;
		std::shared_ptr<PositionSupplier> supplier;
		int measuredPlayer = 0, opponent = 0;


		double alpha, beta, elo1, elo0;
		bool SPRTCompleted = false;
		GameRunnerConfigurationBuilder configBuilder;
		GameRunnerPool::GameRunRequest makeRequest(int whitePlayerId, int blackPlayerId, const GameState & root);

		void displayStats() const;
		void updateModel(const PlayerColor& winner, const GameRunnerPool::GameRunRequest& request);
		void onGameCompleted(const Game& game, const GameRunnerPool::GameRunRequest& request);
		void onGameCrashed(const GameRunnerPool::GameRunRequest& request);
		void spawnNewGameRequest();


	protected:
		mutable std::mutex lock;
		std::condition_variable con_var;

	public:
		SPRTPTournament(std::shared_ptr<GameRunnerPool> pool = nullptr, double elo0 = 0, double elo1 = 10, double alpha = 0.05, double beta = 0.05):
		alpha(alpha), beta(beta), elo0(elo0), elo1(elo1), gamesIssued(0), gameCounter(0), runners(pool), tournamentModel({0, 0, 0}),
			supplier(std::make_shared<RandomPositionSupplier>(SPELL_START_POS, 6))
		{
			configBuilder.withMoveTimeLimit(1000);
		}

		void startTest(int measuredPlayer, int opponent);
		
		void setPositionSupplier(std::shared_ptr<PositionSupplier> supplier) {
			this->supplier = supplier;
		};

		void setMoveTime(unsigned int moveTimeMs);
		void setTimeFormat(unsigned int baseTimeMs, unsigned int incMs, unsigned int margin);

		Stats::TrinomialModel waitForCompletion();

		const Stats::MatchOutcomeTables & getOutcomeTables() {
			return outcomeTables;
		};
	};

}