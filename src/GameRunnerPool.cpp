#include <SpellChess/GameRunnerPool.h>
#include <memory>

inline void updateSevenTagRooster(SpellChessLib::GameRunner& runner, const SpellChessLib::GameRunnerPool::GameRunRequest & request) {
	auto roster = runner.getGame().getRoster();
	SpellChessLib::Annotation::addPlayerTagsToRoster(roster, request.whPlayer.name, request.blPlayer.name);
	runner.getGame().setSevenTagRooster(roster);
}
std::function<void()> SpellChessLib::GameRunnerPool::jobFromScheduler()
{
	auto timerServicePointer = this->timerService;
	auto schedulerPointer = this->scheduler;
	return [timerServicePointer, schedulerPointer]() {
		std::unique_ptr<Scheduler::GameSchedulerEntry> scheduledGameEntry = nullptr;
		do {
			scheduledGameEntry = std::move(schedulerPointer->getNextGame());
			if (scheduledGameEntry == nullptr)
				std::this_thread::sleep_for(SCHEDULER_POLL_INTERVAL);
		} while (scheduledGameEntry == nullptr);
		
		auto request = std::unique_ptr<GameRunRequest>((GameRunRequest*)scheduledGameEntry->context);
		
		try {
			Scheduler::ResourcePointer<Player> whPlayer = schedulerPointer->getPlayerInstance(scheduledGameEntry->whitePlayerId);
			Scheduler::ResourcePointer<Player> blPlayer = schedulerPointer->getPlayerInstance(scheduledGameEntry->blackPlayerId);
			GameRunner runner(timerServicePointer, request->runnerConfiguration, std::move(whPlayer), std::move(blPlayer));
			updateSevenTagRooster(runner, *request);
			if (request->monitor != nullptr)
				runner.attachGameMonitor(request->monitor);
			runner.run();
			request->completionCallback(runner.getGame(), *request);
		}
		catch (const std::exception & e) {
			request->crashCallback(*request, e);
		}
	};
}

std::function<void()> SpellChessLib::GameRunnerPool::jobFromRequest(GameRunRequest&& request)
{
	auto timerServicePointer = this->timerService;
	return [request = std::move(request), timerServicePointer]() {		
		try {
			auto whPlayerAlloc = std::make_shared<Scheduler::DefaultObjectAllocator<Player>>(request.whPlayer.instanceFactory);
			auto blPlayerAlloc = std::make_shared<Scheduler::DefaultObjectAllocator<Player>>(request.blPlayer.instanceFactory);
			GameRunner runner(timerServicePointer, request.runnerConfiguration, whPlayerAlloc->retrieve(), blPlayerAlloc->retrieve());
			updateSevenTagRooster(runner, request);
			if (request.monitor != nullptr)
				runner.attachGameMonitor(request.monitor);
			runner.run();
			request.completionCallback(runner.getGame(), request);
		}
		catch (const std::exception& e) {
			request.crashCallback(request, e);
		}
		
	};
}

void SpellChessLib::GameRunnerPool::addRequest(GameRunRequest&& request) {
	if (request.ignoreSimultaneousGamesPreferences)
		executorPool.forceTaskExecution(jobFromRequest(std::move(request)));
	else {
		scheduler->emplaceEntry(request.whPlayer.id, request.blPlayer.id, new GameRunRequest(request));
		executorPool.queueTask(jobFromScheduler());
	}
		
}

void SpellChessLib::GameRunnerPool::setConcurrentGamesValue(size_t value) {
	executorPool.setDesiredSize(value);
}

size_t SpellChessLib::GameRunnerPool::getConcurrentGamesValue() const {
	return executorPool.getDesiredSize();
}

size_t SpellChessLib::GameRunnerPool::countGamesBeingRunned() const{
	return executorPool.countBusyWorkers();
}
