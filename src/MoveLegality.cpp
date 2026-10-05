#include <SpellChess/MoveLegality.h>
#include <SpellChess/StringTokenizer.h> // for parseSquareIndex
#include <assert.h>

using namespace SpellChessLib;

SquareIndex SpellChessLib::findKingPos(const GameState& state, const PlayerColor& side)
{
    for (size_t i = 0; i < 64; ++i)
        if (state.getSquare(i).piece.type == King && state.getSquare(i).piece.owner == side)
            return i;
    return NoSquare;
}

int SpellChessLib::isMoveCastling(const GameState& state, const SquareIndex& from, const SquareIndex& to)
{
    /*
        Preconditions:
        - from square is occupied by a king
    */
    
    const SquareIndex e8 = Parsing::parseSquareIndex("e8");
    const SquareIndex e1 = Parsing::parseSquareIndex("e1");
    
    SquareIndex kingPos = state.getSideToMove() == White ? e1 : e8;

    if (kingPos != from) return -1;

    if (from + 2 == to) return Kingside;
    if (from - 2 == to) return QueenSide;

    return -1;
}

bool SpellChessLib::arePointsColinear(const int8_t& x1, const int8_t& y1, const int8_t& x2, const int8_t& y2)
{
    return (x1 == x2) || (y1 == y2) || (x1 + y1 == x2 + y2) || (x1 - y1 == x2 - y2);
}

int SpellChessLib::_sliderStep(const int8_t& u, const int8_t& w)
{
    if(u == w) 
    return 0;
    return (u > w) ? -1 : 1;
}

#define _DECL_SQS(a, b, c, d, e) DECL_SQ(a) DECL_SQ(b) DECL_SQ(c) DECL_SQ(d) DECL_SQ(e)
#define _CAN_KING_PASS_THROUGHT_THIS_SQ(sq) state.getSquare(sq).isEmpty() && getAttackers(state, sq, flipColor(side)) == 0
#define _KING_PATH_SAFE_CHECK(sq1, sq2) _CAN_KING_PASS_THROUGHT_THIS_SQ(sq1) &&  _CAN_KING_PASS_THROUGHT_THIS_SQ(sq2)
bool SpellChessLib::isCastlingLegal(const GameState& state, const CastlingDirection& dir)
{
    DECL_SQ(e1)
    DECL_SQ(e8)
    _DECL_SQS(b1, c1, d1, f1, g1)
    _DECL_SQS(b8, c8, d8, f8, g8)

    auto side = state.getSideToMove();

    // can not castle in check
    if(getAttackers(state, (side == White ? e1 : e8), flipColor(side)) != 0)
        return false;

    if (side == White && dir == Kingside) {
        return _KING_PATH_SAFE_CHECK(f1, g1);
    }

    if (side == Black && dir == Kingside) {
        return _KING_PATH_SAFE_CHECK(f8, g8);
    }

    if (side == White && dir == QueenSide) {
        return state.getSquare(b1).isEmpty() && _KING_PATH_SAFE_CHECK(c1, d1);
    }

    if (side == Black && dir == QueenSide) {
        return state.getSquare(b8).isEmpty() && _KING_PATH_SAFE_CHECK(c8, d8);
    }

    return false;
}

#undef _CAN_KING_PASS_THROUGHT_THIS_SQ
#undef _KING_PATH_SAFE_CHECK
#undef _DECL_SQS

bool SpellChessLib::isSquareInFrozenArea(const SquareIndex& sq, const SquareIndex& freezeCenter)
{

    unsigned int d1 = std::abs(sq % 8 - freezeCenter % 8);
    unsigned int d2 = std::abs(sq / 8 - freezeCenter / 8);
    return std::max(d1, d2) <= 1;
}

uint64_t SpellChessLib::calculateFrozenArea(const SquareIndex& freezeCenter)
{
    if (freezeCenter == NoSquare)
        return 0;
    int x = freezeCenter % 8, y = freezeCenter / 8;
    uint64_t result = 0;
    for (int dx = -1; dx <= 1; ++dx)
        for (int dy = -1; dy <= 1; ++dy)
            if (CORDINATE_BOUNDARY_CHECK(x + dx) && CORDINATE_BOUNDARY_CHECK(y + dy))
                result |= 1ull << squareIndexFromCords(x + dx, y + dy);
    return result;
}

uint64_t SpellChessLib::getAttackers(const GameState& state, const SquareIndex& sq, const PlayerColor& them)
{
    return  _Attackers::getKnightAttackers(state, sq, them) |
            _Attackers::getPawnAttackers(state, sq, them) |
            _Attackers::getKingAttackers(state, sq, them) |
            _Attackers::getSliderAttackers(state, sq, them);
}

bool SpellChessLib::isPawnMovePseudoLegal(const GameState& state, const PlayerColor& us, const SquareIndex& from, const SquareIndex& to, const SquareIndex & jumpSq)
{

    PlayerColor them = us == White ? Black : White;

    int8_t step = us == White ? -8 : 8;

    // pawn is moved 1 square
    if (to == from + step) {
        return state.getSquare(to).isEmpty();
    }

    // pawn is pushed 2 squares
    if (to == from + 2 * step) {
        return (state.getSquare(from + step).isEmpty() || jumpSq == from + step || state.getSquare(from + step).jumpable) && state.getSquare(to).isEmpty();
    }

    // enpassant or capture is required
    if (to != state.getEnppassantSquare() && state.getSquare(to).piece.owner != them) return false;

    int8_t x1 = from % 8, x2 = to % 8;

    return std::abs(x1 - x2) == 1 && to == (from + step + x2 - x1);
}

bool SpellChessLib::isKnightMovePseudoLegal(const GameState & state, const PlayerColor& us, const SquareIndex& from, const SquareIndex& to)
{
    PlayerColor them = us == White ? Black : White;

    int8_t x1, y1, x2, y2;
    x1 = from % 8, x2 = to % 8, y1 = from / 8, y2 = to / 8;
    
    int dis1 = std::abs(x1 - x2), dis2 = std::abs(y1 - y2);

    if((dis1 != 1 || dis2 != 2) && (dis1 != 2 || dis2 != 1))
        return false;

    return state.getSquare(to).piece.owner != us;

}

bool SpellChessLib::isKingMovePseudoLegal(const GameState& state, const PlayerColor& us, const SquareIndex& from, const SquareIndex& to)
{
    int castlingStatus = isMoveCastling(state,  from, to);

    if (castlingStatus != -1) {
        // castling is pseudo

        if(!state.canCastle(us, CastlingDirection(castlingStatus)))
            return false; 

        return true;
    }

    int8_t x1, y1, x2, y2;
    x1 = from % 8, x2 = to % 8, y1 = from / 8, y2 = to / 8;
    int dis1 = std::abs(x1 - x2), dis2 = std::abs(y1 - y2);
    if(dis1 > 1 || dis2 > 1 || from == to)
        return false;

    return state.getSquare(to).piece.owner != us;
}

bool SpellChessLib::isSliderMovePseudoLegal(const GameState& state, const PlayerColor& us, const SquareIndex& from, const SquareIndex& to, const SquareIndex & jumpSq)
{
    int8_t x1, y1, x2, y2;
    x1 = from % 8, x2 = to % 8, y1 = from / 8, y2 = to / 8;
    if(!arePointsColinear(x1, y1, x2, y2))
        return false;

    if (state.getSquare(to).piece.owner == us) return false;

    PieceType type = state.getSquare(from).piece.type;
    int8_t stepx = _sliderStep(x1, x2), stepy = _sliderStep(y1, y2), x = x1, y = y1;
    
    if (type == Rook && stepx && stepy)
        return false;
    
    if (type == Bishop && (stepx == 0 || stepy == 0))
        return false;


    x += stepx;
    y += stepy;
    SquareIndex index;
    while (x != x2 && y != y2) {
        index = 8 * y + x;
        if (!state.getSquare(index).isEmpty()) {
            if (jumpSq != index && !state.getSquare(index).jumpable)
                return false;
        }

        x += stepx;
        y += stepy;
    }

    return true;
}

#define _OFFSET_BOUNDARY_CHECK(dx, dy) CORDINATE_BOUNDARY_CHECK((x + dx)) && CORDINATE_BOUNDARY_CHECK((y + dy))
#define _OFFSET(dx, dy) (8 * dy + dx)

#define _ATTACKER_ON(dx, dy) currSq = int(sq) + _OFFSET(dx, dy); \
     if (_OFFSET_BOUNDARY_CHECK(dx, dy) && state.getSquare(currSq).piece == enemy)\
        result |= 1ull << currSq;

uint64_t SpellChessLib::_Attackers::getKnightAttackers(const GameState& state, const SquareIndex& sq, const PlayerColor& them)
{
    uint64_t result = 0;
    int x = sq % 8, y = sq / 8;

    SquareIndex currSq = NoSquare;
    Piece enemy = { Knight, them };

    _ATTACKER_ON(-2, -1)
    _ATTACKER_ON(-2, 1)
    _ATTACKER_ON(-1, -2)
    _ATTACKER_ON(-1, 2)
    _ATTACKER_ON(2, -1)
    _ATTACKER_ON(2, 1)
    _ATTACKER_ON(1, -2)
    _ATTACKER_ON(1, 2)
    return result;
}

uint64_t SpellChessLib::_Attackers::getKingAttackers(const GameState& state, const SquareIndex& sq, const PlayerColor& them) {
    uint64_t result = 0;
    int x = sq % 8, y = sq / 8;

    SquareIndex currSq = NoSquare;
    Piece enemy = { King, them };
    _ATTACKER_ON(-1, -1)
    _ATTACKER_ON(-1, 0)
    _ATTACKER_ON(-1, 1)
    _ATTACKER_ON(0, -1)
    _ATTACKER_ON(0, 1)
    _ATTACKER_ON(1, -1)
    _ATTACKER_ON(1, 0)
    _ATTACKER_ON(1, 1)
    return result;
}

SquareIndex SpellChessLib::_Attackers::castRay(const GameState& state, const SquareIndex& sq, int dx, int dy)
{
    int x = sq % 8, y = sq / 8;
    int i = 1;
    
    SquareIndex currSq;

    while (_OFFSET_BOUNDARY_CHECK(i * dx, i * dy)) {
        currSq = sq + _OFFSET(i*dx, i*dy);
        if (!state.getSquare(currSq).isEmpty())
            return currSq;
        ++i;
    }

    return NoSquare;
}

uint64_t SpellChessLib::_Attackers::getSliderAttackers(const GameState& state, const SquareIndex& sq, const PlayerColor& them)
{
    uint64_t result = 0;
    PieceType attacker = None, validAttacker = None;
    
    int delta_xs[8] = { -1, -1,  1, 1, -1,  0, 0, 1};
    int delta_ys[8] = { -1,  1, -1, 1,  0, -1, 1, 0};
    SquareIndex attackerLocation;
    Piece piece;
    bool kingEncountered;

    for (size_t i = 0; i < 8; ++i) {
        validAttacker = i < 4 ? Bishop : Rook;
        kingEncountered = false;
        attackerLocation = sq;
        
        do { // this loop recasts ray if non attacking king is in the way 
            attackerLocation = castRay(state, attackerLocation, delta_xs[i], delta_ys[i]);
            if (attackerLocation == NoSquare)
                goto NEXT_ITER;
               
            piece = state.getSquare(attackerLocation).piece;
            kingEncountered = piece.type == King && piece.owner == flipColor(them);
        
            if (piece.owner != them && !kingEncountered)
                 goto NEXT_ITER;

        } while (kingEncountered);


        attacker = piece.type;

        if (attacker == Queen || attacker == validAttacker)
            result |= 1ull << attackerLocation;

    NEXT_ITER:
        continue;
    }

    return result;
}

uint64_t SpellChessLib::_Attackers::getPawnAttackers(const GameState& state, const SquareIndex& sq, const PlayerColor& them) {
    uint64_t result = 0;
    int x = sq % 8, y = sq / 8;
    SquareIndex currSq = NoSquare;
    Piece enemy = { Pawn, them };
    int dy = them == White ? 1 : -1;

    _ATTACKER_ON(1, dy);
    _ATTACKER_ON(-1, dy);
    return result;
}