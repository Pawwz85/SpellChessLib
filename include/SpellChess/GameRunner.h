#pragma once

#include "SpellChessGame.h"

#include "Player.h"
#include "ControlFlowManager.h"
#include "GameRunnerConfiguration.h"
#include "GameMonitor.h"
#include "Scheduler/ObjectAllocator.h"
#include <mutex>
#include <memory>
#include <atomic>

namespace SpellChessLib {

	class GameRunner {
		mutable std::mutex lock;
		std::shared_ptr<ControlFlowManager> flowManager;
		Scheduler::ResourcePointer<Player> whPlayer;
		Scheduler::ResourcePointer<Player> blPlayer;
		std::unique_ptr<Game> game;
		std::shared_ptr<GameMonitor> gameMonitor;
		std::atomic<bool> running = false;
		bool allowPondering;

		class _PlayerWrapper;
		_PlayerWrapper *blockingTask = nullptr;

		Player* currentPlayer = nullptr;
		void setCurrentPlayer(Player* p);

		class _StopCallback : public Game::ResultObserver {
			GameRunner* parent;
			public:
				_StopCallback(GameRunner* parent) : parent(parent) {};
				// Odziedziczono za po�rednictwem elementu ResultObserver
				void onGameOutcome(const PlayerColor& winner, const GameStatusCode& outcome) override;
		};

		class _PlayerWrapper : public ControlFlowManager::BlockingTask {
			Player* p;
		public:
			_PlayerWrapper(Player* player) : p(player) {};
			// Odziedziczono za po�rednictwem elementu BlockingTask
			void interrupt() override;
			void terminate() override;
		};

		static Game* gameFromConfig(const GameRunnerConfiguration& config, std::shared_ptr<TimerService> timeServ);

	public:

		GameRunner(std::shared_ptr<TimerService> timeServ, std::unique_ptr<Game> game, Scheduler::ResourcePointer<Player>&& whPlayer, Scheduler::ResourcePointer<Player>&& blPlayer, bool allowPondering = false) :
			whPlayer(std::move(whPlayer)), blPlayer(std::move(blPlayer)), game(game.release()), allowPondering(allowPondering), flowManager(std::make_shared<ControlFlowManager>(timeServ)) {};
		
		GameRunner(std::shared_ptr<TimerService> timeServ, const GameRunnerConfiguration& config, Scheduler::ResourcePointer<Player>&& whPlayer, Scheduler::ResourcePointer<Player>&& blPlayer) :
			whPlayer(std::move(whPlayer)), blPlayer(std::move(blPlayer)), allowPondering(config.allowPondering), flowManager(std::make_shared<ControlFlowManager>(timeServ)),
			game(gameFromConfig(config, timeServ))
		{};

		~GameRunner();

		void run();
		void stop(); 
		void attachGameMonitor(std::shared_ptr<GameMonitor> monitor);
		bool isRunning() const;
		const Game& getGame() const { return *game; };
		Game& getGame() { return *game; };

	};

}