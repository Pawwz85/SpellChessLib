#pragma once

#include <memory>
#include "OpeningBook.h"
#include "Player.h"
#include "Scheduler/ObjectAllocator.h"

namespace SpellChessLib {
    
    class PlayerWithOpeningBook : public Player {
        class PlayerWithOpeningBookPointer;

        Scheduler::ResourcePointer<Player> wrapped;
        std::shared_ptr<OpeningBook> book;
        
        Result makeResult(const Game& game, const std::chrono::milliseconds & timeOut) override {};
		Result handlePonderHit() override {};
		void startPondering(const Game& game, const SpellChessMove & ponderMove) override {};
		void cancelPondering() override {};

        virtual bool supportsPondering() const override;
		virtual void onNewGame() override;
		virtual void interrupt() override;
		virtual void kill() override;
        virtual Result search(const Game& game) override;
		virtual void ponder(const Game& game, const SpellChessMove & ponderMove) override;
        public:
        PlayerWithOpeningBook(Scheduler::ResourcePointer<Player> && wraped, std::shared_ptr<OpeningBook> book);
    };

    class PlayerWithOpeningBookAllocator : public Scheduler::ObjectAllocator<Player>, 
        public std::enable_shared_from_this<PlayerWithOpeningBookAllocator> {
        std::shared_ptr<OpeningBook> book;
        std::shared_ptr<Scheduler::ObjectAllocator<Player>> wrappedPlayerAllocator;
        
        void release(Player* o) override;
		void destroy(Player* o) override;

        public:
        PlayerWithOpeningBookAllocator(std::shared_ptr<Scheduler::ObjectAllocator<Player>> alloc, std::shared_ptr<OpeningBook> book);
        
		Scheduler::ResourcePointer<Player> retrieve() override;

    };

};