#pragma once

#include "Board.h"
#include <chrono>
#include <memory>


namespace SpellChessLib {
	class Game;

	class Player {
	public:
		class Result;
	private:
		std::unique_ptr<SpellChessMove> ponderMove = nullptr;
		void clearPonderMove() { ponderMove = nullptr; };
	protected:
		bool ponderhit(const SpellChessMove& move) const;
		void setPonderMove(const SpellChessMove& move);
		virtual Result makeResult(const Game& game, const std::chrono::milliseconds & timeOut) = 0;
		virtual Result handlePonderHit() = 0;
		virtual void startPondering(const Game& game, const SpellChessMove & ponderMove) = 0;
		virtual void cancelPondering() = 0;

	public:
		
		class Result {
			std::unique_ptr<SpellChessMove> ponderMove;
			std::unique_ptr<SpellChessMove> bestMove;
		public:
			Result(const SpellChessMove& m) : bestMove(new SpellChessMove(m)), ponderMove(nullptr) {};
			Result(const SpellChessMove& bestMove, const SpellChessMove& ponder) : bestMove(new SpellChessMove(bestMove)), ponderMove(new SpellChessMove(ponder)) {};
			const SpellChessMove& getBestMove() { return *bestMove; }
			const SpellChessMove& getPonderMove() { return *ponderMove; }
			bool hasPonderMove() const { return ponderMove != nullptr; };
		};
		virtual ~Player() = default;
		bool isPondering() { return ponderMove != nullptr; };
		virtual Result search(const Game& game);
		virtual void ponder(const Game& game, const SpellChessMove & ponderMove);
		virtual bool supportsPondering() const = 0;
		virtual void onNewGame() = 0;
		virtual void interrupt() = 0; // ask player to gracefully return control to the game runner thread
		virtual void kill() = 0; // Game Runner patience has ended. Return control back unconditionally 
		virtual bool isAlive() { return true; }; 
	};
}

