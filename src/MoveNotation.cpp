#include <SpellChess/MoveNotation.h>
#include <SpellChess/MoveLegality.h> // for isMoveCastling function
#include <SpellChess/MoveGen.h>
#include <set>
#include <vector>
#include <stdexcept>
#include <algorithm>

enum SANAmbiguityLevel {
			NoAmbiguity,
			FileNeeded,
			RankNeeded,
			FullSquareNeeded
};

std::vector<SpellChessLib::SquareIndex> calculateAmbiguities(const SpellChessLib::PieceType& type, const SpellChessLib::SquareIndex& to, const SpellChessLib::GameState & state) {
    SpellChessLib::MoveGenerator gen(state);
	auto legalMoves = gen.generateMoves();
	std::set<SpellChessLib::SquareIndex> result; // we are using set to filter duplicates. Duplicate might arrive in case of pawn promotions

	for (auto move : legalMoves)
		if (state.getSquare(move.move.from).piece.type == type && move.move.to == to)
			result.insert(move.move.from);

	return std::vector<SpellChessLib::SquareIndex>(result.begin(), result.end());
};

SANAmbiguityLevel calculateAmbiguityLevel(const SpellChessLib::PieceType& type, const SpellChessLib::SpellChessMove& move, const bool & isCapture, const SpellChessLib::GameState & state) {
    if (type == SpellChessLib::Pawn) {
		return isCapture ? FileNeeded : NoAmbiguity;
	}

	auto ambiguities = calculateAmbiguities(type, move.to, state);
	
	if (ambiguities.empty())
		throw std::logic_error("Illegal Move");

	if (ambiguities.size() == 1)
		return NoAmbiguity;

	size_t matches = 0;
	int tmp = move.from % 8;
	if (std::count_if(ambiguities.cbegin(), ambiguities.cend(), [tmp](const SpellChessLib::SquareIndex& i) { return i % 8 == tmp; }) == 1)
		return FileNeeded;
	tmp = move.from / 8;
	if (std::count_if(ambiguities.cbegin(), ambiguities.cend(), [tmp](const SpellChessLib::SquareIndex& i) { return i / 8 == tmp; }) == 1)
		return RankNeeded;
	return FullSquareNeeded;
};

char checkMark(const SpellChessLib::GameState & state) {

	if (state.countPiece({SpellChessLib::King, state.getSideToMove()}) == 0)
		return '#';

    SpellChessLib::MoveGenerator gen(state);
	if(!gen.isInCheck()) 
		return '\0';

	return (gen.generateMoves().size()) ? '+' : '#';
};

std::string SpellChessLib::spellChessMoveToSAN(const SpellChessMove & move, const GameState & context) {
    char piecePrefixes[7] = { '\0', '\0', 'N', 'B', 'R', 'Q', 'K' };
	char prefix;
	PieceType movedPiece = context.getSquare(move.from).piece.type;
	bool isCapture = !context.getSquare(move.to).isEmpty();

	std::string base;

	if (movedPiece == None) 
		throw std::runtime_error("Illegal Move encountered");

	std::string from_str = squareIndexToString(move.from);
	std::string to_str = squareIndexToString(move.to);
	SANAmbiguityLevel lvl = NoAmbiguity;

	if (movedPiece == King) {
		int castlingDirection = isMoveCastling(context, move.from, move.to);
		if (castlingDirection == int(Kingside)) {
			base = "O-O";
			goto BASE_CALCULATED;
		}

		if (castlingDirection == int(QueenSide)) {
			base = "O-O-O";
			goto BASE_CALCULATED;
		}
	}

	if (movedPiece == Pawn && context.getEnppassantSquare() == move.to)
		isCapture = true;

	if ((prefix = piecePrefixes[movedPiece]))
		base += prefix;


	lvl = calculateAmbiguityLevel(movedPiece, move, isCapture, context);

	switch (lvl) {
	case FileNeeded:
		base += from_str[0];
		break;
	case RankNeeded:
		base += from_str[1];
		break;
	case FullSquareNeeded:
		base += from_str;
		break;
    default:
        break;
	}

	if (isCapture)
		base += 'x';

	base += to_str;

	if (move.promoteTo != None)
		base += std::string("=") + piecePrefixes[move.promoteTo];

BASE_CALCULATED:
    GameState copy = context;
	copy.makeMove(move);
	char tmp = checkMark(copy);
	if (tmp) base += tmp;
	base += ' ';
	return base;
};


std::string algebraicSpellPrefix(const SpellChessLib::SpellChessMove & move) {
    std::string result;

    if (move.freezeSquare != SpellChessLib::NoSquare) {
        result = "\U00002744";
        result += SpellChessLib::squareIndexToString(move.freezeSquare);
        result += "+";
    } else if (move.jumpSquare != SpellChessLib::NoSquare) {
        result = "\U000021B7";
        result += SpellChessLib::squareIndexToString(move.jumpSquare);
        result += "+";
    };

    return result;
}

std::string SpellChessLib::spellChessMoveToUnicodeString(const SpellChessMove & move, const GameState & context) {
    validateSpellChessMove(move);
    std::string result = algebraicSpellPrefix(move);

    /*static std::string pieceSymbols[7] = {
        "",     // SpellChessLib::None
        "",     // SpellChessLib::Pawn
        "N",    // SpellChessLib::Knight
        "B",    // SpellChessLib::Bishop 
        "R",    // SpellChessLib::Rook 
        "Q",    // SpellChessLib::Queen 
        "K",    // SpellChessLib::King 
    };*/

    result += spellChessMoveToSAN(move, context);
    return result;
};