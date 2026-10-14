#include <SpellChess/Board.h>
#include <SpellChess/SpellChessUciLoaderPort.h>

#include <UCILoader/ProcessWrapper.h>
#include <UCILoader/EngineConnection.h>
#include <UCILoader/UCI.h>

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <cmath>
#include <cstdlib>
#include <exception>
#include <filesystem>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace {

struct SearchScore {
    std::atomic<int32_t> value{0};
    std::atomic<bool> mate{false};
    std::atomic<bool> received{false};

    void reset() {
        value = 0;
        mate = false;
        received = false;
    }
};

std::pair<std::string, std::string> parseInputRecord(const std::string& line) {
    const auto separator = line.find(',');
    if (separator == std::string::npos || separator == 0 || separator + 1 == line.size()) {
        throw std::runtime_error("expected <key>,<fen>");
    }
    return {line.substr(0, separator), line.substr(separator + 1)};
}

int32_t normalizedScore(const SearchScore& score) {
    if (!score.mate.load()) {
        return score.value.load();
    }

    const int32_t distance = score.value.load();
    const int32_t magnitude = 100000 - std::min<int32_t>(std::abs(distance), 1000);
    return distance < 0 ? -magnitude : magnitude;
}

} // namespace

int main(int argc, char** argv) {
    if (argc != 6) {
        std::cerr << "Usage: transition_score_exporter <engine> <input.csv> <output.csv> <nodes> <timeout_ms>\n";
        return EXIT_FAILURE;
    }

    const std::filesystem::path enginePath = std::filesystem::absolute(argv[1]);
    const std::filesystem::path inputPath = std::filesystem::absolute(argv[2]);
    const std::filesystem::path outputPath = std::filesystem::absolute(argv[3]);
    const auto nodes = static_cast<unsigned int>(std::stoul(argv[4]));
    const auto timeout = std::chrono::milliseconds(std::stoul(argv[5]));

    std::ifstream input(inputPath);
    if (!input) {
        std::cerr << "Cannot open input file: " << inputPath << '\n';
        return EXIT_FAILURE;
    }

    std::ofstream output(outputPath, std::ios::trunc);
    if (!output) {
        std::cerr << "Cannot open output file: " << outputPath << '\n';
        return EXIT_FAILURE;
    }
    output << "key,score_cp,mate,status\n";

    UCILoader::ProcessWrapper* process = nullptr;
    UCILoader::EngineInstance<SpellChessLib::SpellChessMove>* engine = nullptr;

    try {
        process = UCILoader::openProcess({enginePath.string()}, enginePath.parent_path().string());
        engine = SpellChessLib::SpellChessEngineInstanceBuilder->build(process);
        // Magic-table initialization is unrelated to per-position search cost
        // and can legitimately exceed the small bounded-search timeout.
        engine->sync(std::chrono::seconds(30));

        if (!engine->receivedUCIOK()) {
            throw std::runtime_error("engine did not send uciok");
        }

        SearchScore latestScore;
        auto infoReceiver = engine->addInfoHandler([&latestScore](const UCILoader::Info<SpellChessLib::SpellChessMove>& info) {
            if (info.getType() != UCILoader::Score) {
                return;
            }
            const auto score = info.getAsScore();
            latestScore.value = score.getValue();
            latestScore.mate = score.getUnit() == UCILoader::UciScore::Mate;
            latestScore.received = true;
        });

        SpellChessLib::FenParser parser;
        UCILoader::GoParamsBuilder<SpellChessLib::SpellChessMove> paramsBuilder;
        const auto params = paramsBuilder.withNodes(nodes).build();
        std::string line;
        std::size_t completed = 0;

        while (std::getline(input, line)) {
            if (line.empty()) {
                continue;
            }

            const auto [key, fen] = parseInputRecord(line);
            try {
                const auto state = parser.fromSpellChessFen(fen);
                latestScore.reset();

                auto search = engine->search(params, SpellChessLib::StateFormatter(state));
                search->waitFor(timeout);
                if (search->getStatus() != UCILoader::ResultReady) {
                    search->stop();
                    output << key << ",0,0,timeout\n";
                    output.flush();
                    std::cerr << "Search timed out for key " << key << "; ending this batch cleanly\n";
                    break;
                }
                if (!latestScore.received.load()) {
                    output << key << ",0,0,no_score\n";
                    output.flush();
                    std::cerr << "Search returned no score for key " << key << "; ending this batch cleanly\n";
                    break;
                }

                int32_t score = normalizedScore(latestScore);
                if (state.getSideToMove() == SpellChessLib::Black) {
                    score = -score;
                }
                output << key << ',' << score << ',' << (latestScore.mate.load() ? 1 : 0) << ",ok\n";
                output.flush();
            }
            catch (const std::exception& error) {
                output << key << ",0,0,search_error\n";
                output.flush();
                std::cerr << "Search failed for key " << key << ": " << error.what() << "; ending this batch cleanly\n";
                break;
            }
            catch (...) {
                output << key << ",0,0,search_error\n";
                output.flush();
                std::cerr << "Search failed for key " << key << "; ending this batch cleanly\n";
                break;
            }

            ++completed;
            if (completed % 250 == 0) {
                std::cerr << "Evaluated " << completed << " positions\n";
            }
        }

        delete engine;
        engine = nullptr;
        std::cerr << "Evaluated " << completed << " positions in total\n";
        return EXIT_SUCCESS;
    }
    catch (const std::exception& error) {
        std::cerr << "Transition score export failed: " << error.what() << '\n';
        delete engine;
        return EXIT_FAILURE;
    }
}
