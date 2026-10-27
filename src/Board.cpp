#include <SpellChess/Board.h>
#include <SpellChess/StringTokenizer.h>
#include <sstream>
#include <cctype> // for isspace
#include <cmath>
#include <unordered_map>

using namespace SpellChessLib;

PlayerColor SpellChessLib::flipColor(const PlayerColor& c)
{
    static PlayerColor flippedValue[3] = { Nobody, PlayerColor(2), PlayerColor(1)};
    return flippedValue[c];
}

const char* SpellChessLib::IllegalMoveException::what() const noexcept
{
    return msg.c_str();
}

void SpellChessLib::GameState::revokeCastlingRightsAssociatedWithGivenRook(const SquareIndex& index)
{
    const auto a1 = Parsing::parseSquareIndex("a1");
    const auto a8 = Parsing::parseSquareIndex("a8");
    const auto h1 = Parsing::parseSquareIndex("h1");
    const auto h8 = Parsing::parseSquareIndex("h8");

    if (index == a8)
        getCastlingRight(Black, QueenSide) = false;

    if (index == h8)
        getCastlingRight(Black, Kingside) = false;

    if (index == a1)
        getCastlingRight(White, QueenSide) = false;

    if (index == h1)
        getCastlingRight(White, Kingside) = false;
    
}

void SpellChessLib::GameState::makeCastling(const PlayerColor& playerColor, const CastlingDirection& direction)
{

    castlingRights[playerColor - 1][0] = castlingRights[playerColor - 1][1] = false;

    if (playerColor == White && direction == Kingside) {
        board[Parsing::parseSquareIndex("e1")].piece = NO_PIECE;
        board[Parsing::parseSquareIndex("f1")].piece = { Rook, White };
        board[Parsing::parseSquareIndex("g1")].piece = { King, White };
        board[Parsing::parseSquareIndex("h1")].piece = NO_PIECE;
    }

    if (playerColor == Black && direction == Kingside) {
        board[Parsing::parseSquareIndex("e8")].piece = NO_PIECE;
        board[Parsing::parseSquareIndex("f8")].piece = { Rook, Black };
        board[Parsing::parseSquareIndex("g8")].piece = { King, Black };
        board[Parsing::parseSquareIndex("h8")].piece = NO_PIECE;
    }

    if (playerColor == White && direction == QueenSide) {
        board[Parsing::parseSquareIndex("e1")].piece = NO_PIECE;
        board[Parsing::parseSquareIndex("d1")].piece = { Rook, White };
        board[Parsing::parseSquareIndex("c1")].piece = { King, White };
        board[Parsing::parseSquareIndex("a1")].piece = NO_PIECE;
    }

    if (playerColor == Black && direction == QueenSide) {
        board[Parsing::parseSquareIndex("e8")].piece = NO_PIECE;
        board[Parsing::parseSquareIndex("d8")].piece = { Rook, Black };
        board[Parsing::parseSquareIndex("c8")].piece = { King, Black };
        board[Parsing::parseSquareIndex("a8")].piece = NO_PIECE;
    }
}

inline char to_symbol(const SpellChessLib::Piece & piece) {
    static char bl_piece_symbols[7] = { ' ', 'p', 'n', 'b', 'r', 'q', 'k' };
    static char wh_piece_symbols[7] = { ' ', 'P', 'N', 'B', 'R', 'Q', 'K' };
    auto & symbol_table = (piece.owner == White ? wh_piece_symbols : bl_piece_symbols);
    return symbol_table[piece.type];
};

std::string SpellChessLib::GameState::makePieceFenStringFragment() const
{
    class Accumulator {
        uint8_t val = 0;
        std::string* result;
        public:

        Accumulator(std::string & result) : result(&result){}
        ~Accumulator() { flush(); }

        Accumulator& operator++() {
            val++;
            return *this;
        }

        void flush() {
            if(val) 
                *result += std::to_string(val);
            val = 0;
        }
    };

    std::string result;

    for (uint8_t y = 0; y < 8; ++y) {
        Accumulator acc = result;

        if (y) result += "/";

        for (uint8_t x = 0; x < 8; ++x) {
            uint8_t index = 8 * y + x;
            if (!board[index].isEmpty()) {
                acc.flush();
                result += to_symbol(board[index].piece);
            }
            else 
                ++acc;
        }
    }

    return result;
}

std::string SpellChessLib::GameState::makeCastlingRightsFenStringFragment() const
{
    std::string result = "";

    if (canCastle(White, Kingside))
        result += 'K';

    if (canCastle(White, QueenSide))
        result += 'Q';

    if (canCastle(Black, Kingside))
        result += 'k';

    if (canCastle(Black, QueenSide))
        result += 'q';

    return result.empty()? "-" : result;
}

std::string SpellChessLib::GameState::makeSpellInfoFenStringFragment() const
{
    std::string result = "";

    result += "F" + std::to_string((int)spells[White - 1][FreezeSpell].usagesLeft) +
        std::to_string((int)spells[White - 1][FreezeSpell].cooldown);
    result += '/';
    result += "J" + std::to_string((int)spells[White - 1][JumpSpell].usagesLeft)  +
        std::to_string(spells[White - 1][JumpSpell].cooldown);
    result += '/';
    result += "f" + std::to_string((int)spells[Black - 1][FreezeSpell].usagesLeft) +
        std::to_string((int)spells[Black - 1][FreezeSpell].cooldown);
    result += '/';
    result += "j" + std::to_string((int)spells[Black - 1][JumpSpell].usagesLeft)  +
        std::to_string((int)spells[Black - 1][JumpSpell].cooldown);

    return result;
}

std::string SpellChessLib::GameState::makeSquareIndexFenStringFragment(const SquareIndex& index) const
{
    if (index == NoSquare)
        return "-";

    return squareIndexToString(index);
}

std::string SpellChessLib::GameState::makeStandardChessFen() const
{
    // example
    // rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1
    return makePieceFenStringFragment() + ' ' + ((sideToMove == White) ? 'w' : 'b') + ' ' + makeCastlingRightsFenStringFragment() + ' ' + makeSquareIndexFenStringFragment(enpassantSquare)
        + ' ' + std::to_string(halfMoveRuleCounter) + ' ' + std::to_string(fullMoveCounter);
}

std::string SpellChessLib::GameState::makeSpellChessFen() const
{
    return makeStandardChessFen() + ' ' + makeSquareIndexFenStringFragment(frozeSquare) + ' ' + makeSquareIndexFenStringFragment(jumpSquare) + ' ' 
        + makeSpellInfoFenStringFragment();
}

void SpellChessLib::GameState::clearSpells()
{
    for (SquareIndex index = 0; index < 64; ++index) 
        board[index].frozen = board[index].jumpable = false;
    
    jumpSquare = NoSquare;
    frozeSquare = NoSquare;
    spellFields.clear();
}

void SpellChessLib::GameState::loadCastlingRights(bool* array)
{
    castlingRights[White - 1][Kingside] = array[0];
    castlingRights[White - 1][QueenSide] = array[1];
    castlingRights[Black - 1][Kingside] = array[2];
    castlingRights[Black - 1][QueenSide] = array[3];
}

void SpellChessLib::GameState::loadSpellInfos(const SpellCounter* array)
{
    spells[White - 1][FreezeSpell] = array[0];
    spells[White - 1][JumpSpell] = array[1];
    spells[Black - 1][FreezeSpell] = array[2];
    spells[Black - 1][JumpSpell] = array[3];
}

void SpellChessLib::GameState::loadSpellFields(const std::vector<SpellField>& fields)
{
    for (const auto& field : fields) {
        if (field.type == FreezeSpell) 
            frozeArea(field.target);
        else if (field.type == JumpSpell) 
            makeJumpable(field.target);
    }
}

void SpellChessLib::GameState::frozeArea(const SquareIndex& sq)
{
    uint8_t x, y;
    x = sq / 8, y = sq % 8;
    
    for (size_t i = 0; i < 64; ++i) if (std::abs(int(i / 8) - sq / 8) <= 1 && std::abs(int(i % 8) - sq % 8) <= 1)
        board[i].frozen = true;

    frozeSquare = sq;
}

void SpellChessLib::GameState::makeJumpable(const SquareIndex & sq){
    board[sq].jumpable = true;
    jumpSquare = sq;
}


std::string SpellChessLib::squareIndexToString(const SquareIndex& index)
{
    uint8_t x, y;
    x = index & 7;
    y = 7 - (index >> 3);
    
    return std::string() + char('a' + x) + char('1' + y);
}

std::string SpellChessLib::spellChessMoveToString(const SpellChessMove& move)
{
    validateSpellChessMove(move);

    static std::unordered_map<PieceType, char> promotionTargets = {
        {Knight, 'n'},
        {Bishop, 'b'},
        {Rook,   'r'},
        {Queen,  'q'}
    };

    std::string fromSq = squareIndexToString(move.from);
    std::string toSq = squareIndexToString(move.to);

    std::string result = fromSq + toSq;
    
    if (promotionTargets.count(move.promoteTo))
        result += promotionTargets[move.promoteTo];

    if (move.freezeSquare < 64)
        result = "freeze@" + squareIndexToString(move.freezeSquare) + "&" + result;

    if (move.jumpSquare < 64) 
        result = "jump@" + squareIndexToString(move.jumpSquare) + "&" + result;

    return result;
}

SpellChessMove SpellChessLib::parseMove(std::string_view str)
{
    // Grammar {freeze@[sq]&, jump@[sq]&}[sq][sq]
    SpellChessMove result = {NoSquare, NoSquare, NoSquare, NoSquare, None};
    size_t len = str.size();
    size_t spellSepPos = 0;
   
    std::string spell = "";
    std::string spellTarget = "-";
    std::string from;
    std::string to;
    std::string tmp;

    bool specialSymbolEncounter = false;

    char c;
    for (size_t i = 0; i < len; ++i) {
        c = str[i];
        if (c == '@') {
            specialSymbolEncounter = true;
            spell = tmp;
            tmp.clear();
        }
        else if (c == '&') {
            specialSymbolEncounter = true;
            spellTarget = tmp;
            tmp.clear();
        } else
        tmp += str[i];
    }

    if (tmp.size() != 4 && tmp.size() != 5) {
        throw Parsing::ParsingError();
    }
   
    from = tmp.substr(0, 2);
    to = tmp.substr(2, 2);

    result.from = Parsing::parseSquareIndex(from);
    result.to = Parsing::parseSquareIndex(to);

    if (!spell.empty() && spellTarget == "-")
        throw Parsing::ParsingError();

    if (spell == "freeze") {
        result.freezeSquare = Parsing::parseSquareIndex(spellTarget);
    }
    else if (spell == "jump" ) {
        result.jumpSquare = Parsing::parseSquareIndex(spellTarget);
    }
    else if (!spell.empty() || specialSymbolEncounter)
        throw Parsing::ParsingError();

    if (tmp.size() == 5) {
        switch (tmp[4]) {
        case 'q':
            result.promoteTo = Queen; break;
        case 'n':
            result.promoteTo = Knight; break;
        case 'b':
            result.promoteTo = Bishop; break;
        case 'r':
            result.promoteTo = Rook; break;
        default:
            throw Parsing::ParsingError();
        }
    }

    return result;
}

SpellChessLib::SpellChessMove::SpellChessMove(const char * stringValue) {
    *this = parseMove(stringValue);
};

SpellChessLib::SpellChessMove::SpellChessMove(const std::string & token) {
    *this = parseMove(token);
};

SpellChessLib::SpellChessMove::SpellChessMove(uint8_t from, uint8_t to, uint8_t freezeSquare, uint8_t jumpSquare, PieceType promoteTo) :
    from(from), to(to), freezeSquare(freezeSquare), jumpSquare(jumpSquare), promoteTo(promoteTo) {};


void SpellChessLib::validateSpellChessMove(const SpellChessMove& move)
{
    if (move.from >= 64 || move.to >= 64)
        throw InvalidMoveException();

    if (move.freezeSquare != NoSquare && move.freezeSquare >= 64)
        throw InvalidMoveException();

    if (move.jumpSquare != NoSquare && move.jumpSquare >= 64)
        throw InvalidMoveException();

    if (move.jumpSquare != NoSquare && move.freezeSquare != NoSquare)
        throw InvalidMoveException();
}

SquareIndex SpellChessLib::calculateEnpassantPawnIndex(const SquareIndex& from, const SquareIndex& to) {
    return to > from ? to - 8 : to + 8;
}

SquareIndex SpellChessLib::calculateEnpassantSquareIndex(const SquareIndex& from, const SquareIndex& to) {
    return (std::abs(from - to) == 16) ? (from + to) / 2 : NoSquare;
}

inline bool internal_isKingMoveCastling(const SquareIndex& from, const SquareIndex& to) {
    auto d = std::abs(to % 8 - from % 8);
    return d == 2;
}

bool SpellChessLib::GameState::hasSufficientMatingMaterial(const PlayerColor& side) const
{
    size_t ourPieces[7] = {0, 0, 0, 0, 0, 0, 0};
    size_t theirPieces[7] = { 0, 0, 0, 0, 0, 0, 0 };;
    size_t ourPiecesTotal = 0;
    size_t theirPiecesTotal = 0;
    bool whBishopPresent = false, blBishopPresent = false;
    for (SquareIndex i = 0; i < 64; ++i) {
        if (board[i].piece.owner == side) {
            ourPieces[board[i].piece.type]++;
            ourPiecesTotal++;
        }

        if (board[i].piece.owner == flipColor(side)) {
            ourPieces[board[i].piece.type]++;
            theirPiecesTotal++;
        }
    }

    bool hasFreezesLeft = spells[side - 1][FreezeSpell].usagesLeft > 0;
    bool hasJumpsLeft = spells[side - 1][JumpSpell].usagesLeft > 0;

    if (ourPieces[Pawn] || ourPieces[Rook] || ourPieces[Queen])
        return true;

    if (ourPiecesTotal == 1)
        return false; // lone king

    if (ourPiecesTotal == 2 && ourPieces[Bishop] == 1 && theirPiecesTotal == 1 && !hasFreezesLeft && !hasJumpsLeft)
        return false; // KB vs K  

    if (ourPiecesTotal == 2 && ourPieces[Knight] == 1 && theirPiecesTotal == 1 && !hasFreezesLeft)
        return false; // KN vs K

    if (ourPiecesTotal == 3 && ourPieces[Knight] == 2 && !hasFreezesLeft)
        return false; // KNN vs K

    return true;
}

void SpellChessLib::GameState::makeMove(const SpellChessMove& move)
{

    PieceType type = board[move.from].piece.type, capturedType = board[move.to].piece.type;
    int colorIndex = sideToMove - 1;

    bool isEnpassant = move.to == enpassantSquare && type == Pawn;
    bool resetHalfMoveCounter = (!board[move.to].isEmpty()) || type == Pawn;
    bool isCastling = type == King && internal_isKingMoveCastling(move.from, move.to);
    
    clearSpells();

    if (move.freezeSquare != NoSquare) {
        frozeArea(move.freezeSquare);
        spellFields.push_back({FreezeSpell, sideToMove, move.freezeSquare, 1});
        decrementSpellUsages(sideToMove, FreezeSpell);
    }
    else
        decrementSpellCooldown(sideToMove, FreezeSpell);
    
    if (move.jumpSquare != NoSquare) {
        makeJumpable(move.jumpSquare);
        spellFields.push_back({JumpSpell, sideToMove, move.jumpSquare, 1});
        decrementSpellUsages(sideToMove, JumpSpell);
    }
    else
       decrementSpellCooldown(sideToMove, JumpSpell);
       

    if (isCastling) {
        makeCastling(sideToMove, move.to > move.from ? Kingside : QueenSide);
    } else {
        enpassantSquare = (type == Pawn) ? calculateEnpassantSquareIndex(move.from, move.to) : NoSquare;
        board[move.to].piece = (move.promoteTo != None) ? Piece({move.promoteTo, sideToMove}) : board[move.from].piece;
        board[move.from].piece = NO_PIECE;
        
        if (isEnpassant) 
            board[calculateEnpassantPawnIndex(move.from, move.to)].piece = NO_PIECE;
        
        if (type == King) 
            castlingRights[sideToMove - 1][0] = castlingRights[sideToMove - 1][1] = false;

        if (type == Rook)
            revokeCastlingRightsAssociatedWithGivenRook(move.from);

        if (capturedType == Rook)
            revokeCastlingRightsAssociatedWithGivenRook(move.to);
    }

    halfMoveRuleCounter = resetHalfMoveCounter ? 0 : (halfMoveRuleCounter + 1);
    if (sideToMove == Black) 
        ++fullMoveCounter;
    sideToMove = flipColor(sideToMove);
}

int SpellChessLib::GameState::countPiece(Piece piece) const{
    int result = 0;
    for(size_t i = 0; i < 64; ++i)
        if (board[i].piece == piece)
            ++result;
    return result;
};

int SpellChessLib::GameState::countPiece(PieceType type, PlayerColor color) const{
    return countPiece(Piece{type, color});
}



void SpellChessLib::FenValidator::throwIfInvalid(std::string_view fen) const
{
    if (!isValidFen(fen)) 
        throw InvalidFenException(std::string(fen));
}

size_t SpellChessLib::StandardChessFenValidator::countCharactersToWhiteSpace(std::string_view str) const
{
    size_t res = 0;
    for (auto c: str) 
        if (isspace(c))
            return res;
        else
            ++res;
    return res;
}


bool SpellChessLib::StandardChessFenValidator::validatePieceList(std::string_view pieceList) const
{
    int squaresVisited = 0;
    int squaresInCurrentRow = 0;
    int rowsCount = 1;

    char allowedTokens[12] = { 'p', 'n', 'b', 'r', 'q', 'k', 'P', 'N', 'B', 'R', 'Q', 'K' };
    
    bool lastCharWasDigit = false;


    for (auto c : pieceList) {
        
        for (auto token : allowedTokens)
            if (token == c)
                goto VALID_TOKEN;

        if (c == '/') {
            lastCharWasDigit = false;
            if (squaresInCurrentRow != 8) return false;
            
            ++rowsCount;
            squaresInCurrentRow = 0;
            continue;
        }
        
        if (c >= '1' && c <= '8') {

            if (lastCharWasDigit) return false; // consecutive digits are not allowed

            squaresVisited += c - '0';
            squaresInCurrentRow += c - '0';
            lastCharWasDigit = true;
            continue;
        }

        return false;

    VALID_TOKEN:
        lastCharWasDigit = false;
        ++squaresVisited;
        ++squaresInCurrentRow;
        continue;
    }
    
    return squaresVisited == 64 && rowsCount == 8;
}

bool SpellChessLib::StandardChessFenValidator::validateCastlingRightsString(std::string_view str) const
{
    if (str.empty())
        return false;
    
    if (str.at(0) == '-')
        return str.size() == 1;
    
    int occurence_count[4] = { 0, 0, 0, 0 };
    size_t placements[4] = { 0, 0, 0, 0 };
    char allowedCharacters[4] = {'K', 'Q', 'k', 'q'};
    bool validChar;
    
    for (size_t i = 0; i < str.size(); ++i) {
        validChar = false;
        for(size_t j = 0; j < 4; ++j)
            if (str[i] == allowedCharacters[j]) {
                ++occurence_count[j];
                placements[j] = i;
                validChar = true;
                break;
            }
        if (!validChar) return false;
    };
        
    // ensure any characters exists at most once
    for (size_t i = 0; i < 4; ++i) if (occurence_count[i] > 1)
        return false;
    
    // ensure characters follow KQkq order
    for (size_t i = 0; i < 3; ++i)
        if (occurence_count[i] && occurence_count[i + 1] && placements[i + 1] < placements[i])
            return false;

    return true;
}

bool SpellChessLib::StandardChessFenValidator::validateSquareString(std::string_view str) const
{
    bool result = Parsing::validateSquareIndexString(str);
    return result;
}

bool SpellChessLib::StandardChessFenValidator::validateSideToMove(std::string_view str) const
{
    return str.size() == 1 && (str[0] == 'w' || str[0] == 'b');
}

bool SpellChessLib::StandardChessFenValidator::validateTokenized(const std::vector<std::string>& tokens) const
{
    if (tokens.size() != 6) return false;

    if (!validatePieceList(tokens[0]))
        return false;

    if (!validateSideToMove(tokens[1]))
        return false;

    if (!validateCastlingRightsString(tokens[2]))
        return false;

    if (!validateSquareString(tokens[3]))
        return false;

    int tmp;

    if ((std::stringstream(tokens[4]) >> tmp).fail() || tmp < 0 || tmp> 100)
        return false;

    if ((std::stringstream(tokens[5]) >> tmp).fail() || tmp < 0)
        return false;

    return true;
}

bool SpellChessLib::StandardChessFenValidator::isValidFen(std::string_view fen) const
{
    // 
    // <pieceList> w/b <CastlingRights> <enp> <half_moves> <full_moves>
    auto tokens = Parsing::tokenizeStringBySpace(fen);
    return validateTokenized(tokens);
}


bool SpellChessLib::SpellChessFenValidator::validateSpell(std::string_view str) const
{
    // example: "F00/J00/f00/j00"

    if (str.size() < 15) return false;
    
    char c, spell = 0;

    for (size_t i = 0; i < 15; ++i) {
        c = str[i];
        switch (i%4)
        {
        case 0: 
            if(c != 'F' && c!= 'f' && c != 'J' && c != 'j') return false;
            spell = c;
            break;
            
        case 1:
            if ((spell == 'F' || spell == 'f') && (c > '5' || c < '0')) return false;
            if ((spell == 'J' || spell == 'j') && (c > '2' || c < '0')) return false;

            break;

        case 2:
            if (c < '0' || c > '2') return false;
            break;
        case 3:
            if (c != '/') return false;
            break;
        }
    }


    return true;
}

bool SpellChessLib::SpellChessFenValidator::isValidFen(std::string_view fen) const
{
    StandardChessFenValidator standardChessValidator;
    auto tokens = Parsing::tokenizeStringBySpace(fen);

    if (tokens.size() != 9)
        return false;

    std::vector<std::string> standardChessTokens;

    for (size_t i = 0; i < 6; ++i) standardChessTokens.push_back(tokens[i]);

    if (!standardChessValidator.validateTokenized(standardChessTokens))
        return false;

    if (!standardChessValidator.validateSquareString(tokens[6]))
        return false;

    if (!standardChessValidator.validateSquareString(tokens[7]))
        return false;

    if (!validateSpell(tokens[8]))
        return false;

    return true;
}

void SpellChessLib::FenParser::loadSq(SquareIndex& sq, const std::string& token)
{
   sq = Parsing::parseSquareIndex(token.c_str());  
}

void SpellChessLib::FenParser::loadInteger(uint16_t& target, const std::string& token)
{
    target = Parsing::parseUnsignedInteger(token.c_str());
}

void SpellChessLib::FenParser::loadPieceListString(GameState& state, const std::string& token)
{
    size_t index = 0;

    PlayerColor currentColour;

    for (size_t i = 0; i < 64; ++i) state.board->piece = NO_PIECE;

    for (char c : token) {

        if (c >= '1' && c <= '8') {
            index += c - '0';
            continue;
        }

        if (c == '/') continue;

        currentColour = (c >= 'A' && c <= 'Z') ? White : Black;
        if (currentColour == White)
            c = c - 'A' + 'a';
        
        switch (c)
        {
        case 'p':
            state.board[index].piece = {Pawn, currentColour};
            break;
        case 'n':
            state.board[index].piece = {Knight, currentColour };
            break;
        case 'b':
            state.board[index].piece = {Bishop, currentColour };
            break;
        case 'r':
            state.board[index].piece = { Rook, currentColour };
            break;
        case 'q':
            state.board[index].piece = { Queen, currentColour };
            break;
        case 'k':
            state.board[index].piece = { King, currentColour };
            break;
        }
        ++index;
    }
}

void SpellChessLib::FenParser::loadSideToMove(GameState& state, const std::string& token)
{
    state.sideToMove = (token[0] == 'w') ? White : Black;
}

void SpellChessLib::FenParser::loadCastlingRightString(GameState& state, const std::string& token)
{
    bool castlingRightsBuffer[4] = { false, false, false, false };

    if (token[0] == '-') {
        state.loadCastlingRights(castlingRightsBuffer);
    }

    for (char c : token) {
        switch (c) {
        case 'K':
            castlingRightsBuffer[0] = 1; break;
        case 'Q':
            castlingRightsBuffer[1] = 1; break;
        case 'k':
            castlingRightsBuffer[2] = 1; break;
        case 'q':
            castlingRightsBuffer[3] = 1; break;
        }
    }

    state.loadCastlingRights(castlingRightsBuffer);
}

void SpellChessLib::FenParser::loadSpells(GameState& state, const std::string& token)
{
    char spell;
    uint16_t count, cooldown, index = 0;
    SpellCounter counters[4];

    counters[0].spellId = counters[2].spellId = FreezeSpell;
    counters[1].spellId = counters[3].spellId = JumpSpell;

    for (int section = 0; section < 4; ++section) {
        spell = token[4 * section];
        count = token[4 * section + 1] - '0';
        cooldown = token[4 * section + 2] - '0';

        if (spell == 'F') index = 0;
        else if (spell == 'J') index = 1;
        else if (spell == 'f') index = 2;
        else if (spell == 'j') index = 3;

        counters[index].cooldown = cooldown;
        counters[index].usagesLeft = count;
    };

    state.loadSpellInfos(counters);
}

void SpellChessLib::FenParser::_loadStandardFen(GameState& state, const std::vector<std::string>& tokens)
{
    loadPieceListString(state, tokens[0]);
    loadSideToMove(state, tokens[1]);
    loadCastlingRightString(state, tokens[2]);
    loadSq(state.enpassantSquare, tokens[3]);
    loadInteger(state.halfMoveRuleCounter, tokens[4]);
    loadInteger(state.fullMoveCounter, tokens[5]);
    state.spellChessMode = false;
}

void SpellChessLib::FenParser::_loadSpellChessFen(GameState& state, const std::vector<std::string>& tokens)
{
    _loadStandardFen(state, tokens);
	state.spellFields.clear();

    loadSq(state.frozeSquare, tokens[6]);
	if(state.frozeSquare != NoSquare) {
        state.frozeArea(state.frozeSquare);
		state.spellFields.push_back({FreezeSpell, flipColor(state.sideToMove), state.frozeSquare, 1});
	}
    
    loadSq(state.jumpSquare, tokens[7]);
	if (state.jumpSquare != NoSquare) {
        state.makeJumpable(state.getJumpSquare());
		state.spellFields.push_back({JumpSpell, flipColor(state.sideToMove), state.jumpSquare, 1});
	}
    
    loadSpells(state, tokens[8]);
    state.spellChessMode = true;
}

GameState SpellChessLib::FenParser::fromStandardChessFen(const std::string& fen)
{
    StandardChessFenValidator validator;
    validator.throwIfInvalid(fen.c_str());

    GameState result;
    auto tokens = Parsing::tokenizeStringBySpace(fen.c_str());
    _loadStandardFen(result, tokens);
    return result;
}

GameState SpellChessLib::FenParser::fromSpellChessFen(const std::string& fen)
{
    SpellChessFenValidator validator;
    validator.throwIfInvalid(fen.c_str());

    GameState result;
    auto tokens = Parsing::tokenizeStringBySpace(fen.c_str());
    _loadSpellChessFen(result, tokens);
    return result;
}


