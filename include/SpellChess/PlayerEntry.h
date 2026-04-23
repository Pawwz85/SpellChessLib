#pragma once

#include <cstdint>
#include <string>
#include <memory>
#include "Factory.h"
#include "Player.h"

namespace SpellChessLib {

	struct PlayerEntry {
		
		enum SimultaneousGamesPolicy : uint32_t {
			SIMULTANEOUS_GAMES_SUPPORTED,
			SIMULTANEOUS_GAMES_NOT_PREFFERED
		};
		
		int32_t id;			// unique id assigned to the current player, used by GameRunnerPool to ensure simultaneous game policies of the players involved
		SimultaneousGamesPolicy simultaneousGamesPolicy;
		std::string name;
		std::shared_ptr<Factory<Player>> instanceFactory;
	};

}