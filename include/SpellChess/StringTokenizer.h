#pragma once

#include <string>
#include <string_view>
#include <vector>
#include <cstdint>
#include <exception>

#include "Board.h"

namespace SpellChessLib {
	
	namespace Parsing {
		extern  std::vector<std::string> tokenizeStringBySpace(std::string_view str);
        extern  std::vector<std::string_view> splitString(std::string_view str, char delim);

		class ParsingError: public std::exception {};
		constexpr bool validateSquareIndexString(std::string_view str);
		constexpr uint8_t parseSquareIndex(std::string_view str);
		extern uint16_t parseUnsignedInteger(std::string_view str);
	}

#define DECL_SQ(sq) const SquareIndex sq = Parsing::parseSquareIndex(#sq); 
}

constexpr uint8_t SpellChessLib::Parsing::parseSquareIndex(std::string_view str)
{
    const uint8_t eight = 8;

    if (!Parsing::validateSquareIndexString(str))
        throw Parsing::ParsingError();

    if (str.size() == 1) return NoSquare;

    return (str[0] - 'a') + eight * (eight - str[1] + '0');
}


constexpr bool SpellChessLib::Parsing::validateSquareIndexString(std::string_view str)
{
    if (str.size() != 1 && str.size() != 2)
        return false;
    if (str.size() == 1)
        return str[0] == '-';
    return str[0] >= 'a' && str[0] <= 'h' && str[1] >= '1' && str[1] <= '8'; 
}