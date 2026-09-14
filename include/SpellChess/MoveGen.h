#pragma once

#include "Board.h"
#include <vector>

namespace SpellChessLib {
	enum SpellPolicy {
		SpellGeneratorEnforced,
		UserCanAddSpell
	};
	
	struct MoveCandidate {
		SpellPolicy spellPolicy;
		SpellChessMove move;
	};

	namespace _Constraints {
		bool isMoveViolatingPin(const SquareIndex& to, const SquareIndex & kingPos, const SquareIndex & pinningPiece); // returns true if given 'to' position violates pin
		uint64_t getBlockedAttacker(const SquareIndex& to, const SquareIndex& kingPos, const uint64_t& attackers); // returns 0 if no attacker is blocked, otherwise it returns a mask containing blocked attacker
		uint64_t getCapturedAttacker(const SquareIndex& to, const uint64_t& attackers);
		uint64_t getFrozenAttackers(const SquareIndex& freezeSq, const uint64_t& attackers); // return 0 if freezeSq is NoSquare
		uint64_t getDiscoveredAttackerByEnpassant(const GameState& state, const SquareIndex & movingSideKing,  const SquareIndex& capturingPawn);
		bool doFreezeFrozeAllAttackers(const SquareIndex& freeze, const uint64_t& attackers);
		bool doesFreezeContainSquare(const SquareIndex& freeze, const SquareIndex& sq);
	}


	class MoveGenerator {

		enum ConstrainType {
			NeutralizeCheck,
			NeutralizePin,
			NeutralizeSqAttackers,
			NeutralizeDiscoveredCheckByEnpassant,
			HasNoKingAttackers,
			CastJumpSq
		};

		struct Constrain {
			ConstrainType type;
			SquareIndex affectedSquare;
		};

		class ProtoMove {
		private:
			std::vector<Constrain> constrains;

		public:


			ProtoMove(const SquareIndex& from, const SquareIndex& to, const std::vector<Constrain> constrains) :
				from(from), to(to), constrains(constrains) {
			};

			using iterator = std::vector<Constrain>::iterator;
			using const_iterator = std::vector<Constrain>::const_iterator;

			const SquareIndex from;
			const SquareIndex to;
			inline iterator begin() { return constrains.begin(); };
			inline iterator end() { return constrains.end(); };
			inline const_iterator cbegin() const { return constrains.cbegin(); };
			inline const_iterator cend()  const { return constrains.cend(); };
		};

		const GameState* currentState;
		uint64_t controlledByEnemy;
		SquareIndex pins[64];
		SquareIndex kingPos;
		bool inCheck;

		void resetCache();
		


		void cacheEnemyPawn(const SquareIndex& sq, const PlayerColor & us);
		void cacheEnemyKnight(const SquareIndex& sq);
		void cacheEnemyKing(const SquareIndex& sq);
		void cacheEnemySliderRay(const GameState & state, const SquareIndex& sq, const PlayerColor & us,  const int& dx, const int& dy);
		void cacheEnemySlider(const GameState& state, const SquareIndex& sq, const PlayerColor& us, const PieceType& sliderType);
		void calculateCache(const GameState& state);

		bool doesMoveSatisfyJumpConstraint(const SpellChessMove& move, const SquareIndex& requiredJumpSq);
		bool doesMoveSatisfyCheckConstraint(const SpellChessMove& move, const uint64_t checkingPieces);
		bool doesMoveSatisfyPinConstraint(const SpellChessMove & move);
		
		void generateProtoMovesForSliderRay(const GameState& state, const int& x, const int& y, std::vector<ProtoMove>& out, std::vector<Constrain>& constrains, const int& dx, const int& dy) const;
	protected:
		void generateProtoMovesForPawn(const GameState& state, const SquareIndex & sq, std::vector<ProtoMove> & out) const;
		void generateProtoMovesForKnight(const GameState& state, const SquareIndex& sq, std::vector<ProtoMove>& out) const ;
		void generateProtoMovesForKing(const GameState& state, const SquareIndex& sq, std::vector<ProtoMove>& out) const ;
		void generateProtoMovesForRook(const GameState& state, const SquareIndex& sq, std::vector<ProtoMove>& out) const;
		void generateProtoMovesForBishop(const GameState& state, const SquareIndex& sq, std::vector<ProtoMove>& out) const;
		void generateProtoMoves(const GameState& state, std::vector<ProtoMove>& out) const;
		void expandProtoMove(const ProtoMove& protoMove, std::vector<MoveCandidate>& out) const;
	public:

		MoveGenerator(const GameState & state);
		SquareIndex getPinningPiece(const SquareIndex& sq) const;
		uint64_t getEnemyControlMask() const;
		std::vector<MoveCandidate> generateMoves() const;
		bool isInCheck() const;
		bool isMoveLegal(const SpellChessMove& move) const;
	};
		

}

