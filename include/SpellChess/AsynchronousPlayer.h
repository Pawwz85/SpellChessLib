#pragma once

#include "Player.h"
#include "Board.h"

#include <chrono>
#include <optional>
#include <future>

namespace SpellChessLib {

    class AsynchronousPlayer : public Player {
        std::optional<std::promise<SpellChessMove>> currentMovePromise;
        GameState currentGameState;
        bool wasInterrupted = false;

        public:

        // Methods inherited from base class
        Result makeResult(const Game& game, const std::chrono::milliseconds & timeOut) override;
        Result handlePonderHit() override;
		void startPondering(const Game& game, const SpellChessMove & ponderMove) override;
		void cancelPondering() override;
        bool supportsPondering() const override;
		virtual void onNewGame() override;
		void interrupt() override;
        void kill() override; 

        void resolveCurrentMovePromise(const SpellChessMove& move);
    };

};