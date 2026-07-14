#include <SpellChess/SpellChessUciLoaderPort.h>
#include <SpellChess/StringTokenizer.h>
void SpellChessLib::SpellChessMoveMarshaler::loadInto(const std::string& token, SpellChessMove & target) const {
	target = token;
}

std::string SpellChessLib::StateFormatter::toFen() const
{
	return state.toFen();
}

std::string SpellChessLib::StateFormatter::toPositionString() const
{
	return "fen " + state.toFen();
}

std::shared_ptr<UCILoader::EngineInstanceBuilder<SpellChessLib::SpellChessMove>> SpellChessLib::SpellChessEngineInstanceBuilder =
std::make_shared<UCILoader::EngineInstanceBuilder<SpellChessLib::SpellChessMove>>(
	std::make_shared<SpellChessLib::SpellChessMoveMatcher>(),
	std::make_shared<SpellChessLib::SpellChessMoveMarshaler>()
);

bool SpellChessLib::SpellChessMoveMatcher::match(const std::string& val) const
{
	try {
		parseMove(val.c_str());
	} catch(Parsing::ParsingError e) {
		return false;
	}
	return true;
}

template <>
std::string(*UCILoader::UciFormatter<SpellChessLib::SpellChessMove>::MoveFormatter)(const SpellChessLib::SpellChessMove& m) = SpellChessLib::spellChessMoveToString;