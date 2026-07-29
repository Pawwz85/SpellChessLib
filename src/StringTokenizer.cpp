#include <SpellChess/StringTokenizer.h>
#include <string.h>
#include <sstream>

using namespace SpellChessLib;

std::vector<std::string> SpellChessLib::Parsing::tokenizeStringBySpace(std::string_view str)
{
    std::vector<std::string> result;
    std::string acc;

    for (char c : str) {
        if (isspace(c) && !acc.empty()) {
            result.push_back(acc);
            acc.clear();
        }
        else {
            acc.push_back(c);
        }
    }

    if (!acc.empty()) result.push_back(acc);

    return result;
}



uint16_t SpellChessLib::Parsing::parseUnsignedInteger(std::string_view str)
{
    uint16_t res;
    if ((std::stringstream(std::string(str)) >> res).fail())
        throw ParsingError();
    return res;
}