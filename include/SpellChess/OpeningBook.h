#pragma once 

#include "Board.h"

#include <optional>
#include <memory>
#include <filesystem>
#include <stdexcept>

namespace SpellChessLib {

    class OpeningBook {
        public:
        virtual ~OpeningBook() = default;
        virtual bool containsPosition(const GameState & state) const = 0;
        virtual std::optional<SpellChessMove> pickRandomMove(const GameState & state) const = 0;
        virtual long count() const = 0;
    };

    class OpeningBookLoader {
        public:
        virtual ~OpeningBookLoader() = default;
        virtual std::shared_ptr<OpeningBook> loadOpeningBook(const std::filesystem::path & path) const = 0;

         class BookNotFoundError : public std::exception {
            std::string msg;

            const char * what() const noexcept override {
                return msg.c_str();
            };
            public:
            BookNotFoundError(const std::filesystem::path & path) : msg(std::string("Couldn't open opening book:" + path.string())) {};

        };
    };

}