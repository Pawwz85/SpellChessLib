#include <SpellBar.h>

using namespace SpellCounter;

Config SpellBar::makeConfigForFreezeSpell() {
    return makeDefaultSpellCounterConfig();
};

Config SpellBar::makeConfigForJumpSpell() {
    return makeDefaultSpellCounterConfig();
};

SpellBar::SpellBar(QWidget * parent) : QWidget(parent) {
    freezeSpell.model.setConfig(makeConfigForFreezeSpell());
    jumpSpell.model.setConfig(makeConfigForJumpSpell());
    layout.addWidget(&freezeSpell);
    layout.addWidget(&jumpSpell);
    setLayout(&layout);
};