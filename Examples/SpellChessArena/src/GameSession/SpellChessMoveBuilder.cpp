#include <GameSession/SpellChessMoveBuilder.hpp>

GameSession::SpellChessMoveBuilder::SpellChessMoveBuilder() {
    reset();
};

GameSession::SpellChessMoveBuilder & GameSession::SpellChessMoveBuilder::setFromSquare(uint8_t fromSq) {
    this->currentValue.from = fromSq;
    return *this;
};  

GameSession::SpellChessMoveBuilder & GameSession::SpellChessMoveBuilder::setDestSquare(uint8_t destSq) {
    this->currentValue.to = destSq;
    return *this;
};

GameSession::SpellChessMoveBuilder & GameSession::SpellChessMoveBuilder::setFreezeSquare(uint8_t freezeSq) {
    this->currentValue.freezeSquare = freezeSq;
    return *this;
};

GameSession::SpellChessMoveBuilder & GameSession::SpellChessMoveBuilder::setJumpSquare(uint8_t jumpSq) {
    this->currentValue.jumpSquare = jumpSq;
    return *this;
};

GameSession::SpellChessMoveBuilder & GameSession::SpellChessMoveBuilder::setPromotion(SpellChessLib::PieceType promotionTarget) {
    this->currentValue.promoteTo = promotionTarget;
    return *this;
};

GameSession::SpellChessMoveBuilder & GameSession::SpellChessMoveBuilder::reset() {
    this->currentValue.from = this->currentValue.to = SpellChessLib::NoSquare;
    this->currentValue.jumpSquare = this->currentValue.freezeSquare = SpellChessLib::NoSquare;
    this->currentValue.promoteTo = SpellChessLib::None;
    return *this;
};

SpellChessLib::SpellChessMove GameSession::SpellChessMoveBuilder::build() {
    return this->currentValue;
}  