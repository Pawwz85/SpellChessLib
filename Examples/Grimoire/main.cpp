#include "Configuration.h"
#include "GameNode.h"
#include "OpeningBookEntry.h"
#include <cxxopts.hpp>

#include <iostream>

int main(int argc, char ** argv) {

    cxxopts::Options option("Grimoire", "A spell chess opening scroll generator");

    option.add_options()
        ("d,depth", "Depth of engine search", cxxopts::value<unsigned int>())
        ("T,time_per_pos", "Time of engine search in milliseconds per position", cxxopts::value<unsigned int>())
        ("t,timeout", "Timeout value of engine search in milliseconds", cxxopts::value<unsigned int>()->default_value("10000"))
        ("e,engine", "Engine used to generate opening book", cxxopts::value<std::string>())
        ("o,output", "output file for an opening book", cxxopts::value<std::string>())
        ("b,bounds", "Describe the eval range in which generates nodes are contained [-B; +B]", cxxopts::value<unsigned int>()->default_value("125"))
        ("M,max", "Describe maximal length of an opening line", cxxopts::value<unsigned int>())
        ("N,nodes", "The minimum amount of non-leaf game nodes to visit before outputting opening book", cxxopts::value<unsigned int>()->default_value("100"))
    ;

    option.parse_positional({"engine", "output"});

    auto parsingResult = option.parse(argc, argv);

    Grimoire::Configuration * config = Grimoire::parseConfiguration(parsingResult);

    if (config == nullptr) {
        std::cerr << "Failed to create grimoire configuration" << std::endl;
        return EXIT_FAILURE;
    };

    UCILoader::ProcessWrapper* engineProcess;
    
    try {
        engineProcess = UCILoader::openProcess({config->pathToEngine}, "/");
    } catch (const UCILoader::CanNotOpenProcessException & e){
        std::cerr << "Failed to open " << config->pathToEngine << std::endl;
        delete config;
        return EXIT_FAILURE;
    };

    auto engineInstance = SpellChessLib::SpellChessEngineInstanceBuilder->build(engineProcess);

    try {
        engineInstance->sync();
    } catch (const UCILoader::EngineInstance<SpellChessLib::SpellChessMove>::TimeoutException & e) {
        std::cerr << "Propably not a chess engine " << config->pathToEngine << std::endl;
        delete config;
        delete engineInstance;
        return EXIT_FAILURE;
    };

    if (!engineInstance->receivedUCIOK()) {
        std::cerr << "Engine did not respond with uciok: " << config->pathToEngine << std::endl;
        delete config;
        delete engineInstance;
        return EXIT_FAILURE;
    }

    if (!engineInstance->options.contains("GrimoireMode")) {
        std::cerr << engineInstance->getName() << " doesn't support GrimoireMode" << std::endl;
        delete config;
        delete engineInstance;
        return EXIT_FAILURE;
    };

    try {
        engineInstance->options["GrimoireMode"] = true;
    } catch(const UCILoader::EngineOptionProxy::WrongTypeError & e) {
        std::cerr << "GrimoireMode option is presend but it isn't check" << std::endl;
        delete config;
        delete engineInstance;
        return EXIT_FAILURE;
    };

    if(engineInstance->options.contains("GrimoireBounds")) {
        try {
            engineInstance->options["GrimoireBounds"] = int(config->evalBounds);
        } catch(const UCILoader::EngineOptionProxy::WrongTypeError & e) {
        std::cerr << "(Warning) Failed to set bounds" << std::endl;
        }
    } else {
        std::cerr << "(Warning) Engine doesn't support bounds" << std::endl;
    }


    Grimoire::GameNode * root = Grimoire::createRoot();
    std::cout << "Started building game tree..." << std::endl;
    Grimoire::build_game_tree(root, config, engineInstance);
    auto book = Grimoire::gather_entries(root);
    std::cout << "Gathered " << book.size() << " entries" <<std::endl;
    Grimoire::write_to_file(book, config->pathToOutput);
    Grimoire::deallocate_game_tree(root);


    delete config;
    delete engineInstance;
    return EXIT_SUCCESS;
};