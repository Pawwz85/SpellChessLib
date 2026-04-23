#pragma once
#include "SpellCounterWidget.h" 
#include <QHBoxLayout>

namespace SpellCounter {

    class SpellBar : public QWidget {
        QHBoxLayout layout;
        SpellCounterWidget freezeSpell;
        SpellCounterWidget jumpSpell;

        static Config makeConfigForFreezeSpell();
        static Config makeConfigForJumpSpell();

        public:
        SpellBar(QWidget * parent = nullptr);
    };

};

