#include "Configuration.h"
#include <SpellChess/Board.h>
#include <UCILoader/UCI.h>
#include <cxxopts.hpp>
#include <iostream>

Grimoire::Configuration *Grimoire::parseConfiguration(cxxopts::ParseResult &parsedArguments)
{
    Configuration * result = new Configuration;
     UCILoader::GoParamsBuilder<SpellChessLib::SpellChessMove> builder;
    result->go_params = builder.build();

    if (parsedArguments.contains("depth"))
        result->go_params.Depth = parsedArguments["depth"].as<unsigned int>();
    else
        result->go_params.Depth = 0;

    if (parsedArguments.contains("time_per_pos"))
        result->go_params.MoveTime = parsedArguments["time_per_pos"].as<unsigned int>();
    else
        result->go_params.MoveTime = 0;

    result->engine_timeout = std::chrono::milliseconds(parsedArguments["timeout"].as<unsigned int>());

    result->evalBounds = parsedArguments["bounds"].as<unsigned int>();
    result->extendedNodeLimit = parsedArguments["nodes"].as<unsigned int>();

    if (result->go_params.MoveTime && result->engine_timeout.count() < result->go_params.MoveTime) {
        std::cerr << "Specified timeout time can not be lesser than time_per_pos parameter" << std::endl;
        delete result;
        return nullptr;
    };

    if (parsedArguments.contains("engine")) 
        result->pathToEngine = parsedArguments["engine"].as<std::string>();
    else {
        std::cerr << "Missing mandatory parameter: engine" << std::endl;
        delete result;
        return nullptr;
    }

    if (parsedArguments.contains("output")) 
        result->pathToOutput = parsedArguments["output"].as<std::string>();
    else {
        std::cerr << "Missing mandatory parameter: output" << std::endl;
        delete result;
        return nullptr;
    }

    if (result->go_params.Depth == 0 && result->go_params.MoveTime == 0) {
        std::cerr << "No engine search stop condition specified, defaulting to time_per_pos=1000" << std::endl;
        result->go_params.MoveTime = 1000;
    }

    if (parsedArguments.contains("max"))
        result->maximumOpeningDepth = parsedArguments["max"].as<unsigned int>();
    else
        result->maximumOpeningDepth = -1u;

    return result;
}

