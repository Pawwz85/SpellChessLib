#include <SpellChess/PGN4Exporter.h>
#include <SpellChess/RandomPlayer.h>
#include <SpellChess/GameRunner.h>
#include <SpellChess/SpellChessDocumentHTMLRenderer.h>


#include <memory>
#include <fstream>
#include <iostream>
#include <cxxopts.hpp>


class RandomPlayerFactory : public SpellChessLib::Factory<SpellChessLib::Player> {
    public:
     std::unique_ptr<SpellChessLib::Player> makeObject() {
        return std::unique_ptr<SpellChessLib::RandomPlayer>(new SpellChessLib::RandomPlayer);
     };
};

SpellChessLib::Scheduler::ResourcePointer<SpellChessLib::Player> makeRandomPlayer(){
    return SpellChessLib::Scheduler::makeResource<SpellChessLib::Player>(std::make_shared<RandomPlayerFactory>());
};

int main(int argc, char ** argv) {

    cxxopts::Options options("Pgn4Generator", "A test tool that generates pgn4s from random play");

    options.add_options()
        ("position", "Starting position of a game", cxxopts::value<std::string>())
        ("output", "Output file",  cxxopts::value<std::string>())
    ;

    options.parse_positional({"position", "output"});

    auto p = options.parse(argc, argv);

    std::string fen = p.as_optional<std::string>("position").value_or(SpellChessLib::SPELL_START_POS);
    std::string output = p.as_optional<std::string>("output").value_or("output.pgn4");

    SpellChessLib::SpellChessFenValidator validator;

    if(!validator.isValidFen(fen.c_str())){
        std::cerr << "Invalid fen";
        return EXIT_FAILURE;
    }

    auto timerService = std::make_shared<SpellChessLib::TimerService>();

    auto config = SpellChessLib::GameRunnerConfigurationBuilder().withPosition(fen).withInfiniteTime().build();

    SpellChessLib::GameRunner runner(timerService, config, makeRandomPlayer(), makeRandomPlayer());
    runner.run();

    std::ofstream out;
    out.open(output);
    SpellChessLib::PGN4Exporter exporter(&out);
    exporter.exportPGN4(runner.getGame().getRoot(), runner.getGame().getDocument()->toAnnotations());

    std::ofstream htmlOut;
    htmlOut.open("test.html");

    auto html = SpellChessLib::toHtml(*runner.getGame().getDocument());
    htmlOut << html->toText();
    
    out.close();
    htmlOut.close();
};