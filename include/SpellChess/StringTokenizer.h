#pragma once

#include <string>
#include <vector>

#include "Board.h"

namespace SpellChessLib {
	
	namespace Parsing {
		extern  std::vector<std::string> tokenizeStringBySpace(const char* str);

		class ParsingError: public std::exception {};
		constexpr size_t strlen_constexpr(const char* str);
		constexpr bool validateSquareIndexString(const char* str);
		constexpr uint8_t parseSquareIndex(const char* str);
		extern uint16_t parseUnsignedInteger(const char* str);
	}

#define DECL_SQ(sq) const SquareIndex sq = Parsing::parseSquareIndex(#sq); 
}


constexpr size_t SpellChessLib::Parsing::strlen_constexpr(const char* str)
{
    size_t result = 0;
    const char* p = str;
    while (*p) {
        ++result;
        ++p;
    }
    return result;
}

constexpr uint8_t SpellChessLib::Parsing::parseSquareIndex(const char* str)
{
    const uint8_t eight = 8;
    
    size_t len = strlen_constexpr(str);

    if (!Parsing::validateSquareIndexString(str))
        throw Parsing::ParsingError();

    if (len == 1) return NoSquare;

    return (str[0] - 'a') + eight * (eight - str[1] + '0');
}


constexpr bool SpellChessLib::Parsing::validateSquareIndexString(const char* str)
{
    size_t len = SpellChessLib::Parsing::strlen_constexpr(str);
    if (len != 1 && len!= 2) return false;
    if (len == 1) return *str == '-';
    return str[0] >= 'a' && str[0] <= 'h' && str[1] >= '1' && str[1] <= '8'; 
}