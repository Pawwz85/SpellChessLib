#pragma once

#include "Board.h"
#include <string>

namespace SpellChessLib {

	extern std::string spellChessMoveToSAN(const SpellChessMove & move, const GameState & context);
    extern std::string spellChessMoveToUnicodeString(const SpellChessMove & move, const GameState & context);

}