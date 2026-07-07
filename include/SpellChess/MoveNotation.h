#pragma once

#include "Board.h"

namespace SpellChessLib {

	extern std::string spellChessMoveToSAN(const SpellChessMove & move, const GameState & context);
    extern std::string spellChessMoveToUnicodeString(const SpellChessMove & move, const GameState & context);

}