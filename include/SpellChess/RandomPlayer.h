#pragma once

#include "Player.h"
#include "Factory.h"
#include <random>
#include <chrono>
namespace SpellChessLib {

	class RandomPlayer : public Player {
		std::random_device dev;
		// Odziedziczono za po�rednictwem elementu Player
		Result makeResult(const Game& game, const std::chrono::milliseconds& timeOut) override;
		Result handlePonderHit() override;
		void startPondering(const Game& game, const SpellChessMove& ponderMove) override;
		void cancelPondering() override;
		bool supportsPondering() const override;
		void interrupt() override;
		void kill() override;
		void onNewGame() override;
	
	public:
		class FactoryClass : public Factory<Player> {
			std::unique_ptr<SpellChessLib::Player> makeObject() override;
		};




	};

	

}