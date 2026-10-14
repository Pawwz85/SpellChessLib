#include "Report.h"
#include "Configuration.h"
#include <SpellChess/MatchStatistics.h>
#include <nlohmann/json.hpp>
#include <fstream>

Runner::Report Runner::makeReport(const SpellChessLib::Stats::TrinomialModel &model,  const Configuration & config){
    Report result;
    result.numberOfGames = SpellChessLib::Stats::getNumberOfGames(model);
    result.wins = model.wins;
    result.draws = model.draws;
    result.losses = model.losses;
    result.eloDiff = SpellChessLib::Stats::eloDiff(model);
    result.drawRate = SpellChessLib::Stats::drawRatio(model);
    result.winRate = SpellChessLib::Stats::winRatio(model);
    result.LLR = SpellChessLib::Stats::calculateLLR(model, config.elo0, config.elo1);
    return result;
}
void Runner::to_json(nlohmann::json &j, const Report &report)
{
    j = nlohmann::json{
        {"games_total", report.numberOfGames},
        {"wins", report.wins},
        {"draws", report.draws},
        {"losses", report.losses},
        {"win_rate", report.winRate},
        {"draw_rate", report.drawRate},
        {"elo_difference", report.eloDiff},
        {"LLR", report.LLR}
    };
}

void Runner::exportReport(const Report &report, const std::string &file){
    nlohmann::json j = report;
    std::ofstream out;
    out.open(file);
    out << j;
    out.close();
}
