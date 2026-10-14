#include "Configuration.h"
#include <cxxopts.hpp>
#include <thread>

Runner::Configuration Runner::parseConfig(cxxopts::ParseResult & parsedArguments) {
    Configuration result;
    result.oldEnginePath = parsedArguments.as_optional<std::string>("old").value_or("");
    result.newEnginePath = parsedArguments.as_optional<std::string>("new").value_or("");
    result.openingBook = parsedArguments.as_optional<std::string>("opening_suite").value_or("book.spell");
    result.outputPath = parsedArguments.as_optional<std::string>("output").value_or("SPRT_REPORT.json");
    result.jobs = parsedArguments.as_optional<int>("jobs").value_or(std::thread::hardware_concurrency());
    result.alpha = parsedArguments.as_optional<double>("alpha").value_or(0.05);
    result.beta = parsedArguments.as_optional<double>("beta").value_or(0.05);
    result.baseTime =   1000 * parsedArguments.as_optional<double>("time").value_or(10.f);
    result.increment =  1000 * parsedArguments.as_optional<double>("increment").value_or(0.1f);
    result.moveTime =   1000 * parsedArguments.as_optional<double>("move_time").value_or(0.f);
    result.latencyMargin = parsedArguments.as_optional<int>("margin").value_or(0);

    int elo = parsedArguments.as_optional<int>("elo").value_or(10);
    bool regression_bound = parsedArguments.contains("regression");

    if (!regression_bound) {
        result.elo0 = 0;
        result.elo1 = elo;
    } else {
        result.elo0 = -elo;
        result.elo1 = 0;
    }

    return result;
};

std::optional<std::string> Runner::validateConfig(const Configuration & config) {
    if(config.oldEnginePath.empty())
        return std::optional<std::string>("No base engine provided");

    if(config.newEnginePath.empty())
        return std::optional<std::string>("No new engine provided");

    if(config.outputPath.empty())
        return std::optional<std::string>("No output path provided");

    if (config.elo0 >= config.elo1)
        return std::optional<std::string>("elo1 is not less than elo0");

    if (config.alpha < 0 || config.alpha > 1)
        return std::optional<std::string>("alpha parameter not in (0, 1)");

    if (config.beta < 0 || config.beta > 1)
        return std::optional<std::string>("beta parameter not in (0, 1)");
    
    if (config.baseTime == 0 && config.moveTime == 0)
        return std::optional<std::string>("base time and move_time are both zero");

    if(config.baseTime < 0)
        return std::optional<std::string>("base time is less than 0");

    if(config.increment < 0)
        return std::optional<std::string>("increment is less than 0");

    if(config.moveTime < 0)
        return std::optional<std::string>("move time is less than 0");

    return std::nullopt;
};