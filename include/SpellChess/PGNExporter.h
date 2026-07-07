#pragma once

#include "Board.h"
#include "GameAnnotations.h"
#include <ostream>

namespace SpellChessLib {

	class PGNExporter {

		enum AmbiguityLevel {
			NoAmbiguity,
			FileNeeded,
			RankNeeded,
			FullSquareNeeded
		};

		GameState currentState;
		std::ostream* out_ptr;
		size_t currentLineLength = 0; // used in Move text to limit line number to 80 characters
		size_t maxLineLength;
		bool firstMoveWritten = false;

		std::string calculateSanMoveTokenAndAdvance(const SpellChessMove& move);

		bool isStartPos(const GameState& root) const;

		void writeToken(const std::string& token, std::ostream& out);
		void writeTagLine(const Annotation::TagPair& tagPair, std::ostream & out);
		void writeSequence(const Annotation::AnnotatedSequence& sequence, std::ostream& out);
		void writeAnnotationSequenceElement(const Annotation::AnnotatedSequenceElement& element, std::ostream& out);
		void writeNumberIdentification(const Annotation::MoveIndication& identificationNumber, std::ostream& out);
		void writeNumericAnnotationGlyph(const Annotation::NumericAnnotationGlyph& glyph, std::ostream& out);
		void writeTerminationReason(const Annotation::GameTermination & reason, std::ostream& out);
		void exportInternal(const Annotation::GameAnnotations& sequence, std::ostream& out, bool skipTags);

	public:
		PGNExporter(std::ostream* out_ptr, const size_t & maxLineLength = 80) : out_ptr(out_ptr), maxLineLength(maxLineLength) {};
		virtual ~PGNExporter() = default;
		void exportPGN(const Annotation::GameAnnotations& annotations);
		void exportMoveTextOnly(const Annotation::GameAnnotations& annotations);
	};

	extern std::string toInlinePgnMoveText(const Annotation::GameAnnotations & anotations);
	
}