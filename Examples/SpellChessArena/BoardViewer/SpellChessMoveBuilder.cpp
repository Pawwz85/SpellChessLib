#include <SpellChessMoveBuilder.hpp>

BoardViewer::SpellChessMoveBuilder::SpellChessMoveBuilder() {
    reset();
};

BoardViewer::SpellChessMoveBuilder & BoardViewer::SpellChessMoveBuilder::setFromSquare(uint8_t fromSq) {
    this->currentValue.from = fromSq;
    return *this;
};  

BoardViewer::SpellChessMoveBuilder & BoardViewer::SpellChessMoveBuilder::setDestSquare(uint8_t destSq) {
    this->currentValue.to = destSq;
    return *this;
};

BoardViewer::SpellChessMoveBuilder & BoardViewer::SpellChessMoveBuilder::setFreezeSquare(uint8_t freezeSq) {
    this->currentValue.freezeSquare = freezeSq;
    return *this;
};

BoardViewer::SpellChessMoveBuilder & BoardViewer::SpellChessMoveBuilder::setJumpSquare(uint8_t jumpSq) {
    this->currentValue.jumpSquare = jumpSq;
    return *this;
};

BoardViewer::SpellChessMoveBuilder & BoardViewer::SpellChessMoveBuilder::setPromotion(SpellChessLib::PieceType promotionTarget) {
    this->currentValue.promoteTo = promotionTarget;
    return *this;
};

BoardViewer::SpellChessMoveBuilder & BoardViewer::SpellChessMoveBuilder::reset() {
    (void)clearSquares();
    (void)clearSpells();
    return *this;
};

BoardViewer::SpellChessMoveBuilder & BoardViewer::SpellChessMoveBuilder::clearSquares() {
    this->currentValue.from = this->currentValue.to = SpellChessLib::NoSquare;
    this->currentValue.promoteTo = SpellChessLib::None;
    return *this;
};


BoardViewer::SpellChessMoveBuilder & BoardViewer::SpellChessMoveBuilder::clearSpells() {
    this->currentValue.jumpSquare = this->currentValue.freezeSquare = SpellChessLib::NoSquare;
    return *this;
}


SpellChessLib::SpellChessMove BoardViewer::SpellChessMoveBuilder::build() {
    return this->currentValue;
}  