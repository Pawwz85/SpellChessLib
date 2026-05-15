#pragma once

#include <UCILoader/EngineConnection.h>
#include <SpellChess/SpellChessUciLoaderPort.h>
#include <cstdint>
#include <cxxopts.hpp>

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