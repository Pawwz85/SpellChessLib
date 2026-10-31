#include <SpellChess/DocumentAnnotator.h>
#include <SpellChess/PGN4Parser.h>
#include <SpellChess/SpellChessDocument.h>
#include <SpellChess/SpellChessUciLoaderPort.h>

#include <UCILoader/EngineConnection.h>
#include <UCILoader/Logger.h>
#include <UCILoader/ProcessWrapper.h>

#include <cxxopts.hpp>

#include <cmath>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>

namespace {

std::string readFile(const std::string& path) {
    std::ifstream input(path, std::ios::binary);
    if (!input) {
        throw std::runtime_error("cannot open input file: " + path);
    }

    std::ostringstream contents;
    contents << input.rdbuf();
    if (!input.good() && !input.eof()) {
        throw std::runtime_error("cannot read input file: " + path);
    }
    return contents.str();
}

std::string csvEscape(const std::string& value) {
    if (value.find_first_of(",\"\r\n") == std::string::npos) {
        return value;
    }

    std::string escaped = "\"";
    for (const char character : value) {
        if (character == '\"') {
            escaped += "\"\"";
        } else {
            escaped += character;
        }
    }
    escaped += '\"';
    return escaped;
}

double whiteWinProbability(const SpellChessLib::PositionInsights& insights) {
    const auto score = insights.positionScore.toPov(SpellChessLib::White).getScore();
    if (score.getUnit() == UCILoader::UciScore::Mate) {
        return score.getValue() >= 0 ? 1.0 : 0.0;
    }

    const double alpha = -std::log(10.0) / 400.0;
    return 1.0 / (1.0 + std::exp(static_cast<double>(score.getValue()) * alpha));
}

void writeRecord(std::ostream& output, const SpellChessLib::GameState& state,
                 const SpellChessLib::PositionInsights& insights) {
    const auto sideToMove = state.getSideToMove();
    const char side = sideToMove == SpellChessLib::White ? 'w' :
                      sideToMove == SpellChessLib::Black ? 'b' : '\0';
    if (side == '\0') {
        throw std::runtime_error("position has no side to move");
    }

    const std::string fen = SpellChessLib::StateFormatter(state).toFen();
    output << csvEscape(fen) << ',' << side << ','
           << std::setprecision(17) << whiteWinProbability(insights) << '\n';
    if (!output) {
        throw std::runtime_error("error writing CSV output");
    }
}

} // namespace

int main(int argc, char** argv) {
    UCILoader::EngineInstance<SpellChessLib::SpellChessMove>* engine = nullptr;

    try {
        cxxopts::Options options("pgn4_position_insights_exporter",
                                 "Export White-POV position insights for a PGN4 main line");
        options.add_options()
            ("e,engine", "SpellChess UCI engine executable", cxxopts::value<std::string>())
            ("d,depth", "Fixed engine search depth", cxxopts::value<unsigned int>())
            ("o,output", "CSV output path (stdout by default)", cxxopts::value<std::string>())
            ("engine-log", "UCI engine communication log path", cxxopts::value<std::string>()->default_value("engine.log"))
            ("input", "PGN4 input file", cxxopts::value<std::string>());
        options.parse_positional({"input"});

        const auto parsed = options.parse(argc, argv);
        if (!parsed.count("input") || !parsed.count("engine") || !parsed.count("depth")) {
            std::cerr << options.help() << '\n';
            return EXIT_FAILURE;
        }

        const auto inputPath = parsed["input"].as<std::string>();
        const auto enginePath = std::filesystem::absolute(parsed["engine"].as<std::string>());
        const auto depth = parsed["depth"].as<unsigned int>();
        if (depth == 0) {
            throw std::runtime_error("search depth must be greater than zero");
        }

        auto annotations = SpellChessLib::PGN4Parser::parsePgn4(readFile(inputPath));
        auto document = std::make_shared<SpellChessLib::SpellChessDocument>(annotations);

        std::ofstream fileOutput;
        std::ostream* output = &std::cout;
        if (parsed.count("output")) {
            fileOutput.open(parsed["output"].as<std::string>(), std::ios::trunc);
            if (!fileOutput) {
                throw std::runtime_error("cannot open output file: " + parsed["output"].as<std::string>());
            }
            output = &fileOutput;
        }

        *output << "position_fen,side_to_move,white_win_probability\n";
        if (!*output) {
            throw std::runtime_error("error writing CSV header");
        }

        auto* process = UCILoader::openProcess({enginePath.string()}, enginePath.parent_path().string());
        const auto logPath = parsed["engine-log"].as<std::string>();
        engine = SpellChessLib::SpellChessEngineInstanceBuilder->build(
            process,
            UCILoader::Loggers::toFile(logPath) | UCILoader::LoggerTraits::Pretty | UCILoader::LoggerTraits::Timestamp);
        auto engineInstance = std::shared_ptr<UCILoader::EngineInstance<SpellChessLib::SpellChessMove>>(engine);
        engine = nullptr;
        engineInstance->sync();
        if (!engineInstance->receivedUCIOK()) {
            throw std::runtime_error("engine did not send uciok");
        }

        SpellChessLib::DocumentAnnotator annotator(
            SpellChessLib::makeFixedDetphAnalizer(
                engineInstance, depth),
            SpellChessLib::SimplePositionClassifier,
            false);
        annotator(document);

        auto current = document->getRoot();
        while (current.lock()) {
            const auto insights = annotator.lookupCache(current);
            if (!insights) {
                throw std::runtime_error("annotator did not cache a main-line position");
            }
            writeRecord(*output, document->getGameState(current), *insights);
            current = document->getNextNode(current);
        }

        delete engine;
        engine = nullptr;
        return EXIT_SUCCESS;
    } catch (const std::exception& error) {
        std::cerr << "PGN4 position insights export failed: " << error.what() << '\n';
        delete engine;
        return EXIT_FAILURE;
    } catch (...) {
        std::cerr << "PGN4 position insights export failed with an unknown error\n";
        delete engine;
        return EXIT_FAILURE;
    }
}
