#include "PGN4Processor.h"
#include "Feature.h"
#include <iostream>
#include <SpellChess/PGN4Parser.h>

using namespace Extractor;

void PGN4Processor::extractRecord(std::ostream & out, const SpellChessLib::SpellChessDocument & doc, SpellChessLib::SpellChessDocument::Handle nodeHandle){
    for (size_t i = 0; i < featuresToExtract.size(); ++i) {
        if (i > 0) out << ",";
        out << escapeCsvField(extract(featuresToExtract[i], doc, nodeHandle)->toString());
    }
    out << "\n";
}

void PGN4Processor::extractCSV(std::ostream & out, const SpellChessLib::SpellChessDocument & doc) {

    auto currentNodeHandle = doc.getRoot();

    while (currentNodeHandle.lock() != nullptr) {
        extractRecord(out, doc, currentNodeHandle);
        currentNodeHandle = doc.getNextNode(currentNodeHandle);
    }
}

void PGN4Processor::extractCSV(std::unique_ptr<PGN4Supplier> && supplier, std::ostream & out) {
    // write header
    for (size_t i = 0; i < featuresToExtract.size(); ++i) {
        if (i > 0) out << ",";
        out << escapeCsvField(featuresToExtract[i]);
    }
    out << "\n";

    while (supplier->hasNext()) {
        try {
             const std::string pgn4 = supplier->getNext();
            auto pgn4Data = SpellChessLib::PGN4Parser::parsePgn4(pgn4);
            SpellChessLib::SpellChessDocument doc(pgn4Data);
            extractCSV(out, doc);
        } catch(const std::exception & e) {
            std::cerr << "Error processing PGN4: " << e.what() << std::endl;
        }
    }
}