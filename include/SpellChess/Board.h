#pragma once
#include <cstdint>
#include <string>
#include <cstring> // for memcpy
#include <vector>

namespace SpellChessLib {

	using SquareIndex=uint8_t;

	const SquareIndex NoSquare = -1;
	const uint8_t SpellCooldown = 2;

	enum PieceType {
		None,
		Pawn,
		Knight,
		Bishop,
		Rook,
		Queen,
		King
	};

	enum PlayerColor {
		Nobody,
		White,
		Black
	};

	extern "C" PlayerColor flipColor(const PlayerColor& c);

	enum CastlingDirection {
		Kingside,
		QueenSide
	};

	enum SpellIdentificator {
		FreezeSpell,
		JumpSpell,
	};

	struct Piece {
		PieceType type;
		PlayerColor owner;

		bool operator == (const Piece& other) const {
			return this->type == other.type && this->owner == other.owner;
		}
	};

	struct BoardSquare {		
		Piece piece; 
		bool jumpable;
		bool frozen;

		inline bool isEmpty() const {
			return !(piece.owner || piece.type);
		}
	};
	
	struct SpellCounter {
		SpellIdentificator spellId;
		uint8_t usagesLeft;
		uint8_t cooldown;

		inline bool canCastNow() const {
			return usagesLeft && cooldown == 0;
		}

		inline void decreaseCooldown() {
			if (cooldown) --cooldown;
		}

		inline void cast() {
			--usagesLeft;
			cooldown = SpellCooldown;
		}

	};

	struct SpellChessMove{
		uint8_t from;
		uint8_t to;
		uint8_t freezeSquare;
		uint8_t jumpSquare;
		PieceType promoteTo;

		SpellChessMove() = default;
		SpellChessMove(const SpellChessMove &) = default;
		SpellChessMove(SpellChessMove &&) = default;
		SpellChessMove(const char * stringValue);
		SpellChessMove(const std::string & stringValue);
		SpellChessMove(uint8_t from, uint8_t to, uint8_t freezeSquare, uint8_t jumpSquare, PieceType promoteTo = SpellChessLib::None);

		inline bool operator==(const SpellChessMove & other) const {
			return  this->from == other.from &&
					this->to  == other.to &&
					this->freezeSquare == other.freezeSquare &&
					this->jumpSquare == other.jumpSquare &&
					this->promoteTo == other.promoteTo;
		};

		inline bool operator!=(const SpellChessMove & other) const {
			return !(*this == other); 
		}

		inline SpellChessMove & operator=(const SpellChessMove &) = default;
	};
	

	extern std::string squareIndexToString(const SquareIndex& index);
	extern std::string spellChessMoveToString(const SpellChessMove & move);
	inline SquareIndex squareIndexFromCords(const int& x, const int& y) { return 8 * y + x; }

#define CORDINATE_BOUNDARY_CHECK(x) (x < 8 && x >= 0)

	extern SpellChessMove parseMove(const char* str);

	extern void validateSpellChessMove(const SpellChessMove& move);
	class InvalidMoveException : public std::exception {
	public:
		const char* what() const noexcept override {
			return "Invalid move passed to a function that expects only valid moves";
		};
	};

	class IllegalMoveException : public std::exception {
		std::string msg;
	public:
		
		IllegalMoveException(const SpellChessMove& move, const std::string & fen): 
			msg(std::string("Move ") + spellChessMoveToString(move) + " is illegal move in the following position: " + fen) {};
		const char* what() const noexcept override;
	};

	class FenValidator {
	public:

		class InvalidFenException : public std::exception {
			std::string msg;
		public:
			InvalidFenException(const std::string& fen) : msg(std::string("The following fen string is invalid for given chess variant: ")+  fen) {};
			const char* what() const noexcept override { return msg.c_str(); };
		};

		virtual bool isValidFen(const char* fen) const = 0;
		virtual void throwIfInvalid(const char* fen) const;
	};

	class SpellChessFenValidator; // declare friend of StandardChessFenValidator
	class StandardChessFenValidator : public FenValidator {
		friend SpellChessFenValidator;
		
		size_t countCharactersToWhiteSpace(const char* str, size_t strlen) const;
	protected:

		bool validatePieceList(const char* pieceList, size_t characterCount) const;
		bool validateCastlingRightsString(const char* str, size_t characterCount) const;
		bool validateSquareString(const char* str, size_t characterCount) const;
		bool validateSideToMove(const char* str, size_t characterCount) const;

		bool validateTokenized(const std::vector<std::string>& tokens) const;

	public:

		// Odziedziczono za po�rednictwem elementu FenValidator
		bool isValidFen(const char* fen) const override;

	};

	class SpellChessFenValidator : public FenValidator {
	protected:
		bool validateSpell(const char* str, size_t characterCount) const;
	public:
		// Odziedziczono za po�rednictwem elementu FenValidator
		bool isValidFen(const char* fen) const override;
	};


	class FenParser;

	class GameState {
		
		friend FenParser;

		bool spellChessMode;

		std::string makePieceFenStringFragment() const;
		std::string makeCastlingRightsFenStringFragment() const;
		std::string makeSpellInfoFenStringFragment() const;
		std::string makeSquareIndexFenStringFragment(const SquareIndex& index) const;

		std::string makeStandardChessFen() const;
		std::string makeSpellChessFen() const;

		void clearSpells();
		void loadCastlingRights(bool* array);
		void loadSpellInfos(const SpellCounter* array);
		void revokeCastlingRightsAssociatedWithGivenRook(const PlayerColor & rookColor, const SquareIndex & index);
		void makeCastling(const PlayerColor& playerColor, const CastlingDirection & direction);

	protected:
		PlayerColor sideToMove;
		SquareIndex enpassantSquare;
		SquareIndex frozeSquare;
		SquareIndex jumpSquare;
		uint16_t halfMoveRuleCounter;
		uint16_t fullMoveCounter;
		
		SpellCounter spells[2][2];
		bool castlingRights[2][2];
		BoardSquare board[64];

		inline SpellCounter& getSpell(const PlayerColor& side, const SpellIdentificator& spellType) {
			return spells[side - 1][spellType];
		}

		inline bool& getCastlingRight(const PlayerColor& side, const CastlingDirection& direction) {
			return castlingRights[side - 1][direction];
		}

		inline void decrementSpellCooldown(const PlayerColor& side, const SpellIdentificator& spellType) {
			auto& counter = getSpell(side, spellType);
			if (counter.cooldown > 0) 
				--counter.cooldown;
		}

		inline void decrementSpellUsages(const PlayerColor& side, const SpellIdentificator& spellType) {
			auto& counter = getSpell(side, spellType);
			--counter.usagesLeft;

			if (counter.usagesLeft == 0)
				counter.cooldown = 0;
			else
				counter.cooldown = SpellCooldown;
		}

		void frozeArea(const SquareIndex& sq);

		
	public:

		GameState(): sideToMove(White), enpassantSquare(NoSquare), frozeSquare(NoSquare), jumpSquare(NoSquare), halfMoveRuleCounter(0), fullMoveCounter(1) {
			spellChessMode = false;
			for (size_t i = 0; i < 64; ++i) {
				this->board[i] = { {None, Nobody}, false, false };
			}
			castlingRights[0][0] = castlingRights[0][1] = castlingRights[1][0] = castlingRights[1][1] = false;
			spells[0][FreezeSpell].spellId = spells[1][FreezeSpell].spellId = FreezeSpell;
			spells[0][JumpSpell].spellId = spells[1][JumpSpell].spellId = JumpSpell;

			for (size_t i = 0; i < 2; ++i) for (size_t j = 0; j < 2; ++j) {
				spells[i][j].cooldown = spells[i][j].usagesLeft = 0;
			}
		}

		// full standard chess game constructor
		GameState(BoardSquare* board, PlayerColor sideToMove, bool* castlingRights, SquareIndex enpassantSquare, uint16_t halfMoveCounter, uint16_t fullMoveCounter) :
			enpassantSquare(enpassantSquare), halfMoveRuleCounter(halfMoveCounter), fullMoveCounter(fullMoveCounter), sideToMove(sideToMove),
			spellChessMode(false), frozeSquare(NoSquare), jumpSquare(NoSquare) {
			for (size_t i = 0; i < 64; ++i)
				this->board[i] = board[i];
			loadCastlingRights(castlingRights);
		}

		// full spell chess game constructor
		GameState(BoardSquare* board, PlayerColor sideToMove, bool * castlingRights, SquareIndex enpassantSquare, uint16_t halfMoveCounter, uint16_t fullMoveCounter, 
			SquareIndex freezeSquare, SquareIndex jumpSquare, SpellCounter* spells) :
			enpassantSquare(enpassantSquare), halfMoveRuleCounter(halfMoveCounter), fullMoveCounter(fullMoveCounter), sideToMove(sideToMove),
			spellChessMode(true), frozeSquare(freezeSquare), jumpSquare(jumpSquare)
		{
			for (size_t i = 0; i < 64; ++i)
				this->board[i] = board[i];
			loadCastlingRights(castlingRights);
			loadSpellInfos(spells);
		}

		// copy constructor
		GameState(const GameState & other) {
			memcpy(this, &other, sizeof(GameState));
		};

		// getters
		const PlayerColor& getSideToMove() const { return sideToMove; };
		const SquareIndex& getEnppassantSquare() const { return enpassantSquare; };
		const SquareIndex& getJumpSquare() const { return jumpSquare; };
		const SquareIndex& getFreezeSquare() const { return frozeSquare;  }
		const uint16_t& getHalfMoveCounter() const { return halfMoveRuleCounter; };
		const uint16_t& getFullMoveCounter() const { return fullMoveCounter; }
		const BoardSquare& getSquare(const SquareIndex& i) const { return board[i]; };
		const SpellCounter& getSpell(const PlayerColor& side, const SpellIdentificator& spellType) const {
			return spells[side - 1][spellType];
		}

		bool canUseSpell(const PlayerColor& side, const SpellIdentificator& spellType) const {
			auto& spell = getSpell(side, spellType);
			return spell.usagesLeft && spell.cooldown == 0;
		}

		const bool& canCastle(const PlayerColor& side, const CastlingDirection& direction) const {
			return castlingRights[side - 1][direction];
		}

		bool isSpellChessGame() const { return spellChessMode; }

		bool hasSufficientMatingMaterial(const PlayerColor& side) const;

		inline std::string toFen() const {
			return spellChessMode ? makeSpellChessFen() : makeStandardChessFen();
		};

		void makeMove(const SpellChessMove& move);

		int countPiece(Piece piece) const;

	};
	const char START_POS[57] = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
	const char SPELL_START_POS[77] = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1 - - F50/J20/f50/j20";
	class FenParser {
		void loadSq(SquareIndex& sq, const std::string& token);
		void loadInteger(uint16_t& target, const std::string& token);
		void loadPieceListString(GameState & state, const std::string & token);
		void loadSideToMove(GameState& state, const std::string& token);
		void loadCastlingRightString(GameState& state, const std::string& token);
		void loadSpells(GameState& state, const std::string& token);

		void _loadStandardFen(GameState& state, const std::vector<std::string> & tokens);
		void _loadSpellChessFen(GameState& state, const std::vector<std::string>& tokens);

	public:
		GameState fromStandardChessFen(const std::string& fen);
		GameState fromSpellChessFen(const std::string& fen);
		
	};
	
};


