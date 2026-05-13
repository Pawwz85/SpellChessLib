#include "pch.h"

#include <SpellChess/MoveGen.h>
#include "utils.h"

using namespace SpellChessLib;

#define NO_PINS(generator) for (size_t i = 0; i < 64; ++i) ASSERT_EQ(generator.getPinningPiece(i), NoSquare)

TEST(move_gen_enemy_control, base_case) {
	FenParser parser;
	GameState s = parser.fromStandardChessFen("rnbqk2r/p1p2ppp/4pn2/6B1/QbpPP3/2N2N2/1P3PPP/R3KB1R b KQkq - 0 8");
	MoveGenerator gen(s);

	ASSERT_TRUE(gen.isInCheck());
	NO_PINS(gen);
	ASSERT_EQ(gen.getEnemyControlMask(),
		computeExpectedMask({ "a7", "a6", "a5", "e8", "d7", "c6", "b5", "b4", "b3", "c2", "d1", "a3", "a2", "a1" }) // queen on a4
		| computeExpectedMask({ "f6", "h6", "h4", "f4", "e3", "d2", "c1" }) // bishop on g5
		| computeExpectedMask({ "c4", "d3", "e2", "g2" }) // bishop on f1
		| computeExpectedMask({ "a2", "a4", "b5", "d5", "e4", "e2", "d1", "b1" }) // knight on c3
		| computeExpectedMask({ "e1", "d2", "d4", "e5", "g5", "h4", "h2", "g1" }) // knight on f3
		| computeExpectedMask({ "a2", "a3", "a4", "b1", "c1", "d1", "e1" }) // rook on a1
		| computeExpectedMask({ "g1", "f1", "h2" }) // rook on h1
		| computeExpectedMask({ "d1", "d2", "e2", "f2", "f1" }) // king on e1
		| computeExpectedMask({ "a3", "c3", "c5", "d5", "e5", "f5", "e3", "f3", "g3", "h3" }) // pawns
	);
}

TEST(move_gen_enemy_control, pins) {
	FenParser parser;
	GameState s = parser.fromStandardChessFen("1b2q2q/8/3RBR2/q2BKB1r/3RBR2/8/1b5b/4r1k1 w - - 0 1");
	MoveGenerator gen(s);
	ASSERT_FALSE(gen.isInCheck());

#define PINNER_OF(sq) gen.getPinningPiece(Parsing::parseSquareIndex(#sq))
	EXPECT_EQ(PINNER_OF(d6), Parsing::parseSquareIndex("b8"));
	EXPECT_EQ(PINNER_OF(e6), Parsing::parseSquareIndex("e8"));
	EXPECT_EQ(PINNER_OF(f6), Parsing::parseSquareIndex("h8"));
	EXPECT_EQ(PINNER_OF(d5), Parsing::parseSquareIndex("a5"));
	EXPECT_EQ(PINNER_OF(f5), Parsing::parseSquareIndex("h5"));
	EXPECT_EQ(PINNER_OF(d4), Parsing::parseSquareIndex("b2"));
	EXPECT_EQ(PINNER_OF(e4), Parsing::parseSquareIndex("e1"));
	EXPECT_EQ(PINNER_OF(f4), Parsing::parseSquareIndex("h2"));
#undef PINNER_OF
}

TEST(move_gen_enemy_control, ray_propagate_through_king) {
	FenParser parser;
	GameState s = parser.fromStandardChessFen("4k2R/8/8/8/8/8/8/7K b - - 0 1");
	MoveGenerator gen(s);

	ASSERT_TRUE(gen.isInCheck());
	NO_PINS(gen);
	ASSERT_EQ(gen.getEnemyControlMask(),
		computeExpectedMask({ "g1", "g2", "h2" }) // king on h1 attacks
		| computeExpectedMask({ "a8", "b8", "c8", "d8", "e8", "f8", "g8", "h7", "h6", "h5", "h4", "h3", "h2", "h1" })
	);
}

TEST(pin_constraint, valid_move_by_piece_in_pin1 ) {
	ASSERT_FALSE(_Constraints::isMoveViolatingPin(
		Parsing::parseSquareIndex("e4"), 
		Parsing::parseSquareIndex("e1"),
		Parsing::parseSquareIndex("e7")
		));
}

TEST(pin_constraint, valid_move_by_piece_in_pin2) {
	ASSERT_FALSE(_Constraints::isMoveViolatingPin(
		Parsing::parseSquareIndex("e7"),
		Parsing::parseSquareIndex("e1"),
		Parsing::parseSquareIndex("e7")
	));
}

TEST(pin_constraint, valid_move_by_piece_in_pin3) {
	ASSERT_FALSE(_Constraints::isMoveViolatingPin(
		Parsing::parseSquareIndex("f2"),
		Parsing::parseSquareIndex("e1"),
		Parsing::parseSquareIndex("h4")
	));
}

TEST(pin_constraint, valid_move_by_piece_in_pin4) {
	ASSERT_FALSE(_Constraints::isMoveViolatingPin(
		Parsing::parseSquareIndex("c3"),
		Parsing::parseSquareIndex("e1"),
		Parsing::parseSquareIndex("a5")
	));
}

TEST(pin_constraint, pinViolation1) {
	ASSERT_TRUE(_Constraints::isMoveViolatingPin(
		Parsing::parseSquareIndex("d2"),
		Parsing::parseSquareIndex("e1"),
		Parsing::parseSquareIndex("e7")
	));
}

TEST(pin_constraint, pinViolation2) {
	ASSERT_TRUE(_Constraints::isMoveViolatingPin(
		Parsing::parseSquareIndex("e2"),
		Parsing::parseSquareIndex("e1"),
		Parsing::parseSquareIndex("h4")
	));
}

TEST(pin_constraint, pinViolation3) {
	ASSERT_TRUE(_Constraints::isMoveViolatingPin(
		Parsing::parseSquareIndex("e3"),
		Parsing::parseSquareIndex("e1"),
		Parsing::parseSquareIndex("a5")
	));
}

TEST(blocking_attacker, correctlyBlockAttacker1) {
	SquareIndex toSq = Parsing::parseSquareIndex("e2");
	SquareIndex kingPos = Parsing::parseSquareIndex("e1");
	uint64_t attacker = 1ull<<Parsing::parseSquareIndex("e7");

	uint64_t blockedAttacker = _Constraints::getBlockedAttacker(
		toSq,
		kingPos,
		attacker
	);
	ASSERT_EQ(blockedAttacker, attacker);
}

TEST(blocking_attacker, correctlyBlockAttacker2) {

	SquareIndex toSq = Parsing::parseSquareIndex("c6");
	SquareIndex kingPos = Parsing::parseSquareIndex("e8");
	uint64_t attacker = 1ull << Parsing::parseSquareIndex("b5");

	uint64_t blockedAttacker = _Constraints::getBlockedAttacker(
		toSq,
		kingPos,
		attacker
	);
	ASSERT_EQ(blockedAttacker, attacker);
}
// 
TEST(blocking_attacker, notBlockedAttacker1) {

	SquareIndex toSq = Parsing::parseSquareIndex("f3");
	SquareIndex kingPos = Parsing::parseSquareIndex("e1");
	uint64_t attacker = 1ull << Parsing::parseSquareIndex("e7");

	uint64_t blockedAttacker = _Constraints::getBlockedAttacker(
		toSq,
		kingPos,
		attacker
	);
	ASSERT_EQ(blockedAttacker, 0);
}

TEST(blocking_attacker, notBlockedAttacker2) {

	SquareIndex toSq = Parsing::parseSquareIndex("f3");
	SquareIndex kingPos = Parsing::parseSquareIndex("e8");
	uint64_t attacker = 1ull << Parsing::parseSquareIndex("b5");

	uint64_t blockedAttacker = _Constraints::getBlockedAttacker(
		toSq,
		kingPos,
		attacker
	);
	ASSERT_EQ(blockedAttacker, 0);
}

TEST(frozen_attackers, noFreeze) {
	SquareIndex freezeSq = NoSquare;
	uint64_t attackers = computeExpectedMask({ "d3", "e3", "f3" });
	uint64_t frozenAttacker = _Constraints::getFrozenAttackers(freezeSq, attackers);
	ASSERT_EQ(frozenAttacker, 0);
}

TEST(frozen_attackers, case1) {
	SquareIndex freezeSq = Parsing::parseSquareIndex("e3");
	uint64_t attackers = computeExpectedMask({ "d3", "e3", "f3" });
	uint64_t frozenAttacker = _Constraints::getFrozenAttackers(freezeSq, attackers);
	ASSERT_EQ(frozenAttacker, attackers);
}

TEST(frozen_attackers, case2) {
	SquareIndex freezeSq = Parsing::parseSquareIndex("f4");
	uint64_t attackers = computeExpectedMask({ "d3", "e3", "f3" });
	uint64_t frozenAttacker = _Constraints::getFrozenAttackers(freezeSq, attackers);
	ASSERT_EQ(frozenAttacker, computeExpectedMask({"e3", "f3"}));
}


#define CHESS_MOVE_GEN_SETUP(fen, parserCall) FenParser parser; \
	GameState s = parserCall(fen); \
	MoveGenerator gen(s); \
	auto moves = gen.generateMoves();

#define STANDARD_CHESS_MOVE_GEN_SETUP(fen) CHESS_MOVE_GEN_SETUP(fen, parser.fromStandardChessFen)
#define SPELL_CHESS_MOVE_GEN_SETUP(fen) CHESS_MOVE_GEN_SETUP(fen, parser.fromSpellChessFen)

#define ASSERT_REQUIRED_MOVES_PRESENT(requiredMoves, matchingMoves) bool reqMoveFound; \
	for (const auto& reqMove : requiredMoves) { \
		reqMoveFound = false;\
		for (const auto& move : moves)\
			if (move.move == reqMove) {\
			reqMoveFound = true; \
			matchingMoves.push_back(move); }\
		ASSERT_TRUE(reqMoveFound);	}

#define ASSERT_NO_EXTRA_MOVES( sq, expectedMoves) bool moveWasExpected; \
	for (const auto& move : moves){\
		 moveWasExpected = false; \
		if (move.move.from == Parsing::parseSquareIndex(sq)) { \
			for (const auto & expMove : expectedMoves) \
				if (move.move == expMove)\
					{moveWasExpected = true;  break;} \
			if (!moveWasExpected) FAIL();\
		 }\
}

#define ASSERT_MOVE_IS_ILLEGAL(forbiden_move) bool forbiddenMoveFound = false; \
	SpellChessMove forbiden_move_obj = parseMove(forbiden_move);	\
	for (const auto & move : moves) \
		if  (move.move == forbiden_move_obj) \
			forbiddenMoveFound = true; \
	ASSERT_FALSE(forbiddenMoveFound)

TEST(move_gen, knight_on_empty_board) {
	STANDARD_CHESS_MOVE_GEN_SETUP("k1K5/8/8/8/4N3/8/8/8 w - - 0 1") // knight is on e4
	std::vector<SpellChessMove> requiredMoves = parseMoveList({ "e4d6", "e4f6", "e4c5", "e4g5", "e4c3", "e4g3", "e4d2" , "e4f2"});
	std::vector<MoveCandidate> matchingMoves;
	ASSERT_REQUIRED_MOVES_PRESENT(requiredMoves, matchingMoves);
	ASSERT_NO_EXTRA_MOVES("e4", requiredMoves);
	for (auto& m : matchingMoves) {
		ASSERT_EQ(m.spellPolicy, UserCanAddSpell);
	}
}

TEST(move_gen, knight_in_the_corner) {
	STANDARD_CHESS_MOVE_GEN_SETUP("k1K5/8/8/8/8/8/8/7N w - - 0 1") // knight is on e4

	std::vector<SpellChessMove> requiredMoves = parseMoveList({ "h1g3", "h1f2"});
	std::vector<MoveCandidate> matchingMoves;

	ASSERT_REQUIRED_MOVES_PRESENT(requiredMoves, matchingMoves);
	ASSERT_NO_EXTRA_MOVES("h1", requiredMoves);
	for (auto& m : matchingMoves) {
		ASSERT_EQ(m.spellPolicy, UserCanAddSpell);
	}
}

TEST(move_gen, bishop_on_empty_board) {
	STANDARD_CHESS_MOVE_GEN_SETUP("2K4k/8/8/8/4B3/8/8/N7 w - - 0 1") // bishop is on e4
		std::vector<SpellChessMove> requiredMoves = parseMoveList({ "e4a8", "e4b7", "e4c6", "e4d5", "e4f3", "e4g2", "e4h1", "e4b1", "e4c2", "e4d3", "e4f5", "e4g6", "e4h7"});
	std::vector<MoveCandidate> matchingMoves;

	ASSERT_REQUIRED_MOVES_PRESENT(requiredMoves, matchingMoves);
	ASSERT_NO_EXTRA_MOVES("e4", requiredMoves);
	for (auto& m : matchingMoves) {
		ASSERT_EQ(m.spellPolicy, UserCanAddSpell);
	}
}

TEST(move_gen, rook_on_empty_board) {
	STANDARD_CHESS_MOVE_GEN_SETUP("2K4k/8/8/8/4R3/8/8/8 w - - 0 1") // rook is on e4
		std::vector<SpellChessMove> requiredMoves = parseMoveList({ "e4a4", "e4b4", "e4c4", "e4d4", "e4f4", "e4g4", "e4h4", "e4e8", "e4e7", "e4e6", "e4e5", "e4e3", "e4e2", "e4e1"});
	std::vector<MoveCandidate> matchingMoves;

	ASSERT_REQUIRED_MOVES_PRESENT(requiredMoves, matchingMoves);
	ASSERT_NO_EXTRA_MOVES("e4", requiredMoves);
	for (auto& m : matchingMoves) {
		ASSERT_EQ(m.spellPolicy, UserCanAddSpell);
	}
}

TEST(move_gen, queen_on_empty_board) {
	STANDARD_CHESS_MOVE_GEN_SETUP("2K4k/8/8/8/4Q3/8/8/N7 w - - 0 1") // queen is on e4
		std::vector<SpellChessMove> requiredMoves = parseMoveList({ "e4a8", "e4b7", "e4c6", "e4d5", "e4f3", "e4g2", "e4h1", "e4b1", "e4c2", "e4d3", "e4f5", "e4g6", "e4h7",
			"e4a4", "e4b4", "e4c4", "e4d4", "e4f4", "e4g4", "e4h4", "e4e8", "e4e7", "e4e6", "e4e5", "e4e3", "e4e2", "e4e1" });
	std::vector<MoveCandidate> matchingMoves;

	ASSERT_REQUIRED_MOVES_PRESENT(requiredMoves, matchingMoves);
	ASSERT_NO_EXTRA_MOVES("e4", requiredMoves);
	for (auto& m : matchingMoves) {
		ASSERT_EQ(m.spellPolicy, UserCanAddSpell);
	}
}

TEST(move_gen, king_on_empty_board) {
	STANDARD_CHESS_MOVE_GEN_SETUP("7k/8/8/8/4K3/8/8/8 w - - 0 1") // king is on e4
		std::vector<SpellChessMove> requiredMoves = parseMoveList({ "e4d5", "e4e5", "e4f5", "e4d4", "e4f4", "e4d3", "e4e3", "e4f3" });
	std::vector<MoveCandidate> matchingMoves;

	ASSERT_REQUIRED_MOVES_PRESENT(requiredMoves, matchingMoves);
	ASSERT_NO_EXTRA_MOVES("e4", requiredMoves);
	for (auto& m : matchingMoves) {
		ASSERT_EQ(m.spellPolicy, UserCanAddSpell);
	}
}

TEST(move_gen, king_can_not_step_on_unsafe_square) {
	STANDARD_CHESS_MOVE_GEN_SETUP("3r1r1k/8/8/r7/4K3/r7/8/8 w - - 0 1");
	ASSERT_EQ(moves.size(), 0);
}

TEST(move_gen, piece_can_not_violate_pin) {
	STANDARD_CHESS_MOVE_GEN_SETUP("1b2q2q/8/3RBR2/q2BKB1r/3RBR2/8/1b5b/4r1k1 w - - 0 1");
	ASSERT_EQ(moves.size(), 0);
}

TEST(move_gen, rook_moves_in_pin) {
	STANDARD_CHESS_MOVE_GEN_SETUP("4q2k/8/8/8/8/8/4R3/4K3 w - - 0 1");
	std::vector<SpellChessMove> requiredMoves = parseMoveList({ "e2e3", "e2e4", "e2e5", "e2e6", "e2e7", "e2e8" });
	std::vector<MoveCandidate> matchingMoves;
	ASSERT_REQUIRED_MOVES_PRESENT(requiredMoves, matchingMoves);
	ASSERT_NO_EXTRA_MOVES("e2", requiredMoves);
}

TEST(move_gen, bishop_moves_in_pin) {
	STANDARD_CHESS_MOVE_GEN_SETUP("7k/8/8/q7/8/8/3B4/4K3 w - - 0 1");
	std::vector<SpellChessMove> requiredMoves = parseMoveList({ "d2c3", "d2b4", "d2a5"});
	std::vector<MoveCandidate> matchingMoves;
	ASSERT_REQUIRED_MOVES_PRESENT(requiredMoves, matchingMoves);
	ASSERT_NO_EXTRA_MOVES("d2", requiredMoves);
}

TEST(move_gen, pawn_move) {
	STANDARD_CHESS_MOVE_GEN_SETUP("7k/8/8/8/4P3/8/8/7K w - - 0 1"); 
	std::vector<SpellChessMove> requiredMoves = parseMoveList({ "e4e5"});
	std::vector<MoveCandidate> matchingMoves;
	ASSERT_REQUIRED_MOVES_PRESENT(requiredMoves, matchingMoves);
	ASSERT_NO_EXTRA_MOVES("e4", requiredMoves);
}

TEST(move_gen, pawn_captures) {
	STANDARD_CHESS_MOVE_GEN_SETUP("7k/8/8/3ppp2/4P3/8/8/7K w - - 0 1"); 
	std::vector<SpellChessMove> requiredMoves = parseMoveList({ "e4d5", "e4f5"});
	std::vector<MoveCandidate> matchingMoves;
	ASSERT_REQUIRED_MOVES_PRESENT(requiredMoves, matchingMoves);
	ASSERT_NO_EXTRA_MOVES("e4", requiredMoves);
}

TEST(move_gen, pawn_promotion) {
	STANDARD_CHESS_MOVE_GEN_SETUP("r6k/1P6/8/8/8/8/8/7K w - - 0 1"); // king is on e4
	std::vector<SpellChessMove> requiredMoves = parseMoveList({ "b7b8q", "b7b8r", "b7b8n", "b7b8b",  "b7a8q", "b7a8r", "b7a8n", "b7a8b" });
	std::vector<MoveCandidate> matchingMoves;
	ASSERT_REQUIRED_MOVES_PRESENT(requiredMoves, matchingMoves);
	ASSERT_NO_EXTRA_MOVES("b7", requiredMoves);
}

TEST(move_gen, pawn_double_push) {
	STANDARD_CHESS_MOVE_GEN_SETUP("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"); 
	std::vector<SpellChessMove> requiredMoves = parseMoveList({ "e2e3", "e2e4"});
	std::vector<MoveCandidate> matchingMoves;
	ASSERT_REQUIRED_MOVES_PRESENT(requiredMoves, matchingMoves);
	ASSERT_NO_EXTRA_MOVES("e2", requiredMoves);
}

TEST(move_gen, enpassant) {
	STANDARD_CHESS_MOVE_GEN_SETUP("rnbqkbnr/ppp1pp1p/6p1/3pP3/8/8/PPPP1PPP/RNBQKBNR w KQkq d6 0 3");
	std::vector<SpellChessMove> requiredMoves = parseMoveList({ "e5e6", "e5d6" });
	std::vector<MoveCandidate> matchingMoves;
	ASSERT_REQUIRED_MOVES_PRESENT(requiredMoves, matchingMoves);
	ASSERT_NO_EXTRA_MOVES("e5", requiredMoves);
}

TEST(move_gen, illegal_enpassant_1) {
	STANDARD_CHESS_MOVE_GEN_SETUP("rnbqkbnr/ppp1pp1p/6p1/K2pP2r/8/8/PPPP1PPP/RNBQ1BNR w kq d6 0 3");
	std::vector<SpellChessMove> requiredMoves = parseMoveList({ "e5e6"});
	std::vector<MoveCandidate> matchingMoves;
	ASSERT_REQUIRED_MOVES_PRESENT(requiredMoves, matchingMoves);
	ASSERT_NO_EXTRA_MOVES("e5", requiredMoves);
}

TEST(move_gen, illegal_enpassant_2) {
	STANDARD_CHESS_MOVE_GEN_SETUP("rnbqkbnr/ppp1pp1p/4b1p1/3pP3/8/1K6/PPPP1PPP/RNBQ1BNR w kq d6 0 3");
	std::vector<SpellChessMove> requiredMoves = {};
	std::vector<MoveCandidate> matchingMoves;
	ASSERT_REQUIRED_MOVES_PRESENT(requiredMoves, matchingMoves);
	ASSERT_NO_EXTRA_MOVES("e5", requiredMoves);
}

TEST(move_gen, castling_wh_kingside) {
	STANDARD_CHESS_MOVE_GEN_SETUP("r1bqk1nr/pppp1ppp/2n5/2b1p3/2B1P3/5N2/PPPP1PPP/RNBQK2R w KQkq - 4 4");
	std::vector<SpellChessMove> requiredMoves = parseMoveList({"e1e2", "e1f1", "e1g1"});
	std::vector<MoveCandidate> matchingMoves;
	ASSERT_REQUIRED_MOVES_PRESENT(requiredMoves, matchingMoves);
	ASSERT_NO_EXTRA_MOVES("e1", requiredMoves);
}

TEST(move_gen, castling_bl_queenside) {
	STANDARD_CHESS_MOVE_GEN_SETUP("r3k2r/ppp2ppp/2np1n2/2b1p3/2B1P3/2NP3P/PPP2PP1/R1B2RK1 b kq - 0 9");
	std::vector<SpellChessMove> requiredMoves = parseMoveList({ "e8e7", "e8d7", "e8f8", "e8d8", "e8c8", "e8g8"});
	std::vector<MoveCandidate> matchingMoves;
	ASSERT_REQUIRED_MOVES_PRESENT(requiredMoves, matchingMoves);
	ASSERT_NO_EXTRA_MOVES("e8", requiredMoves);
}

TEST(move_gen, castling_is_illegal_in_check) {
	STANDARD_CHESS_MOVE_GEN_SETUP("r1bqk1nr/pppp1ppp/2n5/2b1p3/2B1r3/5N2/PPPP1PPP/RNBQK2R w KQkq - 4 4");
	std::vector<SpellChessMove> requiredMoves = parseMoveList({ "e1f1" });
	std::vector<MoveCandidate> matchingMoves;
	ASSERT_REQUIRED_MOVES_PRESENT(requiredMoves, matchingMoves);
	ASSERT_NO_EXTRA_MOVES("e1", requiredMoves);
}

TEST(move_gen, castling_is_illegal_another_example) {
	SPELL_CHESS_MOVE_GEN_SETUP("r1b1kbn1/pppp1p1p/2n5/4p3/4P3/2N2N2/PPPP1PqP/R1B1KR2 w KQk - 0 1 d6 - F32/J20/f40/j20");
	ASSERT_MOVE_IS_ILLEGAL("e1g1");
}

TEST(move_gen, castling_is_illegal_if_path_is_unsafe) {
	STANDARD_CHESS_MOVE_GEN_SETUP("4k3/8/8/8/2q5/8/3P4/R3K2R w KQ - 0 1");
	std::vector<SpellChessMove> requiredMoves = parseMoveList({ "e1d1", "e1f2"});
	std::vector<MoveCandidate> matchingMoves;
	ASSERT_REQUIRED_MOVES_PRESENT(requiredMoves, matchingMoves);
	ASSERT_NO_EXTRA_MOVES("e1", requiredMoves);
}

TEST(move_gen, check1) {
	STANDARD_CHESS_MOVE_GEN_SETUP("r3kb1r/pppq1Bpp/2np1nb1/4p1N1/4P1P1/2NP3P/PPP2P2/R1BQK2R b KQkq - 0 9");
	std::vector<SpellChessMove> requiredMoves = parseMoveList({ "e8d8", "e8e7", "d7f7", "g6f7"});
	std::vector<MoveCandidate> matchingMoves;
	ASSERT_REQUIRED_MOVES_PRESENT(requiredMoves, matchingMoves);
	ASSERT_EQ(moves.size(), requiredMoves.size());
}

TEST(move_gen, check2) {
	STANDARD_CHESS_MOVE_GEN_SETUP("7k/3P4/2R1K3/8/1N6/8/b7/5r2 w - - 0 1");
	std::vector<SpellChessMove> requiredMoves = parseMoveList({ "e6e7", "e6d6", "e6e5", "c6c4", "b4a2", "b4d5"});
	std::vector<MoveCandidate> matchingMoves;
	ASSERT_REQUIRED_MOVES_PRESENT(requiredMoves, matchingMoves);
	ASSERT_EQ(moves.size(), requiredMoves.size());
}

TEST(move_gen, player_can_cast_jump) {
	SPELL_CHESS_MOVE_GEN_SETUP("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1 - - F50/J20/f50/j20");
	std::vector<SpellChessMove> requiredMoves = parseMoveList({ "jump@h2&h1h3", "jump@h2&h1h4", "jump@h2&h1h5", "jump@h2&h1h6", "jump@h2&h1h7" });
	std::vector<MoveCandidate> matchingMoves;
	ASSERT_REQUIRED_MOVES_PRESENT(requiredMoves, matchingMoves);
	ASSERT_NO_EXTRA_MOVES("h1", requiredMoves);

	for (auto m : matchingMoves)
		ASSERT_EQ(m.spellPolicy, SpellGeneratorEnforced);
}

TEST(move_gen, player_can_not_cast_jump_without_usages) {
	SPELL_CHESS_MOVE_GEN_SETUP("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1 - - F50/J00/f50/j20");
	std::vector<SpellChessMove> requiredMoves = {};
	std::vector<MoveCandidate> matchingMoves;
	ASSERT_REQUIRED_MOVES_PRESENT(requiredMoves, matchingMoves);
	ASSERT_NO_EXTRA_MOVES("h1", requiredMoves);
}

TEST(move_gen, player_can_go_trough_jumpable_square) {
	SPELL_CHESS_MOVE_GEN_SETUP("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1 - h2 F50/J20/f50/j20");
	std::vector<SpellChessMove> requiredMoves = parseMoveList({ "h1h3", "h1h4", "h1h5", "h1h6", "h1h7", "jump@h7&h1h8" });
	std::vector<MoveCandidate> matchingMoves;
	
	ASSERT_REQUIRED_MOVES_PRESENT(requiredMoves, matchingMoves);
	ASSERT_NO_EXTRA_MOVES("h1", requiredMoves);
}

TEST(move_gen, player_can_use_freeze_to_escape_from_check) {
	SPELL_CHESS_MOVE_GEN_SETUP("7k/3P4/2R1K3/8/1N6/8/b7/5r2 w - - 0 1 - - F50/J20/f50/j20");

	std::vector<SpellChessMove> requiredMoves = parseMoveList({ "b4a2", "b4d5", "freeze@a2&b4c2", "freeze@b2&c6c1"});
	std::vector<MoveCandidate> matchingMoves;

	ASSERT_REQUIRED_MOVES_PRESENT(requiredMoves, matchingMoves);
}

TEST(move_gen, freeze_can_break_pin) {
	SPELL_CHESS_MOVE_GEN_SETUP("7k/3P4/4K3/8/1NR5/8/b7/5r2 w - - 1 1 - - F50/J20/f50/j20");
	std::vector<SpellChessMove> requiredMoves = parseMoveList({ "freeze@a2&c4h4", "freeze@b2&c4c1" });
	std::vector<MoveCandidate> matchingMoves;

	ASSERT_REQUIRED_MOVES_PRESENT(requiredMoves, matchingMoves);

	for (auto m : matchingMoves)
		ASSERT_EQ(m.spellPolicy, SpellGeneratorEnforced);
}

TEST(move_gen, freezing_moving_piece_is_illegal) {
	SPELL_CHESS_MOVE_GEN_SETUP("7k/3P4/4K3/8/1NR5/8/b7/5r2 w - - 1 1 - - F50/J20/f50/j20");
	for(const auto & m : moves) 
		if (m.move.from == Parsing::parseSquareIndex("c4")) {
			ASSERT_NE(m.move.freezeSquare, Parsing::parseSquareIndex("b3"));
			ASSERT_NE(m.move.freezeSquare, Parsing::parseSquareIndex("c3"));
		}
}

TEST(move_gen, king_can_not_kiss) {
	STANDARD_CHESS_MOVE_GEN_SETUP("8/3n4/6P1/8/8/1p6/5k2/3R3K b - - 21 128");
	ASSERT_MOVE_IS_ILLEGAL("f2g2");
}

TEST(move_gen, pawn_capture) {
	SPELL_CHESS_MOVE_GEN_SETUP("8/8/3k2K1/1n3R2/2P2p1P/5r2/P3P3/8 w - - 1 1 - - F00/J10/f00/j00");
	std::vector<SpellChessMove> requiredMoves = parseMoveList({ "c4b5" });
	std::vector<MoveCandidate> matchingMoves;
}

TEST(move_gen, can_ignore_check_to_deliver_checkmate) {
	SPELL_CHESS_MOVE_GEN_SETUP("r1b1r1k1/pp3p1p/2p2np1/8/2P3R1/8/PP1P1n1P/R1BK4 w - - 1 1 e2 - F00/J20/f12/j20");
	std::vector<SpellChessMove> requiredMoves = parseMoveList({ "jump@g6&g4g8" });
	std::vector<MoveCandidate> matchingMoves;
}

TEST(move_gen, can_not_cast_freeze_on_current_freeze_sq) {
	SPELL_CHESS_MOVE_GEN_SETUP("r1b1kb1r/ppNqp1pp/2n5/2np1p2/8/2P2NP1/PPQPPPBP/R1B1KR2 b Qkq - 0 1 d6 - F32/J20/f40/j20");
	ASSERT_MOVE_IS_ILLEGAL("freeze@d6&e8d8");
}