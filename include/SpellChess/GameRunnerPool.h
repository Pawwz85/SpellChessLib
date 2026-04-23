#pragma once

#include <functional>
#include "PlayerEntry.h"
#include "GameMonitor.h"
#include "GameRunnerConfiguration.h"
#include "GameRunner.h"
#include "ThreadPool/ThreadPool.h"
#include "TimerService.h"
#include "Scheduler/GameScheduler.h"

namespace SpellChessLib {

	class GameRunnerPool {
	public:
	
		struct GameRunRequest {
			PlayerEntry whPlayer;
			PlayerEntry blPlayer;
			GameRunnerConfiguration runnerConfiguration;
			std::shared_ptr<GameMonitor> monitor;
			bool ignoreSimultaneousGamesPreferences;
			std::function<void(const Game &, const GameRunRequest &)> completionCallback;
			std::function<void(const GameRunRequest&, const std::exception &)> crashCallback;
		};
		std::shared_ptr<Scheduler::GameScheduler> scheduler;
	private:
		ThreadPool executorPool;
		std::shared_ptr<TimerService> timerService;

		std::function<void()> jobFromScheduler();
		std::function<void()> jobFromRequest(GameRunRequest&& request);
	public:
		GameRunnerPool(std::shared_ptr<TimerService> timerService, std::shared_ptr<Scheduler::GameScheduler> scheduler) : executorPool(1), timerService(timerService), 
			scheduler(scheduler) {};

		void addRequest(GameRunRequest && request);
		void setConcurrentGamesValue(size_t value);
		size_t getConcurrentGamesValue() const;
	};

}