#pragma once

#include <queue>
#include <functional>
#include <memory>
#include <mutex>
#include "ObjectAllocatorRepository.h"
#include "../PlayerRegistry.h"

namespace SpellChessLib {

	namespace Scheduler {

		struct GameSchedulerEntry {
			int whitePlayerId;
			int blackPlayerId;
			void* context; // place to store additional context for game runner.
		};

		class GameScheduler {
			size_t countPlayerOccurrencesInEntries(int playerId) const;
			bool canBeInstantiatedNow(int playerId);
		protected:
			mutable std::mutex lock;
			std::shared_ptr<ObjectPoolAllocatorRepository<Player>> poolAllocators;
			std::list<std::unique_ptr<GameSchedulerEntry>> entries;
			size_t calcGamePriority(const GameSchedulerEntry & entry);
			void syncPoolAllocatorWithThePlayerRegistry();
		public:
			GameScheduler() : poolAllocators(std::make_shared<ObjectPoolAllocatorRepository<Player>>()) {};
			std::unique_ptr<GameSchedulerEntry> getNextGame(); // return nullptr if no game is present. 
			void emplaceEntry(int whitePlayerId, int blackPlayerId, void* context);
			Scheduler::ResourcePointer<Player> getPlayerInstance(int playerId);
		
		};

	};

}