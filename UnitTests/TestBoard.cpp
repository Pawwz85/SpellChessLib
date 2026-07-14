#include "pch.h"
#include <SpellChess/Board.h>
#include <SpellChess/StringTokenizer.h>
#include <algorithm>
using namespace SpellChessLib;

TEST(Square, empty) {
	BoardSquare sq;
	sq.piece.owner = Nobody;
	sq.piece.type = None;
	ASSERT_TRUE(sq.isEmpty());
}

TEST(Square, not_empty) {
	BoardSquare sq;
	sq.piece.owner = White;
	sq.piece.type = Pawn;
	ASSERT_FALSE(sq.isEmpty());
}

TEST(Square, index_to_string) {
	ASSERT_EQ(squareIndexToString(0), "a8");
	ASSERT_EQ(squareIndexToString(63), "h1");
	ASSERT_EQ(squareIndexToString(7), "h8");
	ASSERT_EQ(squareIndexToString(15), "h7");
}

TEST(Move, toString) {
	SpellChessMove move;
	move.from = 0;
	move.to = 7;
	move.freezeSquare = move.jumpSquare = NoSquare;
	move.promoteTo = None;
	ASSERT_EQ(spellChessMoveToString(move), "a8h8");
}

TEST(Move, with_promotion_toString) {
	SpellChessMove move;
	move.from = 15;
	move.to = 7;
	move.freezeSquare = move.jumpSquare = NoSquare;
	move.promoteTo = Knight;
	ASSERT_EQ(spellChessMoveToString(move), "h7h8n");
	move.promoteTo = Bishop;
	ASSERT_EQ(spellChessMoveToString(move), "h7h8b");
	move.promoteTo = Rook;
	ASSERT_EQ(spellChessMoveToString(move), "h7h8r");
	move.promoteTo = Queen;
	ASSERT_EQ(spellChessMoveToString(move), "h7h8q");
}

TEST(Move, with_freeze_spell) {
	SpellChessMove move;
	move.from = 0;
	move.to = 7;
	move.freezeSquare = 15;
	move.jumpSquare = NoSquare;
	move.promoteTo = None;
	ASSERT_EQ(spellChessMoveToString(move), "freeze@h7&a8h8");
}

TEST(Move, with_jump_spell) {
	SpellChessMove move;
	move.from = 0;
	move.to = 7;
	move.freezeSquare = NoSquare;
	move.jumpSquare = 6;
	move.promoteTo = None;
	ASSERT_EQ(spellChessMoveToString(move), "jump@g8&a8h8");
}

TEST(Move, invalidMove1) {
	SpellChessMove move;
	move.from = 100;
	move.to = 7;
	move.freezeSquare = NoSquare;
	move.jumpSquare = 6;
	move.promoteTo = None;
	ASSERT_THROW(validateSpellChessMove(move), InvalidMoveException);
}

TEST(Move, invalidMove2) {
	// moves that use both spell and freeze are illegal
	SpellChessMove move;
	move.from = 0;
	move.to = 7;
	move.freezeSquare = 5;
	move.jumpSquare = 6;
	move.promoteTo = None;
	ASSERT_THROW(validateSpellChessMove(move), InvalidMoveException);
}

class GameStateBuilder {
public:
	BoardSquare board[64];
	
	bool castlingRights[4];
	SpellCounter spells[4];
	
	PlayerColor sideToMove = White;
	
	SquareIndex enpassantSq = NoSquare;
	SquareIndex jumpSq = NoSquare;
	SquareIndex freezeSq = NoSquare;
	uint16_t halfMoveCounter = 0;
	uint16_t fullMoveCounter = 1;
	
	GameStateBuilder() {
		for (size_t i = 0; i < 64; ++i)
			std::fill_n((char*)&(board[i]), sizeof(BoardSquare), 0);
		
		SpellCounter zeroCounter = {FreezeSpell, 0, 0};

		std::fill_n(castlingRights, 4, false);
		std::fill_n(spells, 4, zeroCounter);
		spells[1].spellId = JumpSpell;
		spells[3].spellId = JumpSpell;
	}

	GameStateBuilder& spawnPiece(size_t index, Piece p) {
		board[index].piece = p;
		return *this;
	}
	
	GameStateBuilder& despawn(size_t index) {
		board[index].piece.type = None;
		board[index].piece.owner = Nobody;
		return *this;
	}

	// 0b1111 = KQkq, 0 = -
	GameStateBuilder& setCastlingRights(uint8_t castlingFlags) {
		for (size_t i = 0; i < 4; ++i)
			castlingRights[3 - i] = castlingFlags & (1 << i);
		return *this;
	}

	GameStateBuilder& setSpell(PlayerColor side, SpellIdentificator spell, int cooldown, int usagesLeft) {
		size_t index = (side - 1) * 2 + spell;
		spells[index].cooldown = cooldown;
		spells[index].usagesLeft = usagesLeft;
		return *this;
	}

	GameStateBuilder& setToStartPos() {
		spawnPiece(0, { Rook, Black });
		spawnPiece(1, { Knight, Black });
		spawnPiece(2, { Bishop, Black});
		spawnPiece(3, { Queen, Black });
		spawnPiece(4, { King, Black });
		spawnPiece(5, { Bishop, Black });
		spawnPiece(6, { Knight, Black });
		spawnPiece(7, { Rook, Black });
		
		for (size_t i = 8; i <= 15; ++i)
			spawnPiece(i, { Pawn, Black });
		

		spawnPiece(56, { Rook, White });
		spawnPiece(57, { Knight, White });
		spawnPiece(58, { Bishop, White });
		spawnPiece(59, { Queen, White });
		spawnPiece(60, { King, White });
		spawnPiece(61, { Bishop, White });
		spawnPiece(62, { Knight, White });
		spawnPiece(63, { Rook, White });

		for (size_t i = 48; i <= 55; ++i)
			spawnPiece(i, { Pawn, White });

		setCastlingRights(0b1111);
		sideToMove = White;
		enpassantSq = NoSquare;
		freezeSq = NoSquare;
		jumpSq = NoSquare;
		halfMoveCounter = 0;
		fullMoveCounter = 1;

		for (size_t i = 0; i < 4; ++i)
			spells[i].cooldown = spells[i].usagesLeft = 0;

		return *this;
	}
	
	GameStateBuilder& setToSpellStartPos() {
		setToStartPos();
		spells[0].usagesLeft = spells[2].usagesLeft = 5;
		spells[1].usagesLeft = spells[3].usagesLeft = 2;
		return *this;
	}

	GameState build() {
		return GameState(board, sideToMove, castlingRights, enpassantSq, halfMoveCounter, fullMoveCounter);
	}

	GameState buildSpellVersion(){
		return GameState(board, sideToMove, castlingRights, enpassantSq, halfMoveCounter, fullMoveCounter, freezeSq, jumpSq, spells);
	}
};

TEST(GameState, toFenStartPos) {
	GameStateBuilder builder;
	ASSERT_EQ(builder.setToStartPos().build().toFen(), "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
	ASSERT_EQ(builder.setToSpellStartPos().buildSpellVersion().toFen(), "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1 - - F50/J20/f50/j20");
}

TEST(GameState, toFenStartPos_DifferentCastlingRights) {
	GameStateBuilder builder;
	builder.setToStartPos();
	ASSERT_EQ(builder.setCastlingRights(0b0011).build().toFen(), "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w kq - 0 1");
	ASSERT_EQ(builder.setCastlingRights(0b1100).build().toFen(), "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQ - 0 1");
	ASSERT_EQ(builder.setCastlingRights(0).build().toFen(), "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w - - 0 1");
	ASSERT_EQ(builder.setCastlingRights(0b1001).build().toFen(), "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w Kq - 0 1");
	ASSERT_EQ(builder.setCastlingRights(0b0110).build().toFen(), "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w Qk - 0 1");
}

TEST(GameState, toFen_EnpassantSquare) {
	GameStateBuilder builder;
	builder.setToStartPos().despawn(63 - 8).spawnPiece(63 - 24, {Pawn, White}); // despawn h2 pawn and spawn it on h4
	builder.enpassantSq = 63 - 16;
	builder.sideToMove = Black;
	ASSERT_EQ(builder.build().toFen(), "rnbqkbnr/pppppppp/8/8/7P/8/PPPPPPP1/RNBQKBNR b KQkq h3 0 1");
}

TEST(GameState, toFen_FreezeSquare) {
	GameStateBuilder builder;
	builder.setToSpellStartPos().setSpell(White, FreezeSpell, SpellCooldown, 4);
	builder.despawn(52).spawnPiece(44, { Pawn, White });
	builder.freezeSq = 5;
	builder.sideToMove = Black;
	ASSERT_EQ(builder.buildSpellVersion().toFen(), "rnbqkbnr/pppppppp/8/8/8/4P3/PPPP1PPP/RNBQKBNR b KQkq - 0 1 f8 - F42/J20/f50/j20");
}

TEST(GameState, toFen_JumpSquare) {
	GameStateBuilder builder;
	builder.setToSpellStartPos().setSpell(White, JumpSpell, SpellCooldown, 1);
	builder.despawn(63).despawn(15).spawnPiece(15, {Rook, White}).setCastlingRights(0b0111);
	builder.jumpSq = 55;
	builder.sideToMove = Black;
	ASSERT_EQ(builder.buildSpellVersion().toFen(), "rnbqkbnr/pppppppR/8/8/8/8/PPPPPPPP/RNBQKBN1 b Qkq - 0 1 - h2 F50/J12/f50/j20");
}

TEST(StandardChessFenValidation, correct_cases) {
	StandardChessFenValidator validator;

	ASSERT_TRUE(validator.isValidFen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"));
	ASSERT_TRUE(validator.isValidFen("4k3/7p/4p3/8/3KP3/8/6PP/8 b - - 0 1"));
	ASSERT_TRUE(validator.isValidFen("rnbqkbnr/pppppppp/8/8/7P/8/PPPPPPP1/RNBQKBNR b KQkq h3 0 1"));
}

TEST(StandardChessFenValidation, piece_list_missing_row) {
	StandardChessFenValidator validator;
	// missing row
	ASSERT_FALSE(validator.isValidFen("rnbqkbnr/pppppppp/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"));
}

TEST(StandardChessFenValidation, piece_list_extra_row) {
	StandardChessFenValidator validator;
	// extra row
	ASSERT_FALSE(validator.isValidFen("rnbqkbnr/pppppppp/8/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"));
}

TEST(StandardChessFenValidation, piece_list_empty_row) {
	StandardChessFenValidator validator;
	// extra row with 0 content
	ASSERT_FALSE(validator.isValidFen("rnbqkbnr/pppppppp//8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"));
}

TEST(StandardChessFenValidation, piece_list_9) {
	StandardChessFenValidator validator;
	// 9 in input
	ASSERT_FALSE(validator.isValidFen("rnbqkbnr/pppppppp/9/7/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"));
}

TEST(StandardChessFenValidation, piece_list_0) {
	StandardChessFenValidator validator;
	// 0 in input
	ASSERT_FALSE(validator.isValidFen("rnbqkbnr/pppppppp/08/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"));
}

TEST(StandardChessFenValidation, piece_list_rows_do_not_have_8_squares_but_sum_is_still_64) {
	StandardChessFenValidator validator;
	ASSERT_FALSE(validator.isValidFen("rnbqkb/2pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"));
}

TEST(StandardChessFenValidation, piece_list_consecutive_numbers) {
	StandardChessFenValidator validator;
	// numbers shouldn't be consecutive
	ASSERT_FALSE(validator.isValidFen("rnbqkbnr/pppppppp/8/44/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"));
}

TEST(StandardChessFenValidation, piece_list_unknown_character) {
	StandardChessFenValidator validator;
	// replaced 'r' with 'l'
	ASSERT_FALSE(validator.isValidFen("lnbqkbnl/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"));
}

TEST(StandardChessFenValidation, piece_list_unknown_side_to_move) {
	StandardChessFenValidator validator;
	ASSERT_FALSE(validator.isValidFen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR - KQkq - 0 1"));
}

TEST(StandardChessFenValidation, piece_list_unknown_castling_rights) {
	StandardChessFenValidator validator;
	ASSERT_FALSE(validator.isValidFen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w STst - 0 1"));
}

TEST(StandardChessFenValidation, piece_list_castling_rights_not_in_order) {
	StandardChessFenValidator validator;
	ASSERT_FALSE(validator.isValidFen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w kqKQ - 0 1"));
}

TEST(StandardChessFenValidation, piece_list_castling_rights_duplicates) {
	StandardChessFenValidator validator;
	ASSERT_FALSE(validator.isValidFen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR 2 KKQkq - 0 1"));
}

TEST(StandardChessFenValidation, piece_list_wrong_enpassant_square1) {
	StandardChessFenValidator validator;
	ASSERT_FALSE(validator.isValidFen("rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR w KQkq e3f 0 1"));
}

TEST(StandardChessFenValidation, piece_list_wrong_enpassant_square2) {
	StandardChessFenValidator validator;
	ASSERT_FALSE(validator.isValidFen("rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR w KQkq e9 0 1"));
}

TEST(StandardChessFenValidation, piece_list_wrong_enpassant_square3) {
	StandardChessFenValidator validator;
	ASSERT_FALSE(validator.isValidFen("rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR w KQkq z2 0 1"));
}

TEST(StandardChessFenValidation, piece_list_wrong_counter) {
	StandardChessFenValidator validator;
	ASSERT_FALSE(validator.isValidFen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - zero 1"));
}

TEST(StandardChessFenValidation, piece_list_half_move_counter_below_0) {
	StandardChessFenValidator validator;
	ASSERT_FALSE(validator.isValidFen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - -1 1"));
}

TEST(StandardChessFenValidation, piece_list_half_move_counter_beyond_100) {
	StandardChessFenValidator validator;
	ASSERT_FALSE(validator.isValidFen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 101 1"));
}

TEST(StandardChessFenValidation, piece_list_move_counter_below_0) {
	StandardChessFenValidator validator;
	ASSERT_FALSE(validator.isValidFen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 -9"));
}


TEST(SpellChessFenValidation, correct_case) {
	SpellChessFenValidator validator;
	ASSERT_TRUE(validator.isValidFen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1 - - F50/f50/J20/j20"));
	ASSERT_TRUE(validator.isValidFen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1 h2 - F50/f50/J20/j20"));
	ASSERT_TRUE(validator.isValidFen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1 - h2 F50/f50/J20/j20"));
}

TEST(SpellChessFenValidation, incorrect_spell_labels) {
	SpellChessFenValidator validator;
	ASSERT_FALSE(validator.isValidFen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1 - - T50/f50/J20/j20"));
	ASSERT_FALSE(validator.isValidFen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1 - - F50/t50/J20/j20"));
	ASSERT_FALSE(validator.isValidFen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1 - - F50/f50/G20/j20"));
	ASSERT_FALSE(validator.isValidFen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1 - - F50/f50/J20/k20"));
}

TEST(SpellChessFenValidation, incorrect_spell_limit_exceded) {
	SpellChessFenValidator validator;
	ASSERT_FALSE(validator.isValidFen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1 - - F70/f50/J20/j20"));
	ASSERT_FALSE(validator.isValidFen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1 - - F50/f70/J20/j20"));
	ASSERT_FALSE(validator.isValidFen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1 - - F50/f50/J30/j20"));
	ASSERT_FALSE(validator.isValidFen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1 - - F70/f50/J20/j30"));
}

TEST(FenParser, standardChess1) {
	FenParser parser;
	GameState state = parser.fromStandardChessFen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
	ASSERT_EQ(state.toFen(), "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
}

TEST(FenParser, standardChess2) {
	FenParser parser;
	GameState state = parser.fromStandardChessFen("rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1");
	ASSERT_EQ(state.toFen(), "rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1");
}

TEST(FenParser, standardChess3) {
	FenParser parser;
	GameState state = parser.fromStandardChessFen("r2k3r/pp1b1Bpp/2p5/1n2p3/8/8/PBP2PPP/3R1RK1 w - - 4 14");
	ASSERT_EQ(state.toFen(), "r2k3r/pp1b1Bpp/2p5/1n2p3/8/8/PBP2PPP/3R1RK1 w - - 4 14");
}

TEST(MoveParser, e2e4) {
	SpellChessMove move;

	move = "e2e4";
	ASSERT_EQ(move.from, Parsing::parseSquareIndex("e2"));
	ASSERT_EQ(move.to, Parsing::parseSquareIndex("e4"));
	ASSERT_EQ(move.freezeSquare, NoSquare);
	ASSERT_EQ(move.jumpSquare, NoSquare);
	ASSERT_EQ(move.promoteTo, None);
}

TEST(MoveParser, valid_promotions) {
	SpellChessMove move;

	move = "e7e8q";
	ASSERT_EQ(move.from, Parsing::parseSquareIndex("e7"));
	ASSERT_EQ(move.to, Parsing::parseSquareIndex("e8"));
	ASSERT_EQ(move.freezeSquare, NoSquare);
	ASSERT_EQ(move.jumpSquare, NoSquare);
	ASSERT_EQ(move.promoteTo, Queen);

	move = "e7e8n";
	ASSERT_EQ(move.promoteTo, Knight);
	move = "e7e8b";
	ASSERT_EQ(move.promoteTo, Bishop);
	move = "e7e8r";
	ASSERT_EQ(move.promoteTo, Rook);
}

TEST(MoveParser, invalid_promotion) {
	SpellChessMove move;

	ASSERT_THROW(move = "e7e8k", Parsing::ParsingError);
}

TEST(MoveParser, moveWithFreezeSQ) {
	SpellChessMove move;
	move = "freeze@f6&c1g5";
	ASSERT_EQ(move.freezeSquare, Parsing::parseSquareIndex("f6"));
	ASSERT_EQ(move.jumpSquare, NoSquare);
	ASSERT_EQ(move.from, Parsing::parseSquareIndex("c1"));
	ASSERT_EQ(move.to, Parsing::parseSquareIndex("g5"));
	ASSERT_EQ(move.promoteTo, None);
}

TEST(MoveParser, moveWithJumpSq) {
	SpellChessMove move;
	move = "jump@h2&h1h7";
	ASSERT_EQ(move.freezeSquare, NoSquare);
	ASSERT_EQ(move.jumpSquare, Parsing::parseSquareIndex("h2"));
	ASSERT_EQ(move.from, Parsing::parseSquareIndex("h1"));
	ASSERT_EQ(move.to, Parsing::parseSquareIndex("h7"));
	ASSERT_EQ(move.promoteTo, None);
}

TEST(MoveParser, emptySpellStrings) {
	SpellChessMove move;
	ASSERT_THROW(move = "@e2e4", Parsing::ParsingError);
	ASSERT_THROW(move = "&e2e4", Parsing::ParsingError);
}

TEST(MoveParser, usesSpellButSquareIsEmpty) {
	SpellChessMove move;
	ASSERT_THROW(move = "freeze@-&e2e4", Parsing::ParsingError);
}

TEST(make_move, non_pawn_piece_can_not_take_enpassant) {
	FenParser parser;
	GameState s = parser.fromStandardChessFen("rnb1k1nr/ppp1bppp/8/3p4/3PN3/4PN2/PPP1KPPP/R1BQ1B2 w kq d6 0 1");
	s.makeMove("e4d6");

	auto enpassant_pawn = s.getSquare(Parsing::parseSquareIndex("d5")).piece;
	
	ASSERT_EQ(enpassant_pawn.owner, Black);
	ASSERT_EQ(enpassant_pawn.type, Pawn);
};
