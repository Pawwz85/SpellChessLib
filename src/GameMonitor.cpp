#include <SpellChess/GameMonitor.h>
#include <mutex>

void SpellChessLib::GameMonitor::onGameValueChanged(const Game& game){
	observableState->set(game.getGameState());
	observableMoves->set(game.getMovesSinceRoot());
}

int GameRunnerSessionIdService() {
	static std::atomic<int> id = 1;
	return id.fetch_add(1);
}

void SpellChessLib::GameMonitor::onGameRunnerConnect(){
	gameRunnerIdObserver->set(GameRunnerSessionIdService());
}

void SpellChessLib::GameMonitor::onGameRunnerDisconnect(){
	gameRunnerIdObserver->set(0);
}
