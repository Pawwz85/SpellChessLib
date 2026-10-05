#pragma once
#include "Board.h"

namespace SpellChessLib {

	// find king position 
	SquareIndex findKingPos(const GameState& state, const PlayerColor& side);
	// SquareIndex findPinningPiece(const GameState& state, const SquareIndex& sq, const PlayerColor& kingSide);

	/*
		-1 if move is not castling move
		0 if move is KingSide castling
		1 if move is QueenSide castling
	*/
	int isMoveCastling(const GameState& state, const SquareIndex& from, const SquareIndex& to);

	bool arePointsColinear(const int8_t& x1, const int8_t& y1, const int8_t& x2, const int8_t& y2);
	int _sliderStep(const int8_t& u, const int8_t& w);
	
	bool isCastlingLegal(const GameState& state, const CastlingDirection& dir);
	bool isSquareInFrozenArea(const SquareIndex& sq, const SquareIndex& freezeCenter);

	uint64_t calculateFrozenArea(const SquareIndex & freezeCenter);

	namespace _Attackers {
		uint64_t getKnightAttackers(const GameState& state, const SquareIndex& sq, const PlayerColor& them);
		uint64_t getPawnAttackers(const GameState& state, const SquareIndex& sq, const PlayerColor& them);
		uint64_t getKingAttackers(const GameState& state, const SquareIndex& sq, const PlayerColor& them);
		SquareIndex castRay(const GameState& state, const SquareIndex& sq, int dx, int dy);
		uint64_t getSliderAttackers(const GameState& state, const SquareIndex& sq, const PlayerColor& them);
	}

	extern uint64_t getAttackers(const GameState& state, const SquareIndex& sq, const PlayerColor& them);

//	extern uint64_t getSquareAttackers(const GameState & state, const SquareIndex& sq, const PlayerColor& them);

	/*
		is[]PseudoLegal functions check whenever given move could be legal without ensuring moving side's king safety.
	*/


	extern bool isPawnMovePseudoLegal(const GameState& state, const PlayerColor & us, const SquareIndex & from, const SquareIndex & to, const SquareIndex & jumpSq);
	extern bool isKnightMovePseudoLegal(const GameState& state, const PlayerColor& us, const SquareIndex& from, const SquareIndex& to);
	bool isKingMovePseudoLegal(const GameState& state, const PlayerColor& us, const SquareIndex& from, const SquareIndex& to);
	bool isSliderMovePseudoLegal(const GameState& state, const PlayerColor& us, const SquareIndex& from, const SquareIndex& to, const SquareIndex& jumpSq);

}
