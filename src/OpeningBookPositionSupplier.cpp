#include <SpellChess/OpeningBookPositionSupplier.h>
#include <SpellChess/MoveGen.h>

SpellChessLib::GameState SpellChessLib::OpeningBookPositionSupplier::produce() {
    GameState result = startPos;
    while (true)
    {
        auto move = book->pickRandomUniformMove(result);
        if(!move.has_value())
            return result;
        
        SpellChessMove m = move.value();
        MoveGenerator gen(result);

        if(gen.isMoveLegal(m)) 
            result.makeMove(m);
        else
            return result;
    }
};
