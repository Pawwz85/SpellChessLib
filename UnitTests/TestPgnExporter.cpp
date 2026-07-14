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
    

    document->makeMove(document->getTail(), "d7c6");
    document->makeMove(document->getTail(), "a7c6");
    document->makeMove(document->getTail(), "c7c8q");
    document->makeMove(document->getTail(), "c6b8");
    document->makeMove(document->getTail(), "c8b7");

    document->setGameResult(SpellChessLib::Annotation::WhiteWon);

    std::string exported = exportToPGN(document);
    EXPECT_EQ(exported, loadAnswerFromFile("setup_game.pgn"));
};

TEST(PGNExport, export_with_comments) {
    auto document = SpellChessLib::createEmptySpellChessDocument(SpellChessLib::START_POS);
    document->setTag("Date", "2026.07.12");
    
    document->makeMove(document->getTail(), "e2e4");
    document->setComment(document->getTail(), "Best by test");
    document->makeMove(document->getTail(), "e7e5");
    document->setComment(document->getTail(), "Clearly the best response");
    document->makeMove(document->getTail(),  "g1f3");
    document->setComment(document->getTail(), "Attacking the pawn");
    document->makeMove(document->getTail(), "b8c6");
    document->setComment(document->getTail(), "Defending the pawn");
    document->makeMove(document->getTail(),  "f1c4");
    document->setComment(document->getTail(), "Developing the bishop");

    std::string exported = exportToPGN(document);
    EXPECT_EQ(exported, loadAnswerFromFile("comments.pgn"));
};

TEST(PGNExport, export_with_variations) {
    auto document = SpellChessLib::createEmptySpellChessDocument(SpellChessLib::START_POS);
    document->setTag("Date", "2026.07.12");

    auto pos = document->makeMove(document->getRoot(), "e2e4");
    document->makeMove(document->getTail(), "e7e5");
    document->makeMove(document->getTail(), "g1f3");

    auto it = document->makeMove(pos, "d7d5");
    it = document->makeMove(it, "e4d5");
    it = document->makeMove(it, "d8d5");

    it = document->makeMove(pos, "c7c5");
    it = document->makeMove(it, "g1f3");
    it = document->makeMove(it, "d7d6");

    it = document->makeMove(pos, "b8c6");
    it = document->makeMove(it,  "d2d4");
    it = document->makeMove(it, "d7d5");

    std::string exported = exportToPGN(document);
    EXPECT_EQ(exported, loadAnswerFromFile("variations.pgn"));
}; 