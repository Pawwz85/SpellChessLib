#pragma once

#include <cstdint>
#include <chrono>
#include <string>
#include <SpellChess/Board.h>
#include <UCILoader/UCI.h>

namespace cxxopts { class ParseResult; }

namespace Grimoire {

    struct Configuration {
        UCILoader::GoParams<SpellChessLib::SpellChessMove> go_params;
        std::chrono::milliseconds engine_timeout;
        std::string pathToEngine;
        std::string pathToOutput;
        int32_t evalBounds;
        uint32_t maximumOpeningDepth;
        uint32_t extendedNodeLimit;
    };

    Configuration * parseConfiguration(cxxopts::ParseResult & parsedArguments);
};