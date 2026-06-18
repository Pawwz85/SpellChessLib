#include <SpellChess/ZobristHash.h>

std::array<uint64_t, SpellChessLib::Internal::zobrish_key_count()> SpellChessLib::zobrishKeys = 
    SpellChessLib::Internal::generate_random_table< SpellChessLib::Internal::zobrish_key_count()>();


uint64_t SpellChessLib::calculateHash(const SpellChessLib::GameState &state)
{
    uint64_t result = 0ull;

    for(size_t sq = 0; sq < 64; ++sq) 
        if (!state.getSquare(sq).isEmpty()) {

        size_t index = Internal::ZobristOffsets::pieceSectionStart + 6 * sq;
        if (state.getSquare(sq).piece.owner == PlayerColor::Black)
            index += 64 * 6;
        
        index += size_t(state.getSquare(sq).piece.type) - 1;
        result ^= zobrishKeys[index];
    };

    if (state.getSideToMove() == PlayerColor::Black)
        result ^= zobrishKeys[Internal::ZobristOffsets::sideToMoveIndex];

    if (state.canCastle(SpellChessLib::White, SpellChessLib::Kingside))
        result ^= zobrishKeys[Internal::ZobristOffsets::castlingSectionOffset];
    
    if (state.canCastle(SpellChessLib::White, SpellChessLib::QueenSide))
        result ^= zobrishKeys[Internal::ZobristOffsets::castlingSectionOffset + 1];

    if (state.canCastle(SpellChessLib::Black, SpellChessLib::Kingside))
        result ^= zobrishKeys[Internal::ZobristOffsets::castlingSectionOffset + 2];
    
    if (state.canCastle(SpellChessLib::Black, SpellChessLib::QueenSide))
        result ^= zobrishKeys[Internal::ZobristOffsets::castlingSectionOffset + 3];   

    if (state.getEnppassantSquare() != NoSquare)
        result ^= zobrishKeys[Internal::ZobristOffsets::enpassantSqSectionStart + state.getEnppassantSquare()];

    if (state.getFreezeSquare() != NoSquare)
        result ^= zobrishKeys[Internal::ZobristOffsets::freezeSqSectionStart + state.getFreezeSquare()];

    if (state.getJumpSquare() != NoSquare)
        result ^= zobrishKeys[Internal::ZobristOffsets::jumpSqSectionStart + state.getJumpSquare()];

    result ^= zobrishKeys[Internal::ZobristOffsets::freezeUsageSectionOffset + 0 + state.getSpell(White, FreezeSpell).usagesLeft];
    result ^= zobrishKeys[Internal::ZobristOffsets::freezeUsageSectionOffset + 6 + state.getSpell(Black, FreezeSpell).usagesLeft];
    result ^= zobrishKeys[Internal::ZobristOffsets::jumpUsageSectionOffset   + 0 + state.getSpell(White, JumpSpell).usagesLeft];    
    result ^= zobrishKeys[Internal::ZobristOffsets::jumpUsageSectionOffset   + 3 + state.getSpell(Black, JumpSpell).usagesLeft];
    result ^= zobrishKeys[Internal::ZobristOffsets::spellCooldownSectionOffset + 0 + state.getSpell(White, FreezeSpell).cooldown];
    result ^= zobrishKeys[Internal::ZobristOffsets::spellCooldownSectionOffset + 3 + state.getSpell(White, JumpSpell).cooldown];
    result ^= zobrishKeys[Internal::ZobristOffsets::spellCooldownSectionOffset + 6 + state.getSpell(Black, FreezeSpell).cooldown];
    result ^= zobrishKeys[Internal::ZobristOffsets::spellCooldownSectionOffset + 9 + state.getSpell(Black, JumpSpell).cooldown];
    return result;
}  