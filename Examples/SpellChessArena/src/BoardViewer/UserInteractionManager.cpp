#include <BoardViewer/UserInteractionManager.hpp>

using namespace BoardViewer;

void UserInteractionManager::tryAdvanceState() {
    if(nextState) 
        currentState.reset(nextState.release());
};

void UserInteractionManager::setNextState(std::unique_ptr<UserInteractionState> && nextState) {
    this->nextState.reset(nextState.release());
};

void UserInteractionManager::setCurrentState(std::unique_ptr<UserInteractionState> && state) {
    this->currentState.reset(state.release());
};

UserInteractionManager::UserInteractionManager() {};

void UserInteractionManager::onSpellSelected(SpellChessLib::SpellIdentifator spellId) {
    if (currentState){
        currentState->onSpellSelected(spellId);
        tryAdvanceState();
    }
};

void UserInteractionManager::onSpellDiscarded() {
    if(currentState) {
        currentState->onSpellDiscarded();
        tryAdvanceState();
    }
};

void UserInteractionManager::onEmptySquareClicked(SpellChessLib::SquareIndex index) {
    if (currentState){
        currentState->onEmptySquareClicked(index);
        tryAdvanceState();
    }
};

void UserInteractionManager::onPieceClicked(SpellChessLib::SquareIndex pieceSquare, SpellChessLib::Piece piece) {
    if (currentState) {
        currentState->onPieceClicked(pieceSquare, piece);
        tryAdvanceState();
    }
};

void UserInteractionManager::onPieceTargetSquareClicked(SpellChessLib::SquareIndex targetSquare) {
    if (currentState) {
        currentState->onPieceTargetSquareClicked(targetSquare);
        tryAdvanceState();
    };
};

void UserInteractionManager::onPieceDrag(Board::DragHandle<Board::BoardWidget> & drag) {
    if (currentState) {
        currentState->onPieceDrag(drag);
        tryAdvanceState();
    };
};

void UserInteractionState::transistTo(std::unique_ptr<UserInteractionState> && newState) {
    if (auto man = manager.lock()) {
        man->setNextState(std::move(newState));
    };
};

