#pragma once

#include "Board.h"
#include <UCILoader/UCI.h>
#include <UCILoader/Parser.h>
#include <UCILoader/EngineConnection.h>
#include "SpellChessUciLoaderPort.h"

namespace SpellChessLib {

	class SpellChessMoveMarshaler : public UCILoader::Marschaler<SpellChessLib::SpellChessMove> {
		// Odziedziczono za po�rednictwem elementu Marschaler
		void loadInto(const std::string& token, SpellChessMove & target) const override;
	};

	class SpellChessMoveMatcher : public UCILoader::PatternMatcher {
		// Odziedziczono za po�rednictwem elementu PatternMatcher
		bool match(const std::string& val) const override;
	};

	class StateFormatter : public UCILoader::PositionFormatter {
		SpellChessLib::GameState state;
	public:
		StateFormatter(const GameState& state) : state(state) {};
		// Odziedziczono za po�rednictwem elementu PositionFormatter
		std::string toFen() const override;
		std::string toPositionString() const override;
	};

	extern std::shared_ptr<UCILoader::EngineInstanceBuilder<SpellChessLib::SpellChessMove>> SpellChessEngineInstanceBuilder;

}