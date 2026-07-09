#pragma once

#include <SpellChess/Board.h>
#include <SpriteSet/SpriteSet.hpp>

#include <Misc/Singleton.h>

#include <map>

#include <QObject>
#include <QWidget>
#include <QPushButton>
#include <QProgressBar>
#include <QColor>

#include <QGridLayout>
#include <QHBoxLayout>

namespace BoardViewer {
    class SpellPanelModel : public QObject{
        Q_OBJECT
        std::map<SpellChessLib::SpellIdentificator, SpellChessLib::SpellCounter> spells;
        std::map<SpellChessLib::SpellIdentificator, bool> isSpellSelected;

        public:

        SpellPanelModel();

        [[nodiscard]] bool canBeSelected(SpellChessLib::SpellIdentificator spell) const;
        [[nodiscard]] bool canBeUsed(SpellChessLib::SpellIdentificator spell) const;
        bool trySelect(SpellChessLib::SpellIdentificator spell);
        bool deselect();
        bool isSelected(SpellChessLib::SpellIdentificator spell) const; 
        [[nodiscard]] const SpellChessLib::SpellCounter & getSpell(SpellChessLib::SpellIdentificator spell) const; 

        void setSpell(const SpellChessLib::SpellCounter & counter);
        
        signals:

        void spellChanged(SpellChessLib::SpellIdentificator spell); // signal for UI
        void spellSelected(SpellChessLib::SpellIdentificator spell);
        void spellDiscarded();

    };

    extern SpriteSet::SpriteSetScheme makeSpellSetSchemeFromFolder(const QString & pieceSetFolder);
    extern SpriteSet::SpriteSetScheme makeDefaultSpellSetScheme();

    class SpellSpriteSet : public SpriteSet::SpriteSet, public Misc::Singleton<SpellSpriteSet> {
        public:
        SpellSpriteSet();
    };

    class SpellCoolDownBar : public QWidget {
        int maximum;
        int value;

        QColor emptyContainerColor;
        QColor fullContainerColor;
        QColor outlineColor;

        public:
        SpellCoolDownBar(int max = 2, QWidget * parent = nullptr);
        void setMaximum(int maximum);
        void setValue(int val);
        void paintEvent(QPaintEvent * event) override;
    };

    class SpellButton : public QWidget {
        Q_OBJECT
        SpriteSet::SpriteReference icon;
        SpriteSet::SpriteReference gray_icon;

        SpellPanelModel * model;
        
        SpellChessLib::SpellIdentificator id;

        QPushButton * btn;
        QLayout * layout;

        SpellCoolDownBar * cooldownBar;

        public: 

        SpellButton(SpellPanelModel & model, SpellChessLib::SpellIdentificator id);

        public slots:
        void onModelChanged();

        private slots:
        void onClick();
        void onCancel();
    };


    class SpellPanel : public QWidget {
        Q_OBJECT
        QLayout * layout;

        SpellButton * freeze;
        SpellButton * jump;
        
        QPushButton * clearSpellButton;

        SpellPanelModel * model;

        public:

        SpellPanel(SpellPanelModel & model , QWidget * parent = nullptr);
    };
};
