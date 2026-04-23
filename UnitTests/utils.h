#pragma once

#include "pch.h"
#include <SpellChess/StringTokenizer.h>

static uint64_t computeExpectedMask(const std::initializer_list<const char*>& in) {
	uint64_t result = 0ull;
	for (auto x : in)
		result |= (1ull << SpellChessLib::Parsing::parseSquareIndex(x));
	return result;
}

static std::vector<SpellChessLib::SpellChessMove> parseMoveList(const std::initializer_list<const char*>& list) {
	std::vector<SpellChessLib::SpellChessMove> result;
	for (auto& c : list)
		result.push_back(SpellChessLib::parseMove(c));
	return result;
}