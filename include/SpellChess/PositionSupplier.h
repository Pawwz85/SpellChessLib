#pragma once 

#include "Board.h"
#include <string>

namespace SpellChessLib {
    class PositionSupplier {
	public:
		virtual GameState produce() = 0;
	};

	class RandomPositionSupplier : public PositionSupplier {
		int ply;
		GameState startPos;
		GameState recursiveRandomization(GameState in ,int level);
	public:
		RandomPositionSupplier(const std::string startPosFen, const int & ply): ply(ply) {
			FenParser parser;
			startPos = parser.fromSpellChessFen(startPosFen);
		}
		GameState produce() override { return recursiveRandomization(startPos, ply); };
	};

};

	