#include <SpellChess/PGN4Parser.h>
#include <SpellChess/StringTokenizer.h> // for Parsing::parseSquareIndex
#include <vector>
#include <tuple>
#include <sstream>
#include <array>
#include <map>
#include <iostream> // for cerr
#include <nlohmann/json.hpp> // we need to parse some inline json in PGN4

namespace SpellChessLib {

    namespace PGN4Parser {

        bool isSpecialCharacter(char c) {
            static const std::string specialCharacters = "{}[]().\"";
            return specialCharacters.find(c) != std::string::npos;
        }

        bool isWhiteSpace(char c) {
            static const std::string specialCharacters = " \t\r\n";
            return specialCharacters.find(c) != std::string::npos;
        }

        PGN4Parser::TokenType typeOfSpecialCharacter(char c) {
            if (c == '[') 
                return TokenType::square_bracket_open;

            if (c == '{') 
                return TokenType::curly_bracket_open;
            
            if (c == '(')
                return TokenType::bracket_open;

            if (c == ']') 
                return TokenType::square_bracket_close;

            if (c == '}') 
                return TokenType::curly_bracket_close;
            
            if (c == ')')
                return TokenType::bracket_close;

            if (c == '.') 
                return TokenType::dot;

            if (c == '"')
                return TokenType::quote;

            throw std::logic_error(std::string("typeOfSpecialCharacter() character was called on non special character: ") + c);
        };

        std::vector<Token> tokenize(std::string_view source) {

            enum StateMachineState {
                NoContext,
                NumericLiteral,
                StringLiteral,
                Comment,
                Word
            };

            StateMachineState currentState = NoContext;
            std::string accumulator;
            std::vector<Token> result;

            for (char c : source) {
                
                switch (currentState)
                {
                case Word:
                    
                    if (isWhiteSpace(c)) {
                        currentState = NoContext;
                        result.push_back({TokenType::word, accumulator});
                        accumulator.clear();
                        continue;
                    }

                    if (isSpecialCharacter(c)) {
                        currentState = NoContext;
                        result.push_back({TokenType::word, accumulator});
                        accumulator.clear();
                        goto NO_CONTEXT;
                    };

                    accumulator.push_back(c);

                    break;
                case NumericLiteral:
                    if (c >= '0' && c <= '9') {
                        accumulator.push_back(c);
                        continue;
                    }

                    if (isSpecialCharacter(c)) {
                        result.push_back({TokenType::number, accumulator});
                        accumulator.clear();
                        currentState = NoContext;
                        goto NO_CONTEXT;
                    }

                    currentState = Word;
                case NoContext:
                    NO_CONTEXT:
                   
                    if (isSpecialCharacter(c)) {

                        if(c == '{') {
                            currentState = Comment;
                            continue;
                        }
                        
                        if (c == '}') {
                            throw InvalidPGN4("Unexpected character: }");
                        };

                        if (c == '"') {
                            currentState = StringLiteral;
                            continue;
                        }

                        result.push_back({typeOfSpecialCharacter(c), std::string() + c});
                        continue;
                    }

                    if (isWhiteSpace(c)){
                        continue;
                    }

                    if (c >= '0' && c <= '9') {
                        currentState = NumericLiteral;
                        accumulator.push_back(c);
                        continue;
                    }

                    currentState = Word;
                    accumulator.push_back(c);

                    break;
                
                case Comment:

                    if (c == '{')
                        throw InvalidPGN4("'{' character was found inside comment"); 

                    if (c == '}') {
                        currentState = NoContext;
                        result.push_back({TokenType::comment, accumulator});
                        accumulator.clear();
                    } else {
                        accumulator.push_back(c);
                    }
                    break;
                case StringLiteral:
                    if (c == '"') {
                        currentState = NoContext;
                        result.push_back({TokenType::string_literal, accumulator});
                        accumulator.clear();
                    } else {
                        accumulator.push_back(c);
                    }
                    break;
                default:
                    break;

                }

            };


            if(!accumulator.empty()) {
                if (currentState == NumericLiteral)
                    result.push_back({TokenType::number, accumulator});
                if (currentState == Word)
                    result.push_back({TokenType::word, accumulator});
                if (currentState == StringLiteral)
                    throw InvalidPGN4("Encountered unterminated string literal");
                if (currentState == Comment)
                    throw InvalidPGN4("Encountered unterminated comment");
            };

            return result;
        };

        std::string concatTags(size_t index, size_t count, const std::vector<Token> & tokens) {
            std::string result;
            for(size_t i = index; i != index + count; ++i){
                result += tokens[i].value;
                result.push_back(' ');
            }
            return result;
        }

        Annotation::TagPair parseTagPair(size_t index, const std::vector<Token> & tokens){
            
            if (index + 4 > tokens.size())
                throw InvalidPGN4("Failed to parse tags");

            if (tokens[index].type != TokenType::square_bracket_open ||
                tokens[index + 1].type != TokenType::word            ||
                tokens[index + 2].type != TokenType::string_literal  ||
                tokens[index + 3].type != TokenType::square_bracket_close)
                throw InvalidPGN4("Invalid tag: " + concatTags(index, 4, tokens));
            
            return {
                tokens[index + 1].value,
                tokens[index + 2].value
            };
        }

        Annotation::TagSection parseTags(const std::vector<Token> & tokens) {
            size_t index = 0;
            Annotation::TagSection tags;
            while (index < tokens.size()) {
                if (tokens[index].type == TokenType::square_bracket_open) {
                    tags.push_back(parseTagPair(index, tokens));
                    index += 4;
                } else
                    index++;
            }
            return tags;
        }

        SpellChessLib::SquareIndex parsePgn4Square(const std::string_view & squareToken) {
   
            if (squareToken.size() != 2 && squareToken.size() != 3) {
                std::string token(squareToken);
                throw InvalidPGN4("Illegal square coordinate: " + token);
            }

            int file = squareToken[0] - 'd';
            int row  = squareToken[1] - '0';

            if (squareToken.size() == 3)
                row = 10 * row + squareToken[2] - '0';
            
            row -=4;
            row = 7-row;
            return squareIndexFromCords(file, row);
        }   

        void loadSpellPartofMove (SpellChessMove & out, const std::string & part) {
            out.freezeSquare = out.jumpSquare = SpellChessLib::NoSquare;
            if (part.empty()) 
                return;

            if (part.rfind("freeze@", 0) == 0) {
                auto targetSquare = part.substr(7);
                out.freezeSquare = parsePgn4Square(targetSquare);
                return;
            };

            if (part.rfind("jump@", 0) == 0) {
                   auto targetSquare = part.substr(5);
                out.jumpSquare = parsePgn4Square(targetSquare);
                return;
            };

            throw InvalidPGN4("Invalid spell prefix: " + part);
        }

        void loadBasePartOfMove(SpellChessMove & out, const std::string & part, const GameState & context) {
            out.promoteTo = SpellChessLib::None;
            if (part.rfind("O-O-O", 0) == 0) {
                out.from = Parsing::parseSquareIndex((context.getSideToMove() == White) ? "e1" : "e8");
                out.to   = Parsing::parseSquareIndex((context.getSideToMove() == White) ? "c1" : "c8");
                return;
            }

            if (part.rfind("O-O", 0) == 0) {
                out.from = Parsing::parseSquareIndex((context.getSideToMove() == White) ? "e1" : "e8");
                out.to   = Parsing::parseSquareIndex((context.getSideToMove() == White) ? "g1" : "g8");
                return;
            }

            if (part.size() < 5) 
                throw InvalidPGN4("Invalid move: " + part);
            
            std::string_view move_view;

            if(part[0] >= 'A' && part[0] <= 'Z')
                move_view = part.c_str() + 1; // 'part' is in form Nj4-k6
            else
                move_view = part.c_str();  // 'part' is in form of j4-k6

            size_t sqSepPos = move_view.find_first_of("-x");
            if (sqSepPos == std::string_view::npos)
                throw InvalidPGN4("Invalid move: " + part);

            out.from = parsePgn4Square(move_view.substr(0, sqSepPos));

            size_t toSqEnd = move_view.find_last_of("0123456789");
            size_t toSqStart = sqSepPos + 1;

            if (move_view[toSqStart] >= 'A' && move_view[toSqStart] <= 'Z')
                ++toSqStart;

            if (toSqStart >= toSqEnd)
                throw InvalidPGN4("Invalid move: " + part);

            out.to   = parsePgn4Square(move_view.substr(toSqStart, toSqEnd - toSqStart + 1));

            size_t promotionPos = move_view.find_last_of("=");
            if (promotionPos != std::string_view::npos && promotionPos + 1 < move_view.size()){
                char targetChar = move_view[promotionPos + 1];
                switch (targetChar)
                {
                case 'Q':
                    out.promoteTo = Queen;
                    break;
                case 'R':
                    out.promoteTo = Rook;
                    break;
                case 'B':
                    out.promoteTo = Bishop;
                    break;
                case 'N':
                    out.promoteTo = Knight;
                    break;
                default:
                      throw InvalidPGN4("Invalid move: " + part);
                }
            };
        }

        SpellChessMove parseMove(const std::string & token, const GameState & context) {
            std::string spellPart, basePart;
            size_t seperator = token.find('&');

            if (seperator != std::string::npos) {
                spellPart = token.substr(0, seperator);
                basePart = token.substr(seperator + 1);
            } else 
                basePart = token;

            SpellChessMove result;
            loadSpellPartofMove(result, spellPart);
            loadBasePartOfMove(result, basePart, context);
            return result;
        }

        Annotation::AnnotatedSequence parseMoveListPrivate(const std::vector<Token> & tokens, size_t & index, const GameState & root) {
            Annotation::AnnotatedSequence result;
            GameState currentState = root;
            for (;index < tokens.size(); ++index) {
                if (tokens[index].type == TokenType::bracket_close) 
                    return result;
                
                if (tokens[index].type == TokenType::word) {
                    auto move = parseMove(tokens[index].value, currentState);
                    result.appendMove(move);
                    currentState.makeMove(move);
                    continue;
                }

                if (tokens[index].type == TokenType::comment) {
                    result.appendComment(tokens[index].value);
                    continue;
                }

                if (tokens[index].type == TokenType::bracket_open) {
                    ++index; // skip bracket
                    result.appendVariation(parseMoveListPrivate(tokens, index, currentState));
                }
            }

            return result;
        }

        Annotation::AnnotatedSequence parseMoveList(const std::vector<Token> & tokens, const GameState & root) {
            size_t index = 0;

            for (size_t i = 0; i < tokens.size(); ++i)
                if (tokens[i].type == TokenType::square_bracket_close)
                    index = i + 1;

            if (index == tokens.size())
                return {};

            auto result = parseMoveListPrivate(tokens, index, root);
            if (index != tokens.size())
                throw InvalidPGN4("Unexpected token: )");
            return result;
        }

        PlayerColor parseSideToMoveFen4Part(const std::string_view & part) {
            if (part.empty())
                throw InvalidPGN4("Invalid StartFen4: side to move was \"\"");

            char c = part[0];

            if (c == 'R')
                return SpellChessLib::White;
            if (c == 'Y')
                return SpellChessLib::Black;

            throw InvalidPGN4("Invalid StartFen4: side to move was " + std::string(part));
        };

        std::tuple<bool, bool> parseCastlingPart(const std::string_view & part) {
            if (part.size() != 7 || 
                part[1] != ',' ||
                part[3] != ',' ||
                part[5] != ',')
                throw InvalidPGN4("Invalid StartFen4: invalid castling rights: " + std::string(part));
            
            bool canWhiteCastle = part[0] != '0';
            bool canBlackCastle = part[4] != '0';
            return std::make_tuple(canWhiteCastle, canBlackCastle);
        }

        std::array<bool, 4> concatCastlingRights(const std::tuple<bool, bool> & kingside, const std::tuple<bool, bool> & queenside) {
            return {
                std::get<0>(kingside),
                std::get<0>(queenside),
                std::get<1>(kingside),
                std::get<1>(queenside),
            };
        };

        int parseHalfMoveCounterPart(const std::string_view & part) {
            std::stringstream ss{std::string(part)};
            int result;
            ss >> result;

            if (ss.bad()) 
                throw InvalidPGN4("Invalid StartFen4: half move counter must be an integer but was: " +  std::string(part));

            return result;
        }

        inline PieceType tryParsingPieceType(char c) {
            static const std::map<char, PieceType> characterToTypeTable = {
                {'P', Pawn},
                {'R', Rook},
                {'B', Bishop},
                {'N', Knight},
                {'Q', Queen},
                {'K', King}
            };

            auto it = characterToTypeTable.find(c);
            if (it != characterToTypeTable.end())
                return it->second;
            throw InvalidPGN4("Invalid StartFen4: Invalid piece token " + std::string({c, 0}));
        }

        std::array<BoardSquare, 64> parseBoardPart(const std::string_view & part) {
            std::vector<std::string_view> rows = Parsing::splitString(part, '/');
            
            std::array<BoardSquare, 64> result;

            for (size_t i = 0; i < 64; ++i)
                result[i].piece = {None, Nobody};

            if (rows.size() != 14)
                throw InvalidPGN4("Invalid StartFen4: invalid piece list");

            char squareTemp[3] = {0, 0, 0};

            for (size_t i = 3; i < 11; ++i) {
                int j = 3;
                std::vector<std::string_view> chunks = Parsing::splitString(rows[i], ',');

                for (auto chunk : chunks) {
                    if (chunk[0] == 'x')
                        continue;
                    
                    if (chunk[0] >= '1' && chunk[0] <= '8') {
                        j+= chunk[0] - '0';
                        continue;
                    }

                    if (chunk[0] != 'r' && chunk[0] != 'y')
                        throw InvalidPGN4("Invalid StartFen4: invalid piece list");

                    squareTemp[0] = 'a' + j - 3;
                    squareTemp[1] = '8' - i + 3;
                    PlayerColor pieceColor = (chunk[0] == 'r') ? White : Black;
                    PieceType pieceType = tryParsingPieceType(chunk[1]);
                    SquareIndex index = Parsing::parseSquareIndex(squareTemp);
                    result[index].piece = {pieceType, pieceColor};
                    ++j;
                };
            };

            return result;
        };

        std::string fixJsonBrackets(const std::string_view & source) {
            std::string result;
            for (char c : source){
                switch(c){
                    case '(':
                        result.push_back('[');
                        break;
                    case ')':
                        result.push_back(']');
                        break;
                    case '\'':
                        result.push_back('"');
                        break;
                    default:
                        result.push_back(c);
                        break;
                }
            }
            return result;
        };

        nlohmann::json parseJsonPart(const std::string_view & part) {
            // step 1. Replace '()' with '[]'
            std::string fixed = fixJsonBrackets(part);

            // step 2. Parse
            try {
                return nlohmann::json::parse(fixed);
            } catch(std::exception e) {
                std::cerr << "Failed to parse json: " << e.what() << std::endl;
                throw InvalidPGN4("Couldn't decode the following: " + std::string(part));
            }
        };

        void loadSpellString(const std::string & string, std::array<SpellCounter, 4>::iterator it) {
            auto parts = Parsing::splitString(string, ',');

            if (parts.size() != 2) 
                throw InvalidPGN4("Invalid StartFen4: invalid spells");

            bool spellSeen[2] = {false, false};
            size_t spellIndex;

            for (auto & part : parts) {
                spellIndex = -1;

                if (part.rfind("jump_", 0) == 0)
                    spellIndex = SpellChessLib::JumpSpell;

                if (part.rfind("freeze_", 0) == 0)
                    spellIndex = SpellChessLib::FreezeSpell;

                if (spellIndex == -1)
                    throw InvalidPGN4("Invalid StartFen4: invalid spells");

                spellSeen[spellIndex] = true;

                if (spellIndex == FreezeSpell) {
                    it[0].spellId = (SpellIdentificator)spellIndex;
                    it[0].usagesLeft = part[9] - '0';
                    it[0].cooldown   = part[7] - '0';
                }

                if (spellIndex == JumpSpell) {
                    it[1].spellId = (SpellIdentificator)spellIndex;
                    it[1].usagesLeft = part[7] - '0';
                    it[1].cooldown   = part[5] - '0';
                }
            }

            if (!spellSeen[0] || !spellSeen[1])
                throw InvalidPGN4("Invalid StartFen4: invalid spells");
        };

        std::array<SpellCounter, 4> extractSpellInfoFromJson (const nlohmann::json & jsonObject) {
            if(!jsonObject.contains("spells"))
                throw InvalidPGN4("Invalid StartFen4: invalid spells");
            
            auto spells = jsonObject["spells"];

            size_t arrSize = spells.size();

            if(!spells.is_array() || arrSize != 4)
                throw InvalidPGN4("Invalid StartFen4: invalid spells");

            std::string whiteSpells = spells[0];
            std::string blackSpells = spells[2];

            std::array<SpellCounter, 4> result = {
                SpellCounter{SpellIdentificator::FreezeSpell, 0, 0},
                SpellCounter{SpellIdentificator::JumpSpell, 0, 0}, 
                SpellCounter{SpellIdentificator::FreezeSpell, 0, 0},
                SpellCounter{SpellIdentificator::JumpSpell, 0, 0} 
            };

            loadSpellString(whiteSpells, result.begin());
            loadSpellString(blackSpells, result.begin() + 2);

            return result;
        };

        #define IGNORE_FEN4_PART(part)

        SpellChessLib::GameState parseFen4(const std::string_view & fen4) { 
            
            // Step 1. Split fen4 by '-' delimeter
            std::vector<std::string_view> parts = Parsing::splitString(fen4, '-');

            if (parts.size() != 8)
                throw InvalidPGN4("Invalid StartFen4 value: " + std::string(fen4));

            PlayerColor sideToMove = parseSideToMoveFen4Part(parts[0]);
            IGNORE_FEN4_PART(parts[1]) // unused player elimination part
            std::tuple<bool, bool> kingsideCastlings = parseCastlingPart(parts[2]);
            std::tuple<bool, bool> queensideCastlings = parseCastlingPart(parts[3]);
            IGNORE_FEN4_PART(parts[4]) // unused player score part
            int halfMoveCounter = parseHalfMoveCounterPart(parts[5]);
            nlohmann::json jsonObject = parseJsonPart(parts[6]);
            std::array<SpellChessLib::BoardSquare, 64> board = parseBoardPart(parts[7]);

            auto castlingRights = concatCastlingRights(kingsideCastlings, queensideCastlings);
            auto spells = extractSpellInfoFromJson(jsonObject); 
            return GameState(board.data(), sideToMove, castlingRights.data(), NoSquare, halfMoveCounter, 1, NoSquare, NoSquare, spells.data());
        };
        
        Annotation::GameAnnotations parsePgn4(std::string_view source) {
            Annotation::GameAnnotations result;
            auto tokens = tokenize(source);
            result.tagSection = parseTags(tokens);
            result.root = parseFen4(result.tagSection["StartFen4"]);
            result.annotatedSequence = parseMoveList(tokens, result.root);
            
            try {
                Annotation::validateAnnotatedSequenceMoveLegality(result.root, result.annotatedSequence);
            } catch (IllegalMoveException e) {
                throw InvalidPGN4(std::string("PGN4 contains an invalid move: ") + e.what());
            }

            std::string gameResult = result.tagSection["Result"];
            
            if (gameResult == "1-0")
                result.terminationReason = Annotation::WhiteWon;
            else if (gameResult == "0-1")
                result.terminationReason = Annotation::BlackWon;
            else if (gameResult == "1/2-1/2")
                result.terminationReason = Annotation::Draw;
            else
                result.terminationReason = Annotation::Unknown;
            return result;
        }

    };

}