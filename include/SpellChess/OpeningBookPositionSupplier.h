#pragma once

#include "PositionSupplier.h"
#include "OpeningBook.h"
#include "Board.h"
#include <memory>
#include <string>

namespace SpellChessLib {

    class OpeningBookPositionSupplier : public PositionSupplier{
        std::shared_ptr<OpeningBook> book;
		GameState startPos;
        public:
        OpeningBookPositionSupplier(const std::string startPosFen, std::shared_ptr<OpeningBook> book) : book(book) {
            FenParser parser;
			startPos = parser.fromSpellChessFen(startPosFen);
        };
		GameState produce() override;

    };

};