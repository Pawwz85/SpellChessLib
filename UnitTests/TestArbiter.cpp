#include "pch.h"
#include <SpellChess/Arbiter.h>

class EmptyHistory : public SpellChessLib::GameHistory {
    size_t countOccurences(const SpellChessLib::GameState & position) const override {
        return 0ull;
    };
};

EmptyHistory nullHistory;

TEST(Arbiter, Nothing) {
  SpellChessLib::FenParser parser;  
  SpellChessLib::GameState state = parser.fromSpellChessFen(SpellChessLib::SPELL_START_POS); 
  
  ASSERT_EQ(
    SpellChessLib::arbite(state, nullHistory),
    SpellChessLib::ArbiterVerdict::None
  );
}

TEST(Arbiter, Checkmate) {
  SpellChessLib::FenParser parser;  
  SpellChessLib::GameState state = parser.fromSpellChessFen("rnbqkbnr/ppp1p1pp/8/1B1p1p1Q/4P3/8/PPPP1PPP/RNB1K1NR b KQkq - 1 3 - - F41/J20/f42/j20"); 
  
  ASSERT_EQ(
    SpellChessLib::arbite(state, nullHistory),
    SpellChessLib::ArbiterVerdict::Checkmate
  );
}

TEST(Arbiter, KingCapture) {
  SpellChessLib::FenParser parser;  
  SpellChessLib::GameState state = parser.fromSpellChessFen("rnbqBb1r/pppp1ppp/5n2/4p3/4P3/8/PPPP1PPP/RNBQK1NR b KQkq - 0 3 - d7 F50/J12/f50/j20"); 
  
  ASSERT_EQ(
    SpellChessLib::arbite(state, nullHistory),
    SpellChessLib::ArbiterVerdict::KingCapture
  );
}

TEST(Arbiter, Stalemate) {
  SpellChessLib::FenParser parser;  
  SpellChessLib::GameState state = parser.fromSpellChessFen("3k4/5Q2/4B3/2p1p3/2P1P2P/8/PP1P1PPR/RNB1K1N1 b Q - 0 20 c5 - F42/J20/f42/j20"); 
  
  ASSERT_EQ(
    SpellChessLib::arbite(state, nullHistory),
    SpellChessLib::ArbiterVerdict::Stalemate
  );
}

TEST(Arbiter, fiftyMoveRule) {
  SpellChessLib::FenParser parser;  
  SpellChessLib::GameState state = parser.fromSpellChessFen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 100 1 - - F50/J20/f50/j20"); 
  
  ASSERT_EQ(
    SpellChessLib::arbite(state, nullHistory),
    SpellChessLib::ArbiterVerdict::DrawByFiftyMoveRule
  );
}

TEST(Arbiter, insufficientMaterial) {
  SpellChessLib::FenParser parser;  
  SpellChessLib::GameState state = parser.fromSpellChessFen("4k3/8/8/8/8/8/8/4K3 w - - 0 1 - - F00/J00/f00/j00"); 
  
  ASSERT_EQ(
    SpellChessLib::arbite(state, nullHistory),
    SpellChessLib::ArbiterVerdict::DrawByInsufficientMaterial
  );
}

class ThreeFoldRepHistory : public SpellChessLib::GameHistory {
    size_t countOccurences(const SpellChessLib::GameState & position) const override {
        return 3;
    };
};

TEST(Arbiter, threefoldRepetition) {
  SpellChessLib::FenParser parser;  
  SpellChessLib::GameState state = parser.fromSpellChessFen(SpellChessLib::SPELL_START_POS); 
  ThreeFoldRepHistory repHis;
  
  ASSERT_EQ(
    SpellChessLib::arbite(state, repHis),
    SpellChessLib::ArbiterVerdict::ThreefoldRepetition
  );
}