#pragma once

#include "PGN4Supplier.h"

#include <SpellChess/SpellChessDocument.h>

#include <string>
#include <memory>
#include <vector>
#include <ostream>

namespace Extractor {

    // Clas
    class PGN4Processor {
        std::unique_ptr<PGN4Supplier> pgn4Supplier;
        const std::vector<std::string> featuresToExtract;

        void extractRecord(std::ostream & out, const SpellChessLib::SpellChessDocument & doc, SpellChessLib::SpellChessDocument::Handle nodeHandle);
        void extractCSV(std::ostream & out, const SpellChessLib::SpellChessDocument & doc);

        public:
        PGN4Processor(const std::vector<std::string> & featuresToExtract) : featuresToExtract(featuresToExtract) {};
        void extractCSV(std::unique_ptr<PGN4Supplier> && supplier, std::ostream & out);
    }; 

}