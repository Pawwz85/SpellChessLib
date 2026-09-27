#include "Feature.h"
#include "PGN4Processor.h"
#include "PGN4Supplier.h"

#include <cxxopts.hpp>

#include <fstream>
#include <iostream>

int main(int argc, char ** argv) {
    try {

        Extractor::registerFeature("pgn_counter", [](const SpellChessLib::SpellChessDocument &, SpellChessLib::SpellChessDocument::Handle){
            return Extractor::PGN4Processor::counterFeature();
        });

        cxxopts::Options options("Pgn4Extract", "Extract PGN4 positions to CSV");
        options.add_options()
            ("r,recursive", "Inspect nested directories", cxxopts::value<bool>()->default_value("false"))
            ("o,output", "Write CSV to this file", cxxopts::value<std::string>())
            ("path", "PGN4 file or directory", cxxopts::value<std::string>())
            ("pattern", "Comma-separated feature pattern", cxxopts::value<std::string>());
        options.parse_positional({"path", "pattern"});
        const auto parsed = options.parse(argc, argv);
        if (!parsed.count("path") || !parsed.count("pattern")) {
            std::cerr << options.help() << '\n';
            return EXIT_FAILURE;
        }

        const auto features = Extractor::parseFeaturePattern(parsed["pattern"].as<std::string>());
        auto supplier = std::make_unique<Extractor::FilesystemPGN4Supplier>(
            parsed["path"].as<std::string>(), parsed["recursive"].as<bool>());

        if (parsed.count("output")) {
            std::ofstream output(parsed["output"].as<std::string>(), std::ios::binary);
            if (!output) {
                std::cerr << "cannot open output file: " << parsed["output"].as<std::string>() << '\n';
                return EXIT_FAILURE;
            }
            Extractor::PGN4Processor(features).extractCSV(std::move(supplier), output);
            if (!output) {
                std::cerr << "error writing output file\n";
                return EXIT_FAILURE;
            }
        } else {
            Extractor::PGN4Processor(features).extractCSV(std::move(supplier), std::cout);
        }
        return EXIT_SUCCESS;
    } catch (const std::exception & error) {
        std::cerr << error.what() << '\n';
        return EXIT_FAILURE;
    }
}