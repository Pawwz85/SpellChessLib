#pragma once
#include <QWidget>
#include <QGridLayout>
#include <BoardGrid/BoardGridWidget.h>
#include <SpellButton/SpellBar.h>

class SpellChessGui : public QWidget {
    QGridLayout layout;
    Board::BoardWidget board;
    SpellCounter::SpellBar upperSpellBar;
    SpellCounter::SpellBar lowerSpellBar;

    public:

    SpellChessGui(QWidget * parent = nullptr);

    Board::BoardWidget & getBoardWidget() {return board;};
    const Board::BoardWidget & getBoardWidget() const {return board;};


};