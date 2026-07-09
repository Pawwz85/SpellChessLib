#include <UserInteractionManager.hpp>

using namespace BoardViewer;

void UserInteractionManager::tryAdvanceState() {
    if(nextState) {
        currentState.reset(nextState.release());
        stateChangedCallback();
    }
};

void UserInteractionManager::setNextState(std::unique_ptr<UserInteractionState> && nextState) {
    this->nextState.reset(nextState.release());
};

void UserInteractionManager::setCurrentState(std::unique_ptr<UserInteractionState> && state) {
    this->currentState.reset(state.release());
    stateChangedCallback();
};

UserInteractionManager::UserInteractionManager(std::function<void()> stateChangedCallback) : stateChangedCallback(stateChangedCallback)
 {};

void UserInteractionManager::onSpellSelected(SpellChessLib::SpellIdentificator spellId) {
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

std::optional<SpellChessLib::SpellIdentificator> UserInteractionManager::spellPickerRenderTip() const {
    return currentState ? currentState->spellPickerRenderTip() : std::nullopt;
};


void UserInteractionState::transistTo(std::unique_ptr<UserInteractionState> && newState) {
    if (auto man = manager.lock()) {
        man->setNextState(std::move(newState));
    };
};

std::optional<SpellChessLib::SpellIdentificator> UserInteractionState::spellPickerRenderTip() const {
    return std::nullopt;
}

