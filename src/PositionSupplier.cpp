#include <SpellChess/PositionSupplier.h>
#include <SpellChess/MoveGen.h>

SpellChessLib::GameState SpellChessLib::RandomPositionSupplier::recursiveRandomization(GameState in, int level)
{
    if (level == 0)
        return in;

    MoveGenerator gen1(in);
    auto moves = gen1.generateMoves();
    size_t index;

    GameState out;
    bool isInvalid;
    do {
        do {
            index = rand() % moves.size();
        } while (moves[index].spellPolicy != UserCanAddSpell);

        isInvalid = false;
        out = in;
        out.makeMove(moves[index].move);
        out = recursiveRandomization(out, level - 1);
        
        MoveGenerator gen2(out);
        if (gen2.generateMoves().empty())
            isInvalid = true;

    } while (isInvalid);

    return out;
}