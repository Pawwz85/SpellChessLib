#include <SpellChess/Arbiter.h>
#include <SpellChess/MoveGen.h>
#include <SpellChess/ZobristHash.h>
#include <SpellChess/Board.h>
#include <algorithm>
#include <cstdint>

SpellChessLib::ArbiterVerdict
SpellChessLib::arbite(const GameState & pos, const GameHistory & history) {

    if (pos.getHalfMoveCounter() >= 100)
        return ArbiterVerdict::DrawByFiftyMoveRule;

    if (pos.countPiece(King, White) == 0 || pos.countPiece(King, Black) == 0)
        return ArbiterVerdict::KingCapture;

    MoveGenerator gen(pos);

    if (gen.generateMoves().empty()) {
        return gen.isInCheck() ?
            ArbiterVerdict::Checkmate :
            ArbiterVerdict::Stalemate;
    }

    if (!pos.hasSufficientMatingMaterial(White) && !pos.hasSufficientMatingMaterial(Black))
        return ArbiterVerdict::DrawByInsufficientMaterial;

    if (history.countOccurences(pos) >= 3)
        return ArbiterVerdict::ThreefoldRepetition;

    return ArbiterVerdict::None;
};

class GameReplay : public SpellChessLib::GameHistory {
    std::vector<uint64_t> positions;
    SpellChessLib::GameState state;

    public:
    GameReplay(const SpellChessLib::GameState & root, 
        const std::vector<SpellChessLib::SpellChessMove> & movesSinceRoot) : state(root) {
            positions.push_back(SpellChessLib::calculateHash(state));
            for (auto & move : movesSinceRoot){
                state.makeMove(move);
                positions.push_back(SpellChessLib::calculateHash(state));
            }
        };
    
    const SpellChessLib::GameState & getFinalState() const {
        return state;
    };

    size_t countOccurences(const SpellChessLib::GameState & position) const override {
        uint64_t hash = SpellChessLib::calculateHash(position);
        return std::count(positions.begin(), positions.end(), hash);
    }
};

SpellChessLib::ArbiterVerdict
SpellChessLib::arbite( const SpellChessLib::GameState & root, 
        const std::vector<SpellChessLib::SpellChessMove> & movesSinceRoot) {
    GameReplay replay(root, movesSinceRoot);
    return SpellChessLib::arbite(replay.getFinalState(), replay);
};