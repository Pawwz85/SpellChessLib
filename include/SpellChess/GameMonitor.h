#pragma once

#include "Board.h"
#include "Observer.h"
#include "SpellChessGame.h"

#include <memory>
#include <vector>

namespace SpellChessLib {

	class GameMonitor {
		std::shared_ptr<DefaultNullableObservable<GameState>> observableState;
		std::shared_ptr<ObservableState<std::vector<SpellChessMove>>> observableMoves;
		std::shared_ptr<ObservableState<int>> gameRunnerIdObserver; 
		std::shared_ptr<ObservableState<TimerPairState>> timerPairObserver;

	public:

		GameMonitor(): observableState(std::make_shared<DefaultNullableObservable<GameState>>()),
			observableMoves(std::make_shared<ObservableState<std::vector<SpellChessMove>>>(std::vector<SpellChessMove>())),
			gameRunnerIdObserver(std::make_shared<ObservableState<int>>(0)),
			timerPairObserver(new ObservableState<TimerPairState>({}))
		{

		}

		Broadcaster<GameState*>& getObservableGame() { return *observableState; };
		Broadcaster<std::vector<SpellChessMove>>& getObservableMoves() { return *observableMoves; };
		Broadcaster<int>& getGameRunnerIdObserver() { return *gameRunnerIdObserver; };
		Broadcaster<TimerPairState> & getTimerPairObserver() {return *timerPairObserver;}

		void onGameValueChanged(const Game& game);
		void onGameRunnerConnect();
		void onGameRunnerDisconnect();
	};

}