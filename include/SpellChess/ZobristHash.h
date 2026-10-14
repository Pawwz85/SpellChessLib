#pragma once

#include <cstdint>
#include <array>
#include <cstddef>

namespace SpellChessLib {

    class GameState;

    namespace Internal {

        constexpr uint64_t LCG(uint64_t previous) {
            const uint64_t multiplier = 2862933555777941757ull;
            const uint64_t increment =  3037000493ull;
            return multiplier * previous + increment;
        };
       
        constexpr uint32_t PCG_OUTPUT(uint64_t state){
            uint32_t x = (state ^ (state >> 18)) >> 27;
            uint32_t shift = state >> 59;
            uint32_t mask = ~(1u<<shift);
            return (x >> shift) | ((x & mask) << shift);
        };

        template <size_t S>
        constexpr std::array<uint64_t, S> generate_random_table() {
            std::array<uint64_t, S> result;

            uint64_t seed = 0xDEADBEEFull;

            for(size_t i = 0; i < S; ++i) {
                result[i] = PCG_OUTPUT(seed);
                seed = LCG(seed);
                result[i] <<= 32;
                result[i] |= PCG_OUTPUT(seed);
                seed = LCG(seed);
            };

            return result;
        };

        constexpr size_t zobrish_key_count() {
            return  64 * 12 // Piece on the square
                    + 64    // enpassant sq
                    + 64    // freeze sq
                    + 64    // jump sq
                    + 4     // castling rights
                    + 1     // side to move
                    + 2 * 6 // freezes usages left
                    + 2 * 3 // jumps usages left
                    + 4 * 3;// spell cooldown
        };

        namespace ZobristOffsets {
            const size_t pieceSectionStart          = 0;
            const size_t enpassantSqSectionStart    = 64 * 12;
            const size_t freezeSqSectionStart       = enpassantSqSectionStart + 64;
            const size_t jumpSqSectionStart         = freezeSqSectionStart + 64;  
            const size_t castlingSectionOffset      = jumpSqSectionStart + 64;
            const size_t sideToMoveIndex            = castlingSectionOffset + 4;
            const size_t freezeUsageSectionOffset   = sideToMoveIndex + 1;
            const size_t jumpUsageSectionOffset     = freezeUsageSectionOffset + 2 * 6;
            const size_t spellCooldownSectionOffset = jumpUsageSectionOffset   + 2 * 3;
        };

    }; 

    extern std::array<uint64_t, Internal::zobrish_key_count()> zobrishKeys;
    extern uint64_t calculateHash(const SpellChessLib::GameState & state);

};