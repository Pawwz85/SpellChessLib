#pragma once

#include <vector>
#include <string>
#include <string_view>
#include "GameAnnotations.h"
#include "Board.h"

#include <stdexcept>

namespace SpellChessLib {

    namespace PGN4Parser {

        enum class TokenType {
            square_bracket_open,  // [
            square_bracket_close, // ]
            curly_bracket_open,   // {
            curly_bracket_close,  // }
            bracket_open,         // (  
            bracket_close,        // )
            dot,                  // .
            quote,                // "      
            string_literal,       // "string enclosed in quotes"  
            comment,              // {string enclosed in curly brackets}  
            number,               // 2137
            word                  // neither of the above - probably move or tag name    
        };

        struct Token {
            TokenType type;
            std::string value;
        };
        
        extern std::vector<Token> tokenize(std::string_view source);
        extern Annotation::TagSection parseTags(const std::vector<Token> & tokens);
        extern SpellChessMove parseMove(const std::string & token, const GameState & context);
        extern Annotation::AnnotatedSequence parseMoveList(const std::vector<Token> & tokens, const GameState & root);
        extern SpellChessLib::GameState parseFen4(const std::string_view & fen4);
        extern Annotation::GameAnnotations parsePgn4(std::string_view source);
    };

    class InvalidPGN4 : public std::exception {
      std::string msg;
      
      public:
      InvalidPGN4(const std::string & msg) : msg(msg) {};

      const char * what() const noexcept override {
          return msg.c_str();
      };

    };

};