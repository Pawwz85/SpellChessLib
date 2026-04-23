#include <SpellChess/Scheduler/GameScheduler.h>

size_t SpellChessLib::Scheduler::GameScheduler::countPlayerOccurrencesInEntries(int playerId) const{
	size_t result = 0;
	for (auto& e : entries) {
		if (e->whitePlayerId == playerId)
			++result;
		if (e->blackPlayerId == playerId)
			++result;
	}
	return result;
}

bool SpellChessLib::Scheduler::GameScheduler::canBeInstantiatedNow(int playerId){
	bool result;
	try {
		auto playerEntry = PlayerRegistry->get(playerId);
		result = playerEntry.simultaneousGamesPolicy == PlayerEntry::SIMULTANEOUS_GAMES_SUPPORTED ||
			poolAllocators->getResourceReport(playerId).inUse == 0;
	}
	catch (AllocatorNotRegisteredException e) {
		return false;
	}
	catch (PlayerRegistry_t::EntryDoesNotExistError e) {
		return false;
	}
	
	return result;
}

size_t SpellChessLib::Scheduler::GameScheduler::calcGamePriority(const GameSchedulerEntry& entry){
	auto whRaport = poolAllocators->getResourceReport(entry.whitePlayerId);
	auto blRaport = poolAllocators->getResourceReport(entry.blackPlayerId);
	return whRaport.cached + blRaport.cached;
	
}

void SpellChessLib::Scheduler::GameScheduler::syncPoolAllocatorWithThePlayerRegistry(){
	auto it = PlayerRegistry->cbegin();
	while (it != PlayerRegistry->cend()) {
		if(!poolAllocators->exists(it->id))
			poolAllocators->registerAllocator(it->id, std::make_shared<ObjectPoolAllocator<Player>>(it->instanceFactory));
		++it;
	}
	
}

std::unique_ptr<SpellChessLib::Scheduler::GameSchedulerEntry> SpellChessLib::Scheduler::GameScheduler::getNextGame(){
	std::lock_guard<std::mutex> guard(lock);
	if (entries.empty())
		return nullptr;

	syncPoolAllocatorWithThePlayerRegistry();

	auto best = entries.end();
	auto it = entries.begin();
	int topScore = INT_MIN;
	int tmp = 0;

	while (it != entries.end()) {
		tmp = (int)calcGamePriority(**it);
		if (tmp > topScore && canBeInstantiatedNow((*it)->whitePlayerId) && canBeInstantiatedNow((*it)->blackPlayerId)) {
			topScore = tmp;
			best = it;
		}
		++it;
	}

	if (best == entries.end())
		return nullptr;

	std::unique_ptr<GameSchedulerEntry> result = std::move(*best);
	entries.erase(best);

	poolAllocators->setPoolLimit(result->whitePlayerId, std::min(size_t(2), countPlayerOccurrencesInEntries(result->whitePlayerId)));
	poolAllocators->setPoolLimit(result->blackPlayerId, std::min(size_t(2), countPlayerOccurrencesInEntries(result->blackPlayerId)));
	return result;
}

void SpellChessLib::Scheduler::GameScheduler::emplaceEntry(int whitePlayerId, int blackPlayerId, void* context){
	std::lock_guard<std::mutex> guard(lock);
	GameSchedulerEntry* entry = new GameSchedulerEntry({ whitePlayerId, blackPlayerId, context });
	entries.emplace_back(entry);
}

SpellChessLib::Scheduler::ResourcePointer<SpellChessLib::Player> SpellChessLib::Scheduler::GameScheduler::getPlayerInstance(int playerId){
	std::lock_guard<std::mutex> guard(lock);
	syncPoolAllocatorWithThePlayerRegistry();
	return poolAllocators->retrieveResource(playerId);
}
