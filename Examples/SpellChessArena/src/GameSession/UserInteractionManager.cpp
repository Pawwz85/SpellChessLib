#include <GameSession/UserInteractionManager.hpp>

using namespace GameSession;

void UserInteractionManager::tryAdvanceState() {
    if(nextState) 
        currentState.reset(nextState.release());
};

void UserInteractionManager::setNextState(std::unique_ptr<UserInteractionState> && nextState) {
    this->nextState.reset(nextState.release());
};

UserInteractionManager::UserInteractionManager(std::unique_ptr<UserInteractionState> && initialState) {
    this->currentState.reset(initialState.release());
};

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

void UserInteractionManager::onPieceDragStarted(SpellChessLib::SquareIndex pieceSquare, SpellChessLib::Piece piece) {
    if (currentState) {
        currentState->onPieceDragStarted(pieceSquare, piece);
        tryAdvanceState();
    };
};

void UserInteractionManager::onPieceDragEnded(SpellChessLib::SquareIndex targetSquare) {
    if (currentState) {
        currentState->onPieceDragEnded(targetSquare);
        tryAdvanceState();
    };
};

void UserInteractionManager::onPromotionTargetSelected(SpellChessLib::PieceType promotionTarget) {
  if (currentState) {
        currentState->onPromotionTargetSelected(promotionTarget);
        tryAdvanceState();
    };
};


void UserInteractionState::transistTo(std::unique_ptr<UserInteractionState> && newState) {
    if (auto man = manager.lock()) {
        man->setNextState(std::move(newState));
    };
};

