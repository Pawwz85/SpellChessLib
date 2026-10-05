#pragma once

#include "Board.h"
#include "GameAnnotations.h"
#include <ostream>

namespace SpellChessLib {
	class PGN4Exporter {
		std::ostream* out_ptr;
		GameState currentState;
		uint16_t moveCounterOffset;

		bool firstMoveWritten = false;

		void writeTagLine(const Annotation::TagPair& pair, std::ostream & out);
		void writeAnnotationSequenceElement(const Annotation::AnnotatedSequenceElement& element, std::ostream& out);
		void writeSequence(const Annotation::AnnotatedSequence& sequence, std::ostream& out);
	protected:
		char checkMark() const;
		std::string makeStartFen4BoardPart(const GameState& root, const char* pieceSymbols) const;
		std::string serializeSpell(const GameState& root, const PlayerColor& color) const;
		std::string serializeSpellFields(const GameState& root) const;
		std::string SquareIndexToToken(const SquareIndex index) const;
		std::string makeTokenFromMoveAndAdvance(const SpellChessMove& move);
		Annotation::TagPair makeStartFen4(const GameState& root) const;

	public:
		PGN4Exporter(std::ostream* out_ptr) : out_ptr(out_ptr) {};
		void exportPGN4(const GameState& root, const Annotation::GameAnnotations& annotations);
	};
}