#include "TestCase.h"

#include <UCILoader/EngineConnection.h>

#include <SpellChess/SpellChessUciLoaderPort.h>
#include <SpellChess/Board.h>
#include <SpellChess/StringTokenizer.h>

#include <iostream>
#include <stdexcept>
#include <optional>

#include <cxxopts.hpp>

using namespace MoveGeneratorTester;

MoveContainer queryEngineForMoves(UCILoader::EngineInstance<SpellChessLib::SpellChessMove> & engine, const std::string & fen) {
    SpellChessLib::GameState board = SpellChessLib::FenParser().fromSpellChessFen(fen);

    MoveContainer result;

    auto receiver = engine.addInfoHandler([&result](const UCILoader::Info<SpellChessLib::SpellChessMove> & info){
        
        if (info.getType() != UCILoader::InfoType::InfoString)
            return;

        std::string msg = info.getStringValue();
        auto parts = SpellChessLib::Parsing::tokenizeStringBySpace(msg);       
        
        if (parts.size() < 4)
            return;

        if (parts[0] != "MoveGen" || parts[1] != "Entry")
            return;

        try {
            SpellChessLib::SpellChessMove base = SpellChessLib::parseMove(parts[2]);

            if (parts[3] != "addable" && parts[3] != "fixed")
                throw std::invalid_argument(std::string("Unexpected spell policy: ") + parts[3]);

            IceCream::SpellPolicy policy = (parts[3] == "addable") ? 
                        IceCream::Addable : IceCream::Fixed;

            result.push_back({base, policy});
        } catch (const std::exception & exception) {
            std::cerr << exception.what() << std::endl;
        }
        

    });

    engine.sync();

    engine.position(SpellChessLib::StateFormatter(board));
    engine.options["MoveGen"].click();

    engine.sync();
    receiver->unlinkAll();

    return result;
};

int main(int argc, char ** argv) {
    
    cxxopts::Options option("MoveGeneratorTester", "A move generator tester for Icecream engine");

    option.add_options()
        ("e,engine", "Icecream executable", cxxopts::value<std::string>())
    ;

    option.parse_positional({"engine"});

    auto parsingResult = option.parse(argc, argv);

    auto testCases = MoveGeneratorTester::loadTests();

    const size_t total = testCases.size();
    size_t failed = 0;

    UCILoader::ProcessWrapper* process;
    
    try {
        process = UCILoader::openProcess({parsingResult["engine"].as<std::string>()}, ".");
    } catch (const UCILoader::CanNotOpenProcessException & e) {
        std::cerr << e.what();
        return 1;
    }
    

    auto engine = std::unique_ptr<UCILoader::EngineInstance<SpellChessLib::SpellChessMove>>
        (SpellChessLib::SpellChessEngineInstanceBuilder->build(process));

    engine->sync();

    for (const auto & test : testCases) {
        auto engine_moves = queryEngineForMoves(*engine, test.positionFen);
        std::optional<std::string> error = MoveGeneratorTester::checkTestCase(engine_moves, test);
        
        if (error.has_value()) {
            ++failed;
            std::cout << "Test failed for the following position: " << test.positionFen << "\n";
            std::cout << error.value() << "\n";
        }
    }

    std::cout << "Program finished with " << failed << " failed positions";

    return 0;
};