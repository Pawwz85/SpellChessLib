#pragma once

#include "Board.h"
#include "MoveGen.h"
#include "TimerService.h"
#include "GameAnnotations.h"
#include "ZobristHash.h"
#include <mutex>
#include <memory>
#include <functional>
#include <list>
#include <thread>
#include <UCILoader/UCI.h>

namespace SpellChessLib {


	enum GameStatusCode {
		StillOngoing,
		IllegalMovePlayed,
		CheckMate,
		Stalemate,
		InsufficientMaterial,
		OpponentTimedOutWithoutSufficientMatingMaterial,
		OpponentTimedOut,
		DrawBy50MoveRule,
		DrawBy3FoldRepetition,
		OpponentDisconnected,
		KingCaptured,
		PlayerCrashed,
		GamesStatusCode_membersCount
	};
	
	enum ResignReason {
		ExceptionThrownByAPlayer
	};

	class Game {
	public:
		class ResultObserver;
	private:
		GameStatusCode statusCode = StillOngoing;
		PlayerColor winner = Nobody;
		GameState state, root;
		mutable UCILoader::GoParamsBuilder<SpellChessMove> goParamsBuilder;

		std::vector<SpellChessMove> movesSinceRoot;
		std::vector<uint64_t> posHistory;
		std::unique_ptr<TimerController> whTimer;
		std::unique_ptr<TimerController> blTimer;
		std::vector<std::weak_ptr<ResultObserver>> resultObservers;

		Annotation::GameAnnotations annotations;

		void timeOutPlayer(const PlayerColor& color);
		void notifyResultObservers();
		virtual void exportGoParams(UCILoader::GoParamsBuilder<SpellChessMove>& target) const;

		bool has3FoldRepetition() const;
	protected:
		mutable std::mutex lock;
		void setOutcome(const PlayerColor & winner, const GameStatusCode & outcome);


	public:

		class ResultObserver {
		public:
			virtual void onGameOutcome(const PlayerColor& winner, const GameStatusCode& outcome) = 0;
		};

		Game(const GameState& state, std::unique_ptr<TimerController>&& whTimer, std::unique_ptr<TimerController>&& blTimer) :
			whTimer(whTimer.release()), blTimer(blTimer.release()), state(state), root(state) {
			annotations.root = root;
			this->whTimer->setExpirationClb([this]() {this->timeOutPlayer(White); });
			this->blTimer->setExpirationClb([this]() {this->timeOutPlayer(Black); });
			posHistory.push_back(calculateHash(state));
		};
		virtual ~Game() {};
		virtual GameState getRoot() const;
		virtual GameState getGameState() const;
		virtual const GameState& getRoot();
		virtual std::vector<SpellChessMove> getMovesSinceRoot() const;
		virtual PlayerColor getWinner() const;
		virtual GameStatusCode getStatusCode() const;
		bool isFinished() { return getStatusCode() != StillOngoing; };
		std::chrono::milliseconds getRemainingTime(const PlayerColor& color) const;
		virtual void makeMove(const SpellChessMove& move);
		virtual void addResultObserver(std::weak_ptr<ResultObserver> observer);
		virtual void startCurrentPlayerTimer(); // used by game runner to activate game timer
		virtual void resign(const PlayerColor& side, const ResignReason & reason);
		UCILoader::GoParamsBuilder<SpellChessMove>& getGoParamsBuilder();
		Annotation::SevenTagRoster getRoster() const;
		UCILoader::GoParams<SpellChessMove> getGoParams() const;
		void setSevenTagRooster(const Annotation::SevenTagRoster& roster);
		const Annotation::GameAnnotations& getAnnotations() const;
	};
}