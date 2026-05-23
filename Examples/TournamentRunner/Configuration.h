#pragma once

#include <optional>
#include <cxxopts.hpp>

namespace Runner {

    struct Configuration {
        std::string oldEnginePath;  // Path for the old (baseline) engine
        std::string newEnginePath;  // Path for the new (tested) engine
        std::string outputPath;     // Output json file
        std::string openingBook;    // Grimoire opening book used during test
        int elo0;                   // null hypthesis
        int elo1;                   // alternative hypothesis
        double alpha;               
        double beta;
        int moveTime;               // move time in ms per move; can be zero only if baseTime is set
        int baseTime;               // base time in ms per move; can be zero only if moveTime is set
        int increment;              // increment time in ms per move; can be zero
        int latencyMargin;
        int jobs;
    };

    extern Configuration parseConfig(cxxopts::ParseResult & parsedArguments);
    extern std::optional<std::string> validateConfig(const Configuration & config);
};