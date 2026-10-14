#include <SpellChess/PGN4Exporter.h>
#include <SpellChess/MoveGen.h>
#include <SpellChess/StringTokenizer.h>
#include <SpellChess/MoveLegality.h>
#include <SpellChess/Board.h>
#include <SpellChess/GameAnnotations.h>
#include <stdexcept>
#include <cstddef>
#include <variant>
#include <vector>

void SpellChessLib::PGN4Exporter::writeTagLine(const Annotation::TagPair& tagPair, std::ostream& out){
	out << '[' << tagPair.name << " \"" << tagPair.value << "\"]\n";
}

void SpellChessLib::PGN4Exporter::writeAnnotationSequenceElement(const Annotation::AnnotatedSequenceElement& element, std::ostream& out){
	std::string token;
	switch (element.index()) {
	case (size_t)Annotation::AnnotatedSequenceElementType::Comment: 
		out << " {" << std::get<std::string>(element) << "}";
		break;
	case (size_t)Annotation::AnnotatedSequenceElementType::NumericAnnotationGlyph:
		// not supported yet
		break;
	case (size_t)Annotation::AnnotatedSequenceElementType::Move:

		if (!firstMoveWritten && currentState.getSideToMove() == Black) {
			out << std::endl << currentState.getFullMoveCounter() - moveCounterOffset << ". ";
		};

		if (currentState.getSideToMove() == White) {
			out << std::endl << currentState.getFullMoveCounter() - moveCounterOffset << ". ";
		}

		if (currentState.getSideToMove() == Black && firstMoveWritten) {
			out << ".. ";
		};

		firstMoveWritten = true;
		token = makeTokenFromMoveAndAdvance(std::get<SpellChessMove>(element));
		out << token;
		break;
	default:
		throw std::logic_error("Unknown Sequence element");
	}
}

void SpellChessLib::PGN4Exporter::writeSequence(const Annotation::AnnotatedSequence& sequence, std::ostream& out){
	for (const Annotation::AnnotatedSequenceNode& element : sequence) {
		switch (element.index()) {
		case (size_t)Annotation::AnnotatedSequenceNodeType::Variation:
			break;
		case (size_t)Annotation::AnnotatedSequenceNodeType::Element:
			writeAnnotationSequenceElement(std::get<Annotation::AnnotatedSequenceElement>(element), out);
			break;

		default:
			throw std::logic_error("Unknown Sequence element");
		}
	}
}

char SpellChessLib::PGN4Exporter::checkMark() const {
	MoveGenerator gen(currentState);
	
	bool currentPlayerHasKing = findKingPos(currentState, currentState.getSideToMove()) != NoSquare;

	if (currentPlayerHasKing && !gen.isInCheck())
		return '\0';

	if (!currentPlayerHasKing)
		return '#';

	return (gen.generateMoves().size()) ? '+' : '#';
}

std::string SpellChessLib::PGN4Exporter::makeStartFen4BoardPart(const GameState& root, const char * pieceSymbols) const{
	std::string result;
	char sq[3] = { 'a', '8', 0 };
	SquareIndex index;
	size_t acc;
	result += "x,x,x,x,x,x,x,x,x,x,x,x,x,x/x,x,x,x,x,x,x,x,x,x,x,x,x,x/x,x,x,x,x,x,x,x,x,x,x,x,x,x/";
	for (sq[1] = '8'; sq[1] >= '1'; --sq[1]) {
		result += "x,x,x";
		acc = 0;
		for (sq[0] = 'a'; sq[0] <= 'h'; ++sq[0]) {
			index = Parsing::parseSquareIndex(sq);
			if (!root.getSquare(index).isEmpty()) {
				if (acc) {
					result += ',' + std::to_string(acc);
					acc = 0;
				}
				result += ',';
				result += root.getSquare(index).piece.owner == White ? 'r' : 'y';
				result += pieceSymbols[root.getSquare(index).piece.type];
			}
			else
				++acc;
		}
		if (acc)
			result += ',' + std::to_string(acc);
		result += ",x,x,x/";
	}
	result += "x,x,x,x,x,x,x,x,x,x,x,x,x,x/x,x,x,x,x,x,x,x,x,x,x,x,x,x/x,x,x,x,x,x,x,x,x,x,x,x,x,x";
	return result;
}

std::string SpellChessLib::PGN4Exporter::serializeSpell(const GameState& root, const PlayerColor& color) const
{
	std::string result = "'";
	result += std::string() + "jump_" + std::to_string(root.getSpell(color, JumpSpell).cooldown) + "x" + std::to_string(root.getSpell(color, JumpSpell).usagesLeft);
	result += ',';
	result += std::string() + "freeze_" + std::to_string(root.getSpell(color, FreezeSpell).cooldown) + "x" + std::to_string(root.getSpell(color, FreezeSpell).usagesLeft);
	result += '\'';
	return result;
}

std::string SpellChessLib::PGN4Exporter::serializeSpellFields(const GameState& root) const
{
	std::string result = "(";
	bool first = true;
	for (const auto& field : root.getSpellFields()) {
		if (!first)
			result += ',';
		first = false;
		result += "'" + std::to_string(field.life) + "_" + std::to_string(field.player - 1) + "_";
		result += field.type == JumpSpell ? "jump@" : "freeze@";
		result += SquareIndexToToken(field.target) + "'";
	}
	result += ')';
	return result;
}

std::string SpellChessLib::PGN4Exporter::SquareIndexToToken(const SquareIndex index) const{
	std::string sq = squareIndexToString(index);
	char file = sq[0] + 3;
	int rank = (sq[1] - '0' + 3);
	return std::string({ file }) + std::to_string(rank);
}

std::string SpellChessLib::PGN4Exporter::makeTokenFromMoveAndAdvance(const SpellChessMove& move){
	std::string result;
	char pieceSymbols[7] = { 0, 'P', 'N', 'B', 'R', 'Q', 'K' };

	if (move.freezeSquare != NoSquare)
		result += std::string("freeze@") + SquareIndexToToken(move.freezeSquare) + "&";

	if (move.jumpSquare != NoSquare)
		result += std::string("jump@") + SquareIndexToToken(move.jumpSquare) + "&";

	PieceType movedPiece = currentState.getSquare(move.from).piece.type;
	PieceType capturedPiece = currentState.getSquare(move.to).piece.type;

	if (movedPiece == King) {
		int castlingDirection = isMoveCastling(currentState, move.from, move.to);
		if (castlingDirection == int(Kingside)) {
			result = "O-O";
			goto BASE_CALCULATED;
		}

		if (castlingDirection == int(QueenSide)) {
			result = "O-O-O";
			goto BASE_CALCULATED;
		}
	}

	if (movedPiece > Pawn)
		result.push_back(pieceSymbols[movedPiece]);
	
	result += SquareIndexToToken(move.from);
	result += (!currentState.getSquare(move.to).isEmpty() || move.to == currentState.getEnppassantSquare()) ? 'x' : '-';
	
	if (capturedPiece > Pawn)
		result.push_back(pieceSymbols[capturedPiece]);

	result += SquareIndexToToken(move.to);

	if (move.promoteTo != None) {
		result += '=';
		result += pieceSymbols[move.promoteTo];
	}
	
BASE_CALCULATED:
	currentState.makeMove(move);
	char tmp = checkMark();
	if (tmp) result += tmp;
	result += ' ';
	return result;
}



SpellChessLib::Annotation::TagPair SpellChessLib::PGN4Exporter::makeStartFen4(const GameState& root) const {
	std::string result;
	char pieceSymbols[7] = { 0, 'P', 'N', 'B', 'R', 'Q', 'K' };
	result += root.getSideToMove() == White ? "R" : "Y"; // White is 'Red' and Black is 'Yellow'
	result += "-0,1,0,1"; // player elimination status. Fixed value for spell chess
	result += std::string("-") + (root.canCastle(White, Kingside) ? "1" : "0") + ",0," + (root.canCastle(Black, Kingside) ? "1" : "0") + ",0";
	result += std::string("-") + (root.canCastle(White, QueenSide) ? "1" : "0") + ",0," + (root.canCastle(Black, QueenSide) ? "1" : "0") + ",0";
	result += "-0,0,0,0"; // player points. not relevant for spell chess
	result += std::string("-") + std::to_string(root.getHalfMoveCounter());
	result += '-';

	// some inline json, because why not chess.com
	result += "{'pawnBaseRank':5,'wb':true,'dim':'8x8',";

	// spell section
	result += "'spells':(";
	result += serializeSpell(root, White);
	result += ", '',";
	result += serializeSpell(root, Black);
	result += ", '')";

	result += ",'spellFields':" + serializeSpellFields(root);

	result += "}"; // end of json

	result += "-" + makeStartFen4BoardPart(root, pieceSymbols);

	return { "StartFen4", result };
}

void SpellChessLib::PGN4Exporter::exportPGN4(const GameState& root, const Annotation::GameAnnotations& annotations){
	std::ostream& out = *out_ptr;
	currentState = root;
	moveCounterOffset = root.getFullMoveCounter() - 1;

	writeTagLine(makeStartFen4(root), out);
	writeTagLine({"RuleVariants", "EnPassant Play4Mate Spell=1+3"}, out);

	for (const auto& tag : annotations.tagSection)
		writeTagLine(tag, out);

	out << "\n";

	writeSequence(annotations.annotatedSequence, out);

}
