#include <SpellChess/PGNExporter.h>
#include <SpellChess/MoveLegality.h> // for isMoveCastling function
#include <SpellChess/MoveGen.h>
#include <SpellChess/MoveNotation.h>
#include <algorithm>
#include <set>
#include <sstream>


std::string SpellChessLib::PGNExporter::calculateSanMoveTokenAndAdvance(const SpellChessMove& move){
	std::string result = spellChessMoveToSAN(move, currentState);
	currentState.makeMove(move);
	return result;
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
			throw std::logic_error("Unknown Sequence element");
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
		throw std::logic_error("Unknown Sequence element");
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
