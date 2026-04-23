#include <SpellChess/PGNExporter.h>
#include <SpellChess/MoveLegality.h> // for isMoveCastling function
#include <SpellChess/MoveGen.h>
#include <algorithm>
#include <set>
#include <sstream>

std::vector<SpellChessLib::SquareIndex> SpellChessLib::PGNExporter::calculateAmbiguities(const PieceType& type, const SquareIndex& to) const{
	MoveGenerator gen(currentState);
	auto legalMoves = gen.generateMoves();
	std::set<SquareIndex> result; // we are using set to filter duplicates. Duplicate might arrive in case of pawn promotions

	for (auto move : legalMoves)
		if (currentState.getSquare(move.move.from).piece.type == type && move.move.to == to)
			result.insert(move.move.from);

	return std::vector<SquareIndex>(result.begin(), result.end());
}

SpellChessLib::PGNExporter::AmbiguityLevel SpellChessLib::PGNExporter::calculateAmbiguityLevel(const PieceType& type, const SpellChessMove& move, const bool& isCapture) const
{
	if (type == Pawn) {
		return isCapture ? FileNeeded : NoAmbiguity;
	}

	auto ambiguities = calculateAmbiguities(type, move.to);
	
	if (ambiguities.empty())
		throw std::exception("Illegal Move");

	if (ambiguities.size() == 1)
		return NoAmbiguity;

	size_t matches = 0;
	int tmp = move.from % 8;
	if (std::count_if(ambiguities.cbegin(), ambiguities.cend(), [tmp](const SquareIndex& i) { return i % 8 == tmp; }) == 1)
		return FileNeeded;
	tmp = move.from / 8;
	if (std::count_if(ambiguities.cbegin(), ambiguities.cend(), [tmp](const SquareIndex& i) { return i / 8 == tmp; }) == 1)
		return RankNeeded;
	return FullSquareNeeded;
}

char SpellChessLib::PGNExporter::checkMark() const{
	MoveGenerator gen(currentState);
	if(!gen.isInCheck()) 
		return '\0';

	return (gen.generateMoves().size()) ? '+' : '#';
}

std::string SpellChessLib::PGNExporter::calculateSanMoveTokenAndAdvance(const SpellChessMove& move){
	char piecePrefixes[7] = { '\0', '\0', 'N', 'B', 'R', 'Q', 'K' };
	char prefix;
	PieceType movedPiece = currentState.getSquare(move.from).piece.type;
	bool isCapture = !currentState.getSquare(move.to).isEmpty();

	std::string base;

	if (movedPiece == None) 
		throw std::exception("Illegal Move encountered");

	std::string from_str = squareIndexToString(move.from);
	std::string to_str = squareIndexToString(move.to);
	AmbiguityLevel lvl = NoAmbiguity;

	if (movedPiece == King) {
		int castlingDirection = isMoveCastling(currentState, move.from, move.to);
		if (castlingDirection == int(Kingside)) {
			base = "O-O";
			goto BASE_CALCULATED;
		}

		if (castlingDirection == int(QueenSide)) {
			base = "O-O-O";
			goto BASE_CALCULATED;
		}
	}

	if (movedPiece == Pawn && currentState.getEnppassantSquare() == move.to)
		isCapture = true;

	if (prefix = piecePrefixes[movedPiece])
		base += prefix;


	lvl = calculateAmbiguityLevel(movedPiece, move, isCapture);

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
	}

	if (isCapture)
		base += 'x';

	base += to_str;

	if (move.promoteTo != None)
		base += std::string("=") + piecePrefixes[move.promoteTo];

BASE_CALCULATED:
	currentState.makeMove(move);
	char tmp = checkMark();
	if (tmp) base += tmp;
	base += ' ';
	return base;
}

bool SpellChessLib::PGNExporter::isStartPos(const GameState& root) const{
	return root.toFen() == START_POS;
}

void SpellChessLib::PGNExporter::writeToken(const std::string& token, std::ostream& out){
	if (token.size() + currentLineLength > 80) {
		out << '\n';
		currentLineLength = 0;
	}
	currentLineLength += token.size();
	
	out << token;
}

void SpellChessLib::PGNExporter::writeTagLine(const Annotation::TagPair& tagPair, std::ostream& out)
{
	out << '[' << tagPair.name << " \"" << tagPair.value << "\"]\n";
}

void SpellChessLib::PGNExporter::writeSequence(const Annotation::AnnotatedSequence& sequence, std::ostream& out)
{
	GameState currentState = this->currentState;
	for (const Annotation::AnnotatedSequenceNode & element : sequence) {
		switch (element.index()) {
		case 0: // element is another AnnotatedSequenceNode
			currentState = this->currentState;
			writeToken("(", out);
			writeSequence(std::get<Annotation::AnnotatedSequence>(element), out);
			writeToken(")", out);
			this->currentState = currentState;
			break;
		case 1: // element is of type Annotation::AnnotatedSequenceElement
			writeAnnotationSequenceElement(std::get<Annotation::AnnotatedSequenceElement>(element), out);
			break;
		
		default:
			throw std::exception("Unknown Sequence element");
		}
	}
}

void SpellChessLib::PGNExporter::writeAnnotationSequenceElement(const Annotation::AnnotatedSequenceElement& element, std::ostream& out)
{
	std::string token;
	switch (element.index()) {
	case 0:
		writeToken("{", out);
		writeToken(std::get<std::string>(element), out);
		writeToken("}", out);
		break;
	case 1: // Annotation Glyph 
		writeNumericAnnotationGlyph(std::get<Annotation::NumericAnnotationGlyph>(element), out);
		break;
	case 2:
		if (currentState.getSideToMove() == White || !firstMoveWritten)
			writeNumberIdentification(Annotation::MoveIndication(currentState.getFullMoveCounter()), out);
		
		if (!firstMoveWritten && currentState.getSideToMove() == Black)
			writeToken("..", out);

		firstMoveWritten = true;
		token = calculateSanMoveTokenAndAdvance(std::get<SpellChessLib::SpellChessMove>(element));
		writeToken(token, out);
		break;
	default:
		throw std::exception("Unknown Sequence element");
	}
}

void SpellChessLib::PGNExporter::writeNumberIdentification(const Annotation::MoveIndication& identificationNumber, std::ostream& out)
{
	std::string token = std::to_string(identificationNumber.getValue()) + ".";
	writeToken(token, out);
}

void SpellChessLib::PGNExporter::writeNumericAnnotationGlyph(const Annotation::NumericAnnotationGlyph& glyph, std::ostream& out){
	std::string token = "$" + std::to_string(glyph.getGlyphCode()) + " ";
	writeToken(token, out);
}

void SpellChessLib::PGNExporter::writeTerminationReason(const Annotation::GameTermination& reason, std::ostream& out)
{ 
	switch (reason)
	{
	case Annotation::WhiteWon:
		writeToken("1-0", out);
		break;
	case Annotation::BlackWon:
		writeToken("0-1", out);
		break;
	case Annotation::Draw:
		writeToken("1/2-1/2", out);
		break;
	default:
		writeToken("*", out);
	}
}

void SpellChessLib::PGNExporter::exportInternal(const Annotation::GameAnnotations& annotations, std::ostream& out, bool skipTags){
	if (!skipTags) {
		for (const Annotation::TagPair& tagPair : annotations.tagSection)
			writeTagLine(tagPair, out);

		if (!isStartPos(annotations.root)) {
			writeTagLine(Annotation::TagPair({ "SetUp", "1" }), out);
			writeTagLine(Annotation::TagPair({ "FEN", annotations.root.toFen() }), out);
		}
	}
	

	currentState = annotations.root;
	writeSequence(annotations.annotatedSequence, out);
	writeTerminationReason(annotations.terminationReason, out);
}


void SpellChessLib::PGNExporter::exportPGN(const Annotation::GameAnnotations& annotations) {
	std::ostream& out = *out_ptr;
	exportInternal(annotations, out, false);
}

void SpellChessLib::PGNExporter::exportMoveTextOnly(const  Annotation::GameAnnotations& annotations) {
	std::ostream& out = *out_ptr;
	exportInternal(annotations, out, true);
}

std::string SpellChessLib::toInlinePgnMoveText(const Annotation::GameAnnotations& annotations){
	std::stringstream out;
	PGNExporter exporter(&out, INT_MAX);
	exporter.exportMoveTextOnly(annotations);
	return out.str();
}
