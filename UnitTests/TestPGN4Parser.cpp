#include "pch.h"

#include <SpellChess/PGN4Parser.h>
#include <SpellChess/StringTokenizer.h>
#include <SpellChess/PGN4Exporter.h>
#include <sstream>

static std::string spellFieldFen(const std::string& fields, bool includeFields = true) {
    const std::string emptyBoard =
        "x,x,x,x,x,x,x,x,x,x,x,x,x,x/"
        "x,x,x,x,x,x,x,x,x,x,x,x,x,x/"
        "x,x,x,x,x,x,x,x,x,x,x,x,x,x/"
        "x,x,x,x,x,x,x,x,x,x,x,x,x,x/"
        "x,x,x,x,x,x,x,x,x,x,x,x,x,x/"
        "x,x,x,x,x,x,x,x,x,x,x,x,x,x/"
        "x,x,x,x,x,x,x,x,x,x,x,x,x,x/"
        "x,x,x,x,x,x,x,x,x,x,x,x,x,x/"
        "x,x,x,x,x,x,x,x,x,x,x,x,x,x/"
        "x,x,x,x,x,x,x,x,x,x,x,x,x,x/"
        "x,x,x,x,x,x,x,x,x,x,x,x,x,x/"
        "x,x,x,x,x,x,x,x,x,x,x,x,x,x/"
        "x,x,x,x,x,x,x,x,x,x,x,x,x,x/"
        "x,x,x,x,x,x,x,x,x,x,x,x,x,x";
    return "R-0,1,0,1-1,0,1,0-1,0,1,0-0,0,0,0-0-{'spells':('jump_0x0,freeze_0x0','', 'jump_0x0,freeze_0x0','')" +
        (includeFields ? ",'spellFields':(" + fields + ")" : "") + "}-" + emptyBoard;
}

TEST(PGN4SpellFields, parsesMultipleFieldsAndCoordinates) {
    auto state = SpellChessLib::PGN4Parser::parseFen4(spellFieldFen("'1_0_jump@d4','1_1_freeze@k11'"));
    ASSERT_EQ(state.getSpellFields().size(), 2u);
    EXPECT_EQ(state.getSpellFields()[0], (SpellChessLib::SpellField{SpellChessLib::JumpSpell, SpellChessLib::White, 56, 1}));
    EXPECT_EQ(state.getSpellFields()[1], (SpellChessLib::SpellField{SpellChessLib::FreezeSpell, SpellChessLib::Black, 7, 1}));
    EXPECT_TRUE(state.getSquare(7).frozen);
    EXPECT_TRUE(state.getSquare(56).jumpable);
}

TEST(PGN4SpellFields, missingFieldIsBackwardCompatible) {
    auto state = SpellChessLib::PGN4Parser::parseFen4(spellFieldFen("", false));
    EXPECT_TRUE(state.getSpellFields().empty());
}

TEST(PGN4SpellFields, malformedFieldIsRejected) {
    EXPECT_THROW(SpellChessLib::PGN4Parser::parseFen4(spellFieldFen("'1_2_jump@3,3'")), SpellChessLib::InvalidPGN4);
    EXPECT_THROW(SpellChessLib::PGN4Parser::parseFen4(spellFieldFen("'1_0_teleport@3,3'")), SpellChessLib::InvalidPGN4);
    EXPECT_THROW(SpellChessLib::PGN4Parser::parseFen4(spellFieldFen("'1_0_jump@c3'")), SpellChessLib::InvalidPGN4);
}

TEST(PGN4SpellFields, updatedAfterFreezeMove) {
    SpellChessLib::FenParser parser;
    auto state = parser.fromSpellChessFen(SpellChessLib::SPELL_START_POS);

    state.makeMove(SpellChessLib::SpellChessMove("freeze@d5&e2e4"));

    ASSERT_EQ(state.getSpellFields().size(), 1u);
    EXPECT_EQ(state.getSpellFields()[0], (SpellChessLib::SpellField{
        SpellChessLib::FreezeSpell, SpellChessLib::White, 27, 1
    }));
    EXPECT_EQ(state.getFreezeSquare(), 27);
}

TEST(PGN4SpellFields, updatedAfterJumpCast) {
    SpellChessLib::FenParser parser;
    auto state = parser.fromSpellChessFen(SpellChessLib::SPELL_START_POS);

    state.makeMove(SpellChessLib::SpellChessMove("jump@d4&e2e4"));

    ASSERT_EQ(state.getSpellFields().size(), 1u);
    EXPECT_EQ(state.getSpellFields()[0], (SpellChessLib::SpellField{
        SpellChessLib::JumpSpell, SpellChessLib::White, 35, 1
    }));
    EXPECT_EQ(state.getJumpSquare(), 35);
    EXPECT_TRUE(state.getSquare(35).jumpable);
}

TEST(PGN4SpellFields, expiresAfterNextMove) {
    SpellChessLib::FenParser parser;
    auto state = parser.fromSpellChessFen(SpellChessLib::SPELL_START_POS);

    state.makeMove(SpellChessLib::SpellChessMove("freeze@d5&e2e4"));
    ASSERT_FALSE(state.getSpellFields().empty());

    state.makeMove(SpellChessLib::SpellChessMove("e7e5"));

    EXPECT_TRUE(state.getSpellFields().empty());
    EXPECT_EQ(state.getFreezeSquare(), SpellChessLib::NoSquare);
    EXPECT_FALSE(state.getSquare(27).frozen);
}

TEST(PGN4SpellFields, legacyFenFieldSurvivesUiExportAfterNf6) {
    SpellChessLib::FenParser parser;
    auto state = parser.fromSpellChessFen(
        "rnbqkbnr/pppp1ppp/8/1B2p3/4P3/8/PPPP1PPP/RNBQK1NR b KQkq - 1 2 c7 - F42/J20/f50/j20");

    ASSERT_EQ(state.getSpellFields().size(), 1u);
    EXPECT_EQ(state.getSpellFields()[0], (SpellChessLib::SpellField{
        SpellChessLib::FreezeSpell, SpellChessLib::White, 10, 1
    }));

    SpellChessLib::Annotation::GameAnnotations annotations;
    annotations.root = state;
    annotations.annotatedSequence.appendMove(SpellChessLib::SpellChessMove("g8f6"));

    std::stringstream output;
    SpellChessLib::PGN4Exporter exporter(&output);
    exporter.exportPGN4(state, annotations);

    EXPECT_NE(output.str().find("'spellFields':('1_0_freeze@f10')"), std::string::npos);
}


TEST(PGN4Tokenizer, empty_string) {
    auto tokens = SpellChessLib::PGN4Parser::tokenize("");
    ASSERT_EQ(tokens.size(), 0);
};

TEST(PGN4Tokenizer, singular_tag) {
    auto tokens = SpellChessLib::PGN4Parser::tokenize("[Result \"1-0\"]");

    ASSERT_EQ(tokens.size(), 4);
    ASSERT_EQ(tokens[0].type, SpellChessLib::PGN4Parser::TokenType::square_bracket_open);
    ASSERT_EQ(tokens[1].type, SpellChessLib::PGN4Parser::TokenType::word);
    ASSERT_EQ(tokens[1].value, "Result");
    ASSERT_EQ(tokens[2].type, SpellChessLib::PGN4Parser::TokenType::string_literal);
    ASSERT_EQ(tokens[2].value, "1-0");
    ASSERT_EQ(tokens[3].type, SpellChessLib::PGN4Parser::TokenType::square_bracket_close);
};

TEST(PGN4Tokenizer, singular_comment) {
    auto tokens = SpellChessLib::PGN4Parser::tokenize("{White won by checkmate}");

    ASSERT_EQ(tokens.size(), 1);
    ASSERT_EQ(tokens[0].type, SpellChessLib::PGN4Parser::TokenType::comment);
    ASSERT_EQ(tokens[0].value, "White won by checkmate");
};

TEST(PGN4Tokenizer, string_literal_in_comment) {
    auto tokens = SpellChessLib::PGN4Parser::tokenize("{\"string\"}");
    ASSERT_EQ(tokens.size(), 1);
    ASSERT_EQ(tokens[0].type, SpellChessLib::PGN4Parser::TokenType::comment);
    ASSERT_EQ(tokens[0].value, "\"string\"");
};

TEST(PGN4Tokenizer, extra_opening_curly_bracket_in_comment) {
    ASSERT_THROW(
        (void)SpellChessLib::PGN4Parser::tokenize("{{White won by checkmate}"),
        SpellChessLib::InvalidPGN4
    );
};

TEST(PGN4Tokenizer, extra_closing_curly_bracket_in_comment) {
        ASSERT_THROW(
        (void)SpellChessLib::PGN4Parser::tokenize("{White won by }checkmate}"),
        SpellChessLib::InvalidPGN4
    );
};

TEST(PGN4Tokenizer, brackets) {
  auto tokens = SpellChessLib::PGN4Parser::tokenize("jump@k5&Rk4xk6 (Rk4xk3 Nj9-h10)");
  ASSERT_EQ(tokens.size(), 5);
  ASSERT_EQ(tokens[0].type, SpellChessLib::PGN4Parser::TokenType::word);  
  ASSERT_EQ(tokens[0].value, "jump@k5&Rk4xk6");  
  ASSERT_EQ(tokens[1].type, SpellChessLib::PGN4Parser::TokenType::bracket_open);  
  ASSERT_EQ(tokens[2].type, SpellChessLib::PGN4Parser::TokenType::word);  
  ASSERT_EQ(tokens[2].value, "Rk4xk3");  
  ASSERT_EQ(tokens[3].type, SpellChessLib::PGN4Parser::TokenType::word);  
  ASSERT_EQ(tokens[3].value, "Nj9-h10");  
  ASSERT_EQ(tokens[4].type, SpellChessLib::PGN4Parser::TokenType::bracket_close);  
};

TEST(PGN4Tokenizer, number) {
    auto tokens = SpellChessLib::PGN4Parser::tokenize("2137");   
    ASSERT_EQ(tokens.size(), 1);
    ASSERT_EQ(tokens[0].type, SpellChessLib::PGN4Parser::TokenType::number);
    ASSERT_EQ(tokens[0].value, "2137");
};

TEST(PGN4Tokenizer, numberLedWord) {
    auto tokens = SpellChessLib::PGN4Parser::tokenize("2137a");   
    ASSERT_EQ(tokens.size(), 1);
    ASSERT_EQ(tokens[0].type, SpellChessLib::PGN4Parser::TokenType::word);
    ASSERT_EQ(tokens[0].value, "2137a");
};

TEST(PGN4Tokenizer, dotTerminatedNumber) {
    auto tokens = SpellChessLib::PGN4Parser::tokenize("2137.");   
    ASSERT_EQ(tokens.size(), 2);
    ASSERT_EQ(tokens[0].type, SpellChessLib::PGN4Parser::TokenType::number);
    ASSERT_EQ(tokens[0].value, "2137");
    ASSERT_EQ(tokens[1].type, SpellChessLib::PGN4Parser::TokenType::dot);
};

TEST(PGN4Tokenizer, sampleMoveText) {
    auto tokens = SpellChessLib::PGN4Parser::tokenize(
        "1. Bi11-h10\n2. Bi4-f7 .. i10-i9"
    );
    
    ASSERT_EQ(tokens.size(), 9);
    ASSERT_EQ(tokens[0].type, SpellChessLib::PGN4Parser::TokenType::number);
    ASSERT_EQ(tokens[0].value, "1");
    ASSERT_EQ(tokens[1].type, SpellChessLib::PGN4Parser::TokenType::dot);
    ASSERT_EQ(tokens[1].value, ".");
    ASSERT_EQ(tokens[2].type, SpellChessLib::PGN4Parser::TokenType::word);
    ASSERT_EQ(tokens[2].value, "Bi11-h10");
    ASSERT_EQ(tokens[3].type, SpellChessLib::PGN4Parser::TokenType::number);
    ASSERT_EQ(tokens[3].value, "2");
    ASSERT_EQ(tokens[4].type, SpellChessLib::PGN4Parser::TokenType::dot);
    ASSERT_EQ(tokens[4].value, ".");
    ASSERT_EQ(tokens[5].type, SpellChessLib::PGN4Parser::TokenType::word);
    ASSERT_EQ(tokens[5].value, "Bi4-f7");
    ASSERT_EQ(tokens[6].type, SpellChessLib::PGN4Parser::TokenType::dot);
    ASSERT_EQ(tokens[6].value, ".");
    ASSERT_EQ(tokens[7].type, SpellChessLib::PGN4Parser::TokenType::dot);
    ASSERT_EQ(tokens[7].value, ".");
    ASSERT_EQ(tokens[8].type, SpellChessLib::PGN4Parser::TokenType::word);
    ASSERT_EQ(tokens[8].value, "i10-i9");
};

TEST(PGN4Tokenizer, unterminated_string_literal) {
    ASSERT_THROW(
        (void)SpellChessLib::PGN4Parser::tokenize("\"string"),
        SpellChessLib::InvalidPGN4
    );  
};

TEST(PGN4Tokenizer, unterminated_comment) {
    ASSERT_THROW(
        (void)SpellChessLib::PGN4Parser::tokenize("{comment"),
        SpellChessLib::InvalidPGN4
    );  
};

TEST(PGN4TagsParser, base_case) {
    auto tokens = SpellChessLib::PGN4Parser::tokenize(
        "[RuleVariants \"EnPassant Play4Mate Spell=1+3\"]"
        "[Event \"?\"]"
        "[Site \"?\"]"
        "[Date \"2026.07.17\"]"
        "[Round \"?\"]"
        "[White \"Player 1\"]"
        "[Black \"Player 2\"]"
        "[Result \"1-0\"]"
        );
    
    auto tags = SpellChessLib::PGN4Parser::parseTags(tokens);
    
    ASSERT_EQ(tags["Event"], "?");
    ASSERT_EQ(tags["Site"], "?");
    ASSERT_EQ(tags["Round"], "?");
    ASSERT_EQ(tags["Date"], "2026.07.17");
    ASSERT_EQ(tags["White"], "Player 1");
    ASSERT_EQ(tags["Black"], "Player 2");
    ASSERT_EQ(tags["Result"], "1-0");
};

TEST(PGN4TagsParser, unclosed_tag) {
    auto tokens = SpellChessLib::PGN4Parser::tokenize("[SomeTag \"Some Value\" . {Continuation of a pgn here ...}");
    ASSERT_THROW(
        (void)SpellChessLib::PGN4Parser::parseTags(tokens),
        SpellChessLib::InvalidPGN4
    );  
};

TEST(PGN4TagsParser, missing_tag_value) {
    auto tokens = SpellChessLib::PGN4Parser::tokenize("[SomeTag]");
    ASSERT_THROW(
        (void)SpellChessLib::PGN4Parser::parseTags(tokens),
        SpellChessLib::InvalidPGN4
    );  
};

TEST(PGN4TagsParser, tag_value_is_not_a_literal) {
    auto tokens = SpellChessLib::PGN4Parser::tokenize("[SomeTag random-unquoted-string]");
    ASSERT_THROW(
        (void)SpellChessLib::PGN4Parser::parseTags(tokens),
        SpellChessLib::InvalidPGN4
    );  
};

TEST(PGN4MoveParser, b1c3) {
    SpellChessLib::FenParser fen_parser;
    auto context = fen_parser.fromSpellChessFen(SpellChessLib::SPELL_START_POS);
    auto actual = SpellChessLib::PGN4Parser::parseMove("Ne4-f6", context);
    auto expected = SpellChessLib::parseMove("b1c3");
    ASSERT_EQ(actual, expected);
}

TEST(PGN4MoveParser, e2e4) {
    SpellChessLib::FenParser fen_parser;
    auto context = fen_parser.fromSpellChessFen(SpellChessLib::SPELL_START_POS);
    auto actual = SpellChessLib::PGN4Parser::parseMove("h5-h7", context);
    auto expected = SpellChessLib::parseMove("e2e4");
    ASSERT_EQ(actual, expected);
}

TEST(PGN4MoveParser, h2h4) {
    SpellChessLib::FenParser fen_parser;
    auto context = fen_parser.fromSpellChessFen(SpellChessLib::SPELL_START_POS);
    auto actual = SpellChessLib::PGN4Parser::parseMove("k5-k7", context);
    auto expected = SpellChessLib::parseMove("h2h4");
    ASSERT_EQ(actual, expected);
}

TEST(PGN4MoveParser, jumph2_h1h7) {
    SpellChessLib::FenParser fen_parser;
    auto context = fen_parser.fromSpellChessFen(SpellChessLib::SPELL_START_POS);
    auto actual = SpellChessLib::PGN4Parser::parseMove("jump@k5&k4xk10", context);
    auto expected = SpellChessLib::parseMove("jump@h2&h1h7");
    ASSERT_EQ(actual, expected);
}

TEST(PGN4MoveParser, freeze_c7_e2e4) {
    SpellChessLib::FenParser fen_parser;
    auto context = fen_parser.fromSpellChessFen(SpellChessLib::SPELL_START_POS);
    auto actual = SpellChessLib::PGN4Parser::parseMove("freeze@f10&h5-h7", context);
    auto expected = SpellChessLib::parseMove("freeze@c7&e2e4");
    ASSERT_EQ(actual, expected);
}


TEST(PGN4MoveParser, kingside_castle_white) {
    SpellChessLib::FenParser fen_parser;
    auto context = fen_parser.fromSpellChessFen("r1bqkb1r/pppp1ppp/2n2n2/4p3/2B1P3/5N2/PPPP1PPP/RNBQK2R w KQkq - 4 4 - - F50/J20/f50/j20");
    auto actual = SpellChessLib::PGN4Parser::parseMove("O-O", context);
    auto expected = SpellChessLib::parseMove("e1g1");
    ASSERT_EQ(actual, expected);
}

TEST(PGN4MoveParser, kingside_castle_black) {
    SpellChessLib::FenParser fen_parser;
    auto context = fen_parser.fromSpellChessFen("r1bqk2r/pppp1ppp/2n2n2/2b1p3/2BPP3/5N2/PPP2PPP/RNBQ1RK1 b kq d3 0 5 - - F50/J20/f50/j20");
    auto actual = SpellChessLib::PGN4Parser::parseMove("O-O", context);
    auto expected = SpellChessLib::parseMove("e8g8");
    ASSERT_EQ(actual, expected);
}

TEST(PGN4MoveParser, queenside_castle_white) {
    SpellChessLib::FenParser fen_parser;
    auto context = fen_parser.fromSpellChessFen("2kr1b1r/ppp1qppp/2npbn2/4p3/2B1P3/2NPBN2/PPPQ1PPP/R3K2R w KQ - 4 8 - - F50/J20/f50/j20");
    auto actual = SpellChessLib::PGN4Parser::parseMove("O-O-O", context);
    auto expected = SpellChessLib::parseMove("e1c1");
    ASSERT_EQ(actual, expected);
}

TEST(PGN4MoveParser, queenside_castle_black) {
    SpellChessLib::FenParser fen_parser;
    auto context = fen_parser.fromSpellChessFen("r3kb1r/ppp1qppp/2npbn2/4p3/2B1P3/2NPBN2/PPPQ1PPP/R3K2R b KQkq - 3 7 - - F50/J20/f50/j20");
    auto actual = SpellChessLib::PGN4Parser::parseMove("O-O-O", context);
    auto expected = SpellChessLib::parseMove("e8c8");
    ASSERT_EQ(actual, expected);
}

TEST(PGN4MoveParser, move_with_check) {
    SpellChessLib::FenParser fen_parser;
    auto context = fen_parser.fromSpellChessFen("2kr1b1r/ppp1qppp/2npbn2/4p3/2B1P3/2NPBN2/PPPQ1PPP/R3K2R w KQ - 4 8 - - F50/J20/f50/j20");
    auto actual = SpellChessLib::PGN4Parser::parseMove("Bf7xh9+", context);
    auto expected = SpellChessLib::parseMove("c4e6");
    ASSERT_EQ(actual, expected);
}

TEST(PGN4MoveParser, move_with_checkmate) {
    SpellChessLib::FenParser fen_parser;
    auto context = fen_parser.fromSpellChessFen("2k2b1r/ppprqpp1/2npBn2/3Np2p/4P3/3PBN2/PPPQ1PPP/R3K2R w KQ - 2 10 - - F41/J20/f41/j20");
    auto actual = SpellChessLib::PGN4Parser::parseMove("jump@g10&h9xf11", context);
    auto expected = SpellChessLib::parseMove("jump@d7&e6c8");
    ASSERT_EQ(actual, expected);
}

TEST(PGN4MoveParser, move_with_promotion) {
    SpellChessLib::FenParser fen_parser;
    auto context = fen_parser.fromSpellChessFen("rnbqkb1r/pP1npppp/8/8/8/8/PPPP1PPP/RNBQKBNR w KQkq - 1 5 - - F50/J20/f50/j20");
    auto actual = SpellChessLib::PGN4Parser::parseMove("e10xd11=Q", context);
    auto expected = SpellChessLib::parseMove("b7a8q");
    ASSERT_EQ(actual, expected);
}

TEST(PGN4MoveParser, move_with_promotion_n) {
    SpellChessLib::FenParser fen_parser;
    auto context = fen_parser.fromSpellChessFen("rnbqkb1r/pP1npppp/8/8/8/8/PPPP1PPP/RNBQKBNR w KQkq - 1 5 - - F50/J20/f50/j20");
    auto actual = SpellChessLib::PGN4Parser::parseMove("e10xd11=N", context);
    auto expected = SpellChessLib::parseMove("b7a8n");
    ASSERT_EQ(actual, expected);
}

TEST(PGN4MoveParser, move_with_promotion_r) {
    SpellChessLib::FenParser fen_parser;
    auto context = fen_parser.fromSpellChessFen("rnbqkb1r/pP1npppp/8/8/8/8/PPPP1PPP/RNBQKBNR w KQkq - 1 5 - - F50/J20/f50/j20");
    auto actual = SpellChessLib::PGN4Parser::parseMove("e10xd11=R", context);
    auto expected = SpellChessLib::parseMove("b7a8r");
    ASSERT_EQ(actual, expected);
}

TEST(PGN4MoveParser, move_with_promotion_b) {
    SpellChessLib::FenParser fen_parser;
    auto context = fen_parser.fromSpellChessFen("rnbqkb1r/pP1npppp/8/8/8/8/PPPP1PPP/RNBQKBNR w KQkq - 1 5 - - F50/J20/f50/j20");
    auto actual = SpellChessLib::PGN4Parser::parseMove("e10xd11=B", context);
    auto expected = SpellChessLib::parseMove("b7a8b");
    ASSERT_EQ(actual, expected);
}

TEST(PGN4MoveParser, malformed_move) {
    SpellChessLib::FenParser fen_parser;
    auto context = fen_parser.fromSpellChessFen(SpellChessLib::SPELL_START_POS);
    ASSERT_THROW(
        (void)SpellChessLib::PGN4Parser::parseMove("malformed-move", context),
        SpellChessLib::InvalidPGN4
    );
}

TEST(StringTokenizer, split_string1) {
    const std::string input = "Mary.has.a.little.lamb";
    auto parts = SpellChessLib::Parsing::splitString(input, '.');
    ASSERT_EQ(parts.size(), 5);
    ASSERT_EQ(parts[0], "Mary");
    ASSERT_EQ(parts[1], "has");
    ASSERT_EQ(parts[2], "a");
    ASSERT_EQ(parts[3], "little");
    ASSERT_EQ(parts[4], "lamb");
};

TEST(StringTokenizer, split_string2) {
    const std::string input = "Mary.has.a..lamb";
    auto parts = SpellChessLib::Parsing::splitString(input, '.');
    ASSERT_EQ(parts.size(), 5);
    ASSERT_EQ(parts[0], "Mary");
    ASSERT_EQ(parts[1], "has");
    ASSERT_EQ(parts[2], "a");
    ASSERT_EQ(parts[3], "");
    ASSERT_EQ(parts[4], "lamb");
};

TEST(StringTokenizer, split_string3) {
    const std::string input = "Mary has a little lamb";
    auto parts = SpellChessLib::Parsing::splitString(input, '.');
    ASSERT_EQ(parts.size(), 1);
    ASSERT_EQ(parts[0], "Mary has a little lamb");
};
