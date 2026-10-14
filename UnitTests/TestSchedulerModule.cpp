#include <gtest/gtest.h>

#include <SpellChess/Scheduler/ObjectAllocator.h>
#include <SpellChess/Scheduler/PoolObjectAllocator.h>
#include <SpellChess/Scheduler/ObjectAllocatorRepository.h>
#include <SpellChess/Scheduler/GameScheduler.h>
#include <SpellChess/PlayerRegistry.h>
#include <SpellChess/PlayerEntry.h>
#include <SpellChess/Factory.h>
#include <SpellChess/Player.h>
#include <SpellChess/Board.h>

#include <memory>
#include <chrono>
#include <cstddef>
#include <list>
#include <mutex>
#include <utility>
#include <vector>

using namespace SpellChessLib;


class IntegerFactory : public Factory<int> {
	// Odziedziczono za po�rednictwem elementu Factory
	std::unique_ptr<int> makeObject() override
	{
		return std::unique_ptr<int>(new int);
	}
};

TEST(defaultObjectAllocator, alloc) {
	std::shared_ptr<Factory<int>> exampleFactory = std::make_shared<IntegerFactory>();
	auto someAllocator = std::make_shared<Scheduler::DefaultObjectAllocator<int>>(exampleFactory);
	ASSERT_NO_THROW(Scheduler::ResourcePointer<int> ptr = std::move(someAllocator->retrieve()));
}

TEST(poolObjectAllocator, alloc) {
	std::shared_ptr<Factory<int>> exampleFactory = std::make_shared<IntegerFactory>();
	auto someAllocator = std::make_shared<Scheduler::ObjectPoolAllocator<int>>(exampleFactory);
	ASSERT_NO_THROW(Scheduler::ResourcePointer<int> ptr = std::move(someAllocator->retrieve()));
}

TEST(poolObjectAllocator, newObjects) {
	std::shared_ptr<Factory<int>> exampleFactory = std::make_shared<IntegerFactory>();
	auto someAllocator = std::make_shared<Scheduler::ObjectPoolAllocator<int>>(exampleFactory);
	Scheduler::ResourcePointer<int> ptr1 = nullptr, ptr2 = nullptr;
	ptr1 = std::move(someAllocator->retrieve());
	ptr2 = std::move(someAllocator->retrieve());
	ASSERT_NE(ptr1, ptr2);
	ASSERT_EQ(someAllocator->getAliveObjectsCount(), 2);
	ASSERT_EQ(someAllocator->getObjectPoolSize(), 0);
}

TEST(poolObjectAllocator, objectReuse) {
	std::shared_ptr<Factory<int>> exampleFactory = std::make_shared<IntegerFactory>();
	auto someAllocator = std::make_shared<Scheduler::ObjectPoolAllocator<int>>(exampleFactory);
	Scheduler::ResourcePointer<int> ref1 = nullptr, ref2 = nullptr;
	int* ptr1, * ptr2;
	ref1 = someAllocator->retrieve();
	ptr1 = &*ref1;
	ref1.reset();
	ASSERT_EQ(someAllocator->getAliveObjectsCount(), 1);
	ASSERT_EQ(someAllocator->getObjectPoolSize(), 1);
	ref2 = someAllocator->retrieve();
	ptr2 = &*ref2;
	ASSERT_EQ(ptr1, ptr2);
	ASSERT_EQ(someAllocator->getAliveObjectsCount(), 1);
	ASSERT_EQ(someAllocator->getObjectPoolSize(), 0);	
}

TEST(poolObjectAllocator, shrinkingObjectPool) {
	std::shared_ptr<Factory<int>> exampleFactory = std::make_shared<IntegerFactory>();
	auto someAllocator = std::make_shared<Scheduler::ObjectPoolAllocator<int>>(exampleFactory);
	std::vector<Scheduler::ResourcePointer<int>> resources;
	
	for (int i = 0; i < 10; ++i) 
		resources.emplace_back(std::move(someAllocator->retrieve()));

	for (auto & p  : resources)
		p.reset();

	ASSERT_EQ(someAllocator->getObjectPoolSize(), 10);
	someAllocator->shrinkPool(1);
	ASSERT_EQ(someAllocator->getObjectPoolSize(), 1);
}

TEST(poolObjectAllocator, clearPool) {
	std::shared_ptr<Factory<int>> exampleFactory = std::make_shared<IntegerFactory>();
	auto someAllocator = std::make_shared<Scheduler::ObjectPoolAllocator<int>>(exampleFactory);
	std::vector<Scheduler::ResourcePointer<int>> resources;

	for (int i = 0; i < 10; ++i)
		resources.emplace_back(std::move(someAllocator->retrieve()));

	for (auto& p : resources)
		p.reset();

	ASSERT_EQ(someAllocator->getObjectPoolSize(), 10);
	someAllocator->clearPool();
	ASSERT_EQ(someAllocator->getObjectPoolSize(), 0);
}

TEST(poolObjectAllocator, setMaxSize) {
	std::shared_ptr<Factory<int>> exampleFactory = std::make_shared<IntegerFactory>();
	auto someAllocator = std::make_shared<Scheduler::ObjectPoolAllocator<int>>(exampleFactory);
	someAllocator->setMaxCacheSize(2);
	std::vector<Scheduler::ResourcePointer<int>> resources;

	for (int i = 0; i < 10; ++i)
		resources.emplace_back(std::move(someAllocator->retrieve()));

	for (auto& p : resources)
		p.reset();

	ASSERT_EQ(someAllocator->getObjectPoolSize(), 2);
}

TEST(poolObjectAllocator, setMaxSizeShrinksPool) {
	std::shared_ptr<Factory<int>> exampleFactory = std::make_shared<IntegerFactory>();
	auto someAllocator = std::make_shared<Scheduler::ObjectPoolAllocator<int>>(exampleFactory);
	std::vector<Scheduler::ResourcePointer<int>> resources;

	for (int i = 0; i < 10; ++i)
		resources.emplace_back(std::move(someAllocator->retrieve()));

	for (auto& p : resources)
		p.reset();

	ASSERT_EQ(someAllocator->getObjectPoolSize(), 10);
	someAllocator->setMaxCacheSize(2);
	ASSERT_EQ(someAllocator->getObjectPoolSize(), 2);
}

TEST(PoolAllocatorRepository, getResource) {
	std::shared_ptr<Factory<int>> exampleFactory = std::make_shared<IntegerFactory>();
	auto someAllocator = std::make_shared<Scheduler::ObjectPoolAllocator<int>>(exampleFactory);
	Scheduler::ObjectPoolAllocatorRepository<int> repository;
	
	repository.registerAllocator(0, someAllocator);
	ASSERT_NO_THROW(auto res = repository.retrieveResource(0)) ;
}

TEST(PoolAllocatorRepository, getReport) {
	std::shared_ptr<Factory<int>> exampleFactory = std::make_shared<IntegerFactory>();
	auto someAllocator = std::make_shared<Scheduler::ObjectPoolAllocator<int>>(exampleFactory);
	Scheduler::ObjectPoolAllocatorRepository<int> repository;
	std::vector<Scheduler::ResourcePointer<int>> resources;
	for (int i = 0; i < 30; ++i)
		resources.emplace_back(std::move(someAllocator->retrieve()));
	resources.clear();
	for (int i = 0; i < 20; ++i)
		resources.emplace_back(std::move(someAllocator->retrieve()));

	repository.registerAllocator(0, someAllocator);
	auto report = repository.getResourceReport(0);
	ASSERT_EQ(report.inUse, 20);
	ASSERT_EQ(report.cached, 10);
}

TEST(PoolAllocatorRepository, accessBeforeRegistration) {
	Scheduler::ObjectPoolAllocatorRepository<int> repository;
	ASSERT_THROW(repository.retrieveResource(0), Scheduler::AllocatorNotRegisteredException);
}

TEST(PoolAllocatorRepository, gettingReportBeforeAllocatorRegistration) {
	Scheduler::ObjectPoolAllocatorRepository<int> repository;
	ASSERT_THROW(repository.getResourceReport(0), Scheduler::AllocatorNotRegisteredException);
}

TEST(PoolAllocatorRepository, exists) {
	Scheduler::ObjectPoolAllocatorRepository<int> repository;
	std::shared_ptr<Factory<int>> exampleFactory = std::make_shared<IntegerFactory>();
	auto someAllocator = std::make_shared<Scheduler::ObjectPoolAllocator<int>>(exampleFactory);
	repository.registerAllocator(0, someAllocator);
	ASSERT_TRUE(repository.exists(0));
}

TEST(PoolAllocatorRepository, not_exists) {
	Scheduler::ObjectPoolAllocatorRepository<int> repository;
	ASSERT_FALSE(repository.exists(0));
}

TEST(PoolAllocatorRepository, remove) {
	Scheduler::ObjectPoolAllocatorRepository<int> repository;
	std::shared_ptr<Factory<int>> exampleFactory = std::make_shared<IntegerFactory>();
	auto someAllocator = std::make_shared<Scheduler::ObjectPoolAllocator<int>>(exampleFactory);
	repository.registerAllocator(0, someAllocator);
	repository.erase(0);
	ASSERT_FALSE(repository.exists(0));
}


class SchedulerWithAnalytics : public Scheduler::GameScheduler {
public:
	SchedulerWithAnalytics() : GameScheduler() {};
	size_t countEntries() const {
		std::lock_guard<std::mutex> guard(lock);
		return entries.size();
	}
	size_t getCacheSize(int playerId) const {
		std::lock_guard<std::mutex> guard(lock);
		auto report = poolAllocators->getResourceReport(playerId);
		return report.cached;
	}
	size_t countPlayerInstancesInUse(int playerId) const {
		std::lock_guard<std::mutex> guard(lock);
		auto report = poolAllocators->getResourceReport(playerId);
		return report.inUse;
	}
};

class NoopPlayer : public Player {
	Result makeResult(const Game& game, const std::chrono::milliseconds& timeOut) override	{
		return SpellChessMove();
	}
	Result handlePonderHit() override{
		return SpellChessMove();
	}
	void startPondering(const Game& game, const SpellChessMove& ponderMove) override{}
	void cancelPondering() override {}
	bool supportsPondering() const override{
		return false;
	}
	void onNewGame() override {}
	void interrupt() override {}
	void kill() override {}
};

class NoopPlayerFactory : public Factory<Player> {
	// Odziedziczono za po�rednictwem elementu Factory
	std::unique_ptr<Player> makeObject() override{
		return std::make_unique<NoopPlayer>();
	}
};

PlayerRegistry_t* setUpPlayerRegistry() {
	auto result = new PlayerRegistry_t();
	PlayerEntry someEntry1, someEntry2;
	std::shared_ptr<Factory<Player>> noopFactory = std::static_pointer_cast<Factory<Player>>(std::make_shared<NoopPlayerFactory>());

	someEntry1.name = "Test Player 1";
	someEntry1.simultaneousGamesPolicy = PlayerEntry::SIMULTANEOUS_GAMES_SUPPORTED;
	someEntry1.instanceFactory = noopFactory;

	someEntry2.name = "Test Player 2";
	someEntry2.simultaneousGamesPolicy = PlayerEntry::SIMULTANEOUS_GAMES_SUPPORTED;
	someEntry2.instanceFactory = noopFactory;

	result->registerEntry(someEntry1);
	result->registerEntry(someEntry2);
	return result;
}
TEST(GameScheduler, addEntry) {
	PlayerRegistry.reset(setUpPlayerRegistry());
	SchedulerWithAnalytics scheduler;
	scheduler.emplaceEntry(0, 1, nullptr);
	ASSERT_EQ(scheduler.countEntries(), 1);
}

TEST(GameScheduler, noNextGame) {
	PlayerRegistry.reset(setUpPlayerRegistry());
	SchedulerWithAnalytics scheduler;
	auto nextGame = scheduler.getNextGame();
	ASSERT_EQ(nextGame, nullptr);
}

TEST(GameScheduler, nextGame) {
	PlayerRegistry.reset(setUpPlayerRegistry());

	SchedulerWithAnalytics scheduler;
	scheduler.emplaceEntry(0, 1, nullptr);
	auto nextGame = scheduler.getNextGame();
	ASSERT_NE(nextGame, nullptr);
	ASSERT_EQ(nextGame->whitePlayerId, 0);
	ASSERT_EQ(nextGame->blackPlayerId, 1);
	ASSERT_EQ(nextGame->context, nullptr);
}

TEST(GameScheduler, context) {
	PlayerRegistry.reset(setUpPlayerRegistry());
	SchedulerWithAnalytics scheduler;
	int context = 2;
	scheduler.emplaceEntry(0, 1, &context);
	auto nextGame = scheduler.getNextGame();
	ASSERT_NE(nextGame, nullptr);
	ASSERT_EQ(nextGame->whitePlayerId, 0);
	ASSERT_EQ(nextGame->blackPlayerId, 1);
	ASSERT_EQ(nextGame->context, &context);
}

TEST(GameScheduler, playerUsage) {
	PlayerRegistry.reset(setUpPlayerRegistry());

	SchedulerWithAnalytics scheduler;
	scheduler.emplaceEntry(0, 1, nullptr);
	auto nextGame = scheduler.getNextGame();
	auto player = scheduler.getPlayerInstance(0);
	ASSERT_NE(player, nullptr);
	ASSERT_EQ(scheduler.countPlayerInstancesInUse(0), 1);
	ASSERT_EQ(scheduler.getCacheSize(0), 0);
}

TEST(GameScheduler, playerReuse) {
	PlayerRegistry.reset(setUpPlayerRegistry());

	SchedulerWithAnalytics scheduler;
	scheduler.emplaceEntry(0, 1, nullptr);
	scheduler.emplaceEntry(0, 1, nullptr);

	auto nextGame = scheduler.getNextGame();
	auto player = scheduler.getPlayerInstance(0);
	auto player_pointer = player.get();
	ASSERT_NE(player, nullptr);
	player.reset();
	player = scheduler.getPlayerInstance(0);
	ASSERT_EQ(player.get(), player_pointer);
}

TEST(GameScheduler, obsoletePlayerIsDeleted) {
	PlayerRegistry.reset(setUpPlayerRegistry());

	SchedulerWithAnalytics scheduler;
	scheduler.emplaceEntry(0, 1, nullptr);
	auto nextGame = scheduler.getNextGame();
	auto player = scheduler.getPlayerInstance(0);
	ASSERT_NE(player, nullptr);
	player.reset();
	ASSERT_EQ(scheduler.countPlayerInstancesInUse(0), 0);
	ASSERT_EQ(scheduler.getCacheSize(0), 0);
}

// testcase name uses snake because camel case was unreadable: schedulerWontRecommendNewGameWhenPlayerPreferNotToPlayMultipleGamesAtOnce
TEST(GameScheduler, scheduler_wont_recommend_new_game_when_player_prefer_not_to_play_multiple_games_at_once) {
	PlayerRegistry.reset(setUpPlayerRegistry());

	// Step 1. Add new player registry entry for the player with custom policy
	PlayerEntry customPlayer;
	customPlayer.name = "single game preference";
	customPlayer.simultaneousGamesPolicy = PlayerEntry::SIMULTANEOUS_GAMES_NOT_PREFFERED;
	customPlayer.instanceFactory = std::make_shared<NoopPlayerFactory>();
	PlayerRegistry->registerEntry(customPlayer);
	
	// Step2.  inform scheduler that new player with play 2 games
	SchedulerWithAnalytics scheduler;
	scheduler.emplaceEntry(0, customPlayer.id, nullptr);
	scheduler.emplaceEntry(0, customPlayer.id, nullptr);
	
	// Step3. Lock the player instance
	auto nextGame = scheduler.getNextGame();
	auto whPlayer = scheduler.getPlayerInstance(nextGame->whitePlayerId);
	auto blPlayer = scheduler.getPlayerInstance(nextGame->blackPlayerId);

	// Scheduler shouldn't recommend any new game as long custom  player is playing
	ASSERT_EQ(scheduler.getNextGame(), nullptr);
}

TEST(GameScheduler, scheduler_will_wait_for_player_if_simoultanous_games_not_prefered) {
	PlayerRegistry.reset(setUpPlayerRegistry());

	// Step 1. Add new player registry entry for the player with custom policy
	PlayerEntry customPlayer;
	customPlayer.name = "single game preference";
	customPlayer.simultaneousGamesPolicy = PlayerEntry::SIMULTANEOUS_GAMES_NOT_PREFFERED;
	customPlayer.instanceFactory = std::make_shared<NoopPlayerFactory>();
	PlayerRegistry->registerEntry(customPlayer);

	// Step2.  inform scheduler that new player with play 2 games
	SchedulerWithAnalytics scheduler;
	scheduler.emplaceEntry(0, customPlayer.id, nullptr);
	scheduler.emplaceEntry(0, customPlayer.id, nullptr);

	// Step3. Lock the player instance
	auto nextGame = scheduler.getNextGame();
	auto whPlayer = scheduler.getPlayerInstance(nextGame->whitePlayerId);
	auto blPlayer = scheduler.getPlayerInstance(nextGame->blackPlayerId);

	// Step 4. Release both players to inform scheduler that new player is not playing
	whPlayer.reset();
	blPlayer.reset();

	// Scheduler shouldn't recommend any new game as long custom  player is playing
	ASSERT_NE(scheduler.getNextGame(), nullptr);
}

TEST(GameScheduler, scheduler_will_pick_idle_option_when_available) {
	PlayerRegistry.reset(setUpPlayerRegistry());

	// Step 1. Add new player registry entry for the player with custom policy
	PlayerEntry customPlayer;
	customPlayer.name = "single game preference";
	customPlayer.simultaneousGamesPolicy = PlayerEntry::SIMULTANEOUS_GAMES_NOT_PREFFERED;
	customPlayer.instanceFactory = std::make_shared<NoopPlayerFactory>();
	PlayerRegistry->registerEntry(customPlayer);

	SchedulerWithAnalytics scheduler;
	scheduler.emplaceEntry(0, customPlayer.id, nullptr);
	auto someGame = scheduler.getNextGame();
	auto whPlayer1 = scheduler.getPlayerInstance(someGame->whitePlayerId);
	auto blPlayer1 = scheduler.getPlayerInstance(someGame->blackPlayerId);
	
	scheduler.emplaceEntry(0, customPlayer.id, nullptr);
	scheduler.emplaceEntry(0, 1, nullptr);

	auto nextGame = scheduler.getNextGame();
	auto whPlayer = scheduler.getPlayerInstance(nextGame->whitePlayerId);
	auto blPlayer = scheduler.getPlayerInstance(nextGame->blackPlayerId);

	ASSERT_EQ(nextGame->whitePlayerId, 0);
	ASSERT_EQ(nextGame->blackPlayerId, 1);
}