#include <ChessGui.h>

SpellChessGui::SpellChessGui(QWidget * parent) : QWidget(parent) {

    layout.addWidget(&board, 1, 1, 8, 8);
    layout.addWidget(&upperSpellBar, 0, 6, 1, 2);
    layout.addWidget(&lowerSpellBar, 9, 6, 1, 2);

    setLayout(&layout);
};