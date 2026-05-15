#pragma once

#include <nlohmann/json.hpp>
#include <SpellChess/MatchStatistics.h>
#include "Configuration.h"

namespace Runner {

    struct Report {
        unsigned int numberOfGames; 
        unsigned int wins;
        unsigned int losses;
        unsigned int draws;  
        double winRate;
        double drawRate;
        double eloDiff;
        double LLR;
    };

    extern Report makeReport(const SpellChessLib::Stats::TrinomialModel & model, const Configuration & config);
    extern void to_json(nlohmann::json & j, const Report & report);
    extern void exportReport(const Report & report, const std::string & file);
};