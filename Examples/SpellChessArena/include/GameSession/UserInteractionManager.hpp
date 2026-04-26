#pragma once

#include <SpellChess/SpellChessGame.h>
#include <memory>

namespace GameSession {

    /*
        This file defines a state machine responsible for handling high level user interactions (squareClicked, 
        pieceSelected, promotionTarget selected, spell selected, spell casted etc...).
    */
    class UserInteractionState;

     class UserInteractionManager : public std::enable_shared_from_this<UserInteractionManager> {
        std::unique_ptr<UserInteractionState> currentState;
        std::unique_ptr<UserInteractionState> nextState;

        void tryAdvanceState();

        public:
        
        void setNextState(std::unique_ptr<UserInteractionState> && nextState);
        void setCurrentState(std::unique_ptr<UserInteractionState> && state);

        UserInteractionManager();
        virtual ~UserInteractionManager() = default;
        void onSpellSelected(SpellChessLib::SpellIdentifator spellId);
        void onSpellDiscarded();
        void onEmptySquareClicked(SpellChessLib::SquareIndex squareIndex);
        void onPieceClicked(SpellChessLib::SquareIndex pieceSquare, SpellChessLib::Piece piece);
        void onPieceTargetSquareClicked(SpellChessLib::SquareIndex targetSquare);
        void onPieceDragStarted(SpellChessLib::SquareIndex pieceSquare, SpellChessLib::Piece piece);
        void onPieceDragEnded(SpellChessLib::SquareIndex targetSquare);
     };

      class UserInteractionState {
        protected:
        std::weak_ptr<UserInteractionManager> manager;

        void transistTo(std::unique_ptr<UserInteractionState> && newState);

        public:
        UserInteractionState(std::weak_ptr<UserInteractionManager> manager) : manager(manager) {};
        virtual ~UserInteractionState() = default;
        virtual void onSpellSelected(SpellChessLib::SpellIdentifator spellId) = 0;
        virtual void onSpellDiscarded() = 0;
        virtual void onEmptySquareClicked(SpellChessLib::SquareIndex squareIndex) = 0;
        virtual void onPieceClicked(SpellChessLib::SquareIndex pieceSquare, SpellChessLib::Piece piece) = 0;
        virtual void onPieceTargetSquareClicked(SpellChessLib::SquareIndex targetSquare) = 0;
        virtual void onPieceDragStarted(SpellChessLib::SquareIndex pieceSquare, SpellChessLib::Piece piece) = 0;
        virtual void onPieceDragEnded(SpellChessLib::SquareIndex targetSquare) = 0;
     };
     
};