#include <PieceSpriteSet.hpp>

SpriteSet::SpriteSetScheme Board::makePieceSetSchemeFromFolder(const QString & folderPath) {
    QString prefix = QString("file:") + folderPath + QString("/");

    std::vector<SpriteSet::SpriteSetScheme::SpriteDef> defs = {
        {"white_pawn",      prefix + "wp.png"},
        {"white_rook",      prefix + "wr.png"},
        {"white_bishop",    prefix + "wb.png"},
        {"white_knight",    prefix + "wn.png"},
        {"white_king",      prefix + "wk.png"},
        {"white_queen",     prefix + "wq.png"},
        {"black_pawn",      prefix + "bp.png"},
        {"black_rook",      prefix + "br.png"},
        {"black_bishop",    prefix + "bb.png"},
        {"black_knight",    prefix + "bn.png"},
        {"black_king",      prefix + "bk.png"},
        {"black_queen",     prefix + "bq.png"}
    };

    return SpriteSet::SpriteSetScheme(defs);
}

SpriteSet::SpriteSetScheme Board::makeDefaultPieceSetScheme() {
    return makePieceSetSchemeFromFolder(QString("assets/pieces/default"));
}


Board::PieceSpriteSet::PieceSpriteSet() {
    SpriteSet::setSchema(makeDefaultPieceSetScheme());
}