#include <gtest/gtest.h>
#include <SpellChess/StringTokenizer.h>
#include <SpellChess/MoveLegality.h>
#include <SpellChess/Board.h>
#include <cstdint>
#include "utils.h"

using namespace SpellChessLib;
using namespace SpellChessLib::_Attackers;

TEST(KNIGHT_ATTACKERS, middleBoard) {
	FenParser parser;
	GameState s = parser.fromStandardChessFen("nnnnknnn/nnnnnnnn/nnnnnnnn/nnnnnnnn/nnnnnnnn/8/8/7K w - - 0 1");
	const SquareIndex attackedSq = Parsing::parseSquareIndex("e6");
	uint64_t attackersMask = getKnightAttackers(s, attackedSq, Black);

	ASSERT_EQ(attackersMask, computeExpectedMask({ "d8", "f8", "c7", "g7", "c5", "g5", "d4", "f4" }));
}

TEST(KNIGHT_ATTACKERS, a8_corner) {
	FenParser parser;
	GameState s = parser.fromStandardChessFen("nnnnknnn/nnnnnnnn/nnnnnnnn/nnnnnnnn/nnnnnnnn/nnnnnnnn/nnnnnnnn/nnnnnnnK w - - 0 1");
	const SquareIndex attackedSq = Parsing::parseSquareIndex("a8");
	uint64_t attackersMask = getKnightAttackers(s, attackedSq, Black);
	ASSERT_EQ(attackersMask, computeExpectedMask({ "b6", "c7" }));
}

TEST(KNIGHT_ATTACKERS, h1_corner) {
	FenParser parser;
	GameState s = parser.fromStandardChessFen("nnnnknnn/nnnnnnnn/nnnnnnnn/nnnnnnnn/nnnnnnnn/nnnnnnnn/nnnnnnnn/nnnnnnnK w - - 0 1");
	const SquareIndex attackedSq = Parsing::parseSquareIndex("h1");
	uint64_t attackersMask = getKnightAttackers(s, attackedSq, Black);
	ASSERT_EQ(attackersMask, computeExpectedMask({ "g3", "f2" }));
}

TEST(KNIGHT_ATTACKERS, onlyEnemyKnightCounts) {
	FenParser parser;
	GameState s = parser.fromStandardChessFen("8/8/2k1n3/1N3q2/8/1b3r2/2p5/7K w - - 0 1");
	const SquareIndex attackedSq = Parsing::parseSquareIndex("d4");
	uint64_t attackersMask = getKnightAttackers(s, attackedSq, Black);
	ASSERT_EQ(attackersMask, computeExpectedMask({ "e6"}));
}

TEST(KNIGHT_ATTACKERS, onlyEnemyKnightCounts_white) {
	FenParser parser;
	GameState s = parser.fromStandardChessFen("8/8/2k1n3/1N3q2/8/1b3r2/2p5/7K w - - 0 1");
	const SquareIndex attackedSq = Parsing::parseSquareIndex("d4");
	uint64_t attackersMask = getKnightAttackers(s, attackedSq, White);
	ASSERT_EQ(attackersMask, computeExpectedMask({ "b5" }));
}

TEST(KNIGHT_ATTACKERS, nothing) {
	FenParser parser;
	GameState s = parser.fromStandardChessFen("7k/8/8/8/8/8/8/7K w - - 0 1");
	const SquareIndex attackedSq = Parsing::parseSquareIndex("e6");
	uint64_t attackersMask = getKnightAttackers(s, attackedSq, Black);

	ASSERT_EQ(attackersMask, 0);
}

TEST(KING_ATTACKERS, middle_of_the_board_bl) {
	FenParser parser;
	GameState s = parser.fromStandardChessFen("8/8/4k3/8/8/8/8/7K w - - 0 1");
	const SquareIndex attackedSq = Parsing::parseSquareIndex("e5");
	uint64_t attackersMask = getKingAttackers(s, attackedSq, Black);

	ASSERT_EQ(attackersMask, computeExpectedMask({"e6"}));
}

TEST(KING_ATTACKERS, middle_of_the_board_wh) {
	FenParser parser;
	GameState s = parser.fromStandardChessFen("8/8/4K3/8/8/8/8/7k w - - 0 1");
	const SquareIndex attackedSq = Parsing::parseSquareIndex("e5");
	uint64_t attackersMask = getKingAttackers(s, attackedSq, White);

	ASSERT_EQ(attackersMask, computeExpectedMask({ "e6" }));
}

TEST(PAWN_ATTACKERS, middle_of_the_board_black) {
	FenParser parser;
	GameState s = parser.fromStandardChessFen("7k/8/8/2ppp3/2p1p3/2ppp3/8/7K w - - 0 1");
	const SquareIndex attackedSq = Parsing::parseSquareIndex("d4");

	uint64_t attackersMask = getPawnAttackers(s, attackedSq, Black);
	ASSERT_EQ(attackersMask, computeExpectedMask({ "c5", "e5" }));
}

TEST(PAWN_ATTACKERS, middle_of_the_board_white) {
	FenParser parser;
	GameState s = parser.fromStandardChessFen("7k/8/8/2PPP3/2P1P3/2PPP3/8/7K w - - 0 1");
	const SquareIndex attackedSq = Parsing::parseSquareIndex("d4");

	uint64_t attackersMask = getPawnAttackers(s, attackedSq, White);
	ASSERT_EQ(attackersMask, computeExpectedMask({ "c3", "e3" }));
}

TEST(SLIDER_ATTACKERS, all_attackers) {
	FenParser parser;
	GameState s = parser.fromStandardChessFen("b2qkbbr/8/8/r4r2/2b5/5q2/8/3qk3 w - - 0 1");
	const SquareIndex attackedSq = Parsing::parseSquareIndex("d5");

	uint64_t attackersMask = getSliderAttackers(s, attackedSq, Black);
	ASSERT_EQ(attackersMask, computeExpectedMask({ "a8", "d8", "g8", "a5", "f5", "c4", "f3", "d1"}));
}

TEST(SLIDER_ATTACKERS, wrong_attackers) {
	FenParser parser;
	GameState s = parser.fromStandardChessFen("r2bkbrr/8/8/b4b2/2r5/5r2/8/3bk3 w - - 0 1");
	const SquareIndex attackedSq = Parsing::parseSquareIndex("d5");

	uint64_t attackersMask = getSliderAttackers(s, attackedSq, Black);
	ASSERT_EQ(attackersMask, 0);
}

TEST(SLIDER_ATTACKERS, slidersAreBlocked) {
	FenParser parser;
	GameState s = parser.fromStandardChessFen("b2qkbbr/1p1p1p2/8/rP2Pr2/2PPP3/1b3q2/8/3qk3 w - - 0 1");
	const SquareIndex attackedSq = Parsing::parseSquareIndex("d5");
	uint64_t attackersMask = getSliderAttackers(s, attackedSq, Black);
	ASSERT_EQ(attackersMask, 0);
}

TEST(SLIDER_ATTACKERS, non_attacking_king_should_not_block_sliders) {
	FenParser parser;
	GameState s = parser.fromStandardChessFen("1k5R/p1pppppp/8/8/8/8/8/7K b - - 0 1");
	const SquareIndex attackedSq = Parsing::parseSquareIndex("a8");
	uint64_t attackersMask = getSliderAttackers(s, attackedSq, White);
	ASSERT_EQ(attackersMask, computeExpectedMask({ "h8"}));
}

TEST(SLIDER_ATTACKERS, attacking_king_should_block_sliders) {
	FenParser parser;
	GameState s = parser.fromStandardChessFen("2k4r/p1pppppp/8/8/8/8/8/7K b - - 0 1");
	const SquareIndex attackedSq = Parsing::parseSquareIndex("a8");
	uint64_t attackersMask = getSliderAttackers(s, attackedSq, Black);
	ASSERT_EQ(attackersMask, 0);
}