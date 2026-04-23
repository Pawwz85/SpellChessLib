#pragma once

#include <UCILoader/ProcessWrapper.h>
#include <UCILoader/EngineConnection.h>

#include "Player.h"
#include "Factory.h"

namespace SpellChessLib {
	
	class UCIEnginePlayer : public Player {
		std::shared_ptr<UCILoader::EngineInstance<SpellChessMove>> instance;
		std::shared_ptr<UCILoader::SearchConnection<SpellChessMove>> searchConnection;

		Result waitForResult();
	public: 
		UCIEnginePlayer(std::shared_ptr<UCILoader::EngineInstance<SpellChessMove>> instance) : instance(instance), searchConnection(nullptr) {
			instance->sync();
		};
	
		// Odziedziczono za po�rednictwem elementu Player
		Result makeResult(const Game& game, const std::chrono::milliseconds& timeOut) override;
		Result handlePonderHit() override;
		void startPondering(const Game& game, const SpellChessMove& ponderMove) override;
		void cancelPondering() override;
		bool supportsPondering() const override;
		void interrupt() override;
		void kill() override;
		void onNewGame() override;
		bool isAlive() override;
	};

	class UCIPlayerFactory : public Factory<Player> {
		std::string pathToExecutable;
	public:
		UCIPlayerFactory(const std::string& path) : pathToExecutable(path) {};
		// Odziedziczono za po�rednictwem elementu Factory
		std::unique_ptr<Player> makeObject() override;
	};

}