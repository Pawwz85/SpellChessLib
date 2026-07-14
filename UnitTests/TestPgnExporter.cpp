#include "pch.h"

#include <SpellChess/SpellChessDocument.h>
#include <SpellChess/PGNExporter.h>

#include <sstream>
#include <fstream>

std::string exportToPGN(std::shared_ptr<SpellChessLib::SpellChessDocument> doc) {
    std::stringstream sstream;
    SpellChessLib::PGNExporter exporter(&sstream);
    exporter.exportPGN(doc->toAnnotations());
    return sstream.str();
};

std::string loadAnswerFromFile(std::string test_case_name) {
    const std::string prefix = "example_pgns/";
    std::fstream file;
    file.open(prefix + test_case_name);
    
    std::stringstream sstream;
    sstream << file.rdbuf();
    return sstream.str();
};

TEST(PGNExport, export_empty) {
    auto document = SpellChessLib::createEmptySpellChessDocument(SpellChessLib::START_POS);
    document->setTag("Date", "2026.07.12");
    std::string exported = exportToPGN(document);

    EXPECT_EQ(exported, loadAnswerFromFile("empty.pgn"));
};

TEST(PGNExport, export_from_custom_fen) {
    auto document = SpellChessLib::createEmptySpellChessDocument("k7/n1PB4/1K6/8/8/8/8/8 w - - 0 50");
    document->setTag("Date", "2026.07.12");
    

    document->makeMove(document->getTail(), SpellChessLib::parseMove("d7c6"));
    document->makeMove(document->getTail(), SpellChessLib::parseMove("a7c6"));
    document->makeMove(document->getTail(), SpellChessLib::parseMove("c7c8q"));
    document->makeMove(document->getTail(), SpellChessLib::parseMove("c6b8"));
    document->makeMove(document->getTail(), SpellChessLib::parseMove("c8b7"));

    document->setGameResult(SpellChessLib::Annotation::WhiteWon);

    std::string exported = exportToPGN(document);
    EXPECT_EQ(exported, loadAnswerFromFile("setup_game.pgn"));

};
