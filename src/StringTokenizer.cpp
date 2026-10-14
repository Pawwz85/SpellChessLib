#include <SpellChess/StringTokenizer.h>
#include <cctype>
#include <cstddef>
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

 std::vector<std::string_view> SpellChessLib::Parsing::splitString(std::string_view str, char delim) {
    std::vector<std::string_view> result;
    size_t lastDelimPos = -1;

    if (str.empty())
        return result;

    for (size_t i = 0; i < str.size(); ++i) {
        if (str[i] == delim) {
            result.emplace_back(str.data() + lastDelimPos + 1, i - lastDelimPos - 1);
            lastDelimPos = i;
        }
    };

    result.emplace_back(str.data() + lastDelimPos + 1, str.size() - lastDelimPos - 1);
    return result;
}



uint16_t SpellChessLib::Parsing::parseUnsignedInteger(std::string_view str)
{
    uint16_t res;
    if ((std::stringstream(std::string(str)) >> res).fail())
        throw ParsingError();
    return res;
}