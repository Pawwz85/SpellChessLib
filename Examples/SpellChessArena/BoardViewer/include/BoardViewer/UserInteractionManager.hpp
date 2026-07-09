#pragma once

#include <SpellChess/Board.h>
#include <SpellChess/SpellChessGame.h>
#include <BoardGrid/BoardGridWidget.h>
#include <memory>
#include <optional>
#include <functional>

namespace BoardViewer {

    /*
        This file defines a state machine responsible for handling high level user interactions (squareClicked, 
        pieceSelected, promotionTarget selected, spell selected, spell casted etc...).
    */
    class UserInteractionState;

     class UserInteractionManager : public std::enable_shared_from_this<UserInteractionManager> {
        std::unique_ptr<UserInteractionState> currentState;
        std::unique_ptr<UserInteractionState> nextState;
        std::function<void()> stateChangedCallback;

        void tryAdvanceState();

        public:
        
        void setNextState(std::unique_ptr<UserInteractionState> && nextState);
        void setCurrentState(std::unique_ptr<UserInteractionState> && state);

        UserInteractionManager(std::function<void()> stateChangedCallback);
        virtual ~UserInteractionManager() = default;
        void onSpellSelected(SpellChessLib::SpellIdentificator spellId);
        void onSpellDiscarded();
        void onEmptySquareClicked(SpellChessLib::SquareIndex squareIndex);
        void onPieceClicked(SpellChessLib::SquareIndex pieceSquare, SpellChessLib::Piece piece);
        void onPieceTargetSquareClicked(SpellChessLib::SquareIndex targetSquare);
        void onPieceDrag(Board::DragHandle<Board::BoardWidget> & drag);
        std::optional<SpellChessLib::SpellIdentificator> spellPickerRenderTip() const;
    };

      class UserInteractionState {
        protected:
        std::weak_ptr<UserInteractionManager> manager;

        void transistTo(std::unique_ptr<UserInteractionState> && newState);

        public:
        UserInteractionState(std::weak_ptr<UserInteractionManager> manager) : manager(manager) {};
        virtual ~UserInteractionState() = default;
        virtual void onSpellSelected(SpellChessLib::SpellIdentificator spellId) = 0;
        virtual void onSpellDiscarded() = 0;
        virtual void onEmptySquareClicked(SpellChessLib::SquareIndex squareIndex) = 0;
        virtual void onPieceClicked(SpellChessLib::SquareIndex pieceSquare, SpellChessLib::Piece piece) = 0;
        virtual void onPieceTargetSquareClicked(SpellChessLib::SquareIndex targetSquare) = 0;
        virtual void onPieceDrag(Board::DragHandle<Board::BoardWidget> & drag) = 0;
        virtual std::optional<SpellChessLib::SpellIdentificator> spellPickerRenderTip() const;
     };
     
};