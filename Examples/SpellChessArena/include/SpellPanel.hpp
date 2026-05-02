#pragma once

#include <SpellChess/Board.h>
#include <SpriteSet/SpriteSet.hpp>

#include <Misc/Singleton.h>

#include <map>

#include <QObject>
#include <QWidget>
#include <QPushButton>

#include <QGridLayout>
#include <QHBoxLayout>

class SpellPanelModel : public QObject{
    Q_OBJECT
    std::map<SpellChessLib::SpellIdentifator, SpellChessLib::SpellCounter> spells;
    std::map<SpellChessLib::SpellIdentifator, bool> isSpellSelected;

    public:

    SpellPanelModel();

    [[nodiscard]] bool canBeSelected(SpellChessLib::SpellIdentifator spell) const;
    [[nodiscard]] bool canBeUsed(SpellChessLib::SpellIdentifator spell) const;
    bool trySelect(SpellChessLib::SpellIdentifator spell);
    bool deselect();
    bool isSelected(SpellChessLib::SpellIdentifator spell) const; 
    [[nodiscard]] const SpellChessLib::SpellCounter & getSpell(SpellChessLib::SpellIdentifator spell) const; 

    void setSpell(const SpellChessLib::SpellCounter & counter);
    
    signals:

    void spellChanged(SpellChessLib::SpellIdentifator spell); // signal for UI
    void spellSelected(SpellChessLib::SpellIdentifator spell);
    void spellDiscarded();

};

extern SpriteSet::SpriteSetScheme makeSpellSetSchemeFromFolder(const QString & pieceSetFolder);
extern SpriteSet::SpriteSetScheme makeDefaultSpellSetScheme();

class SpellSpriteSet : public SpriteSet::SpriteSet, public Misc::Singleton<SpellSpriteSet> {
    public:
    SpellSpriteSet();
};


class SpellButton : public QWidget {
    Q_OBJECT
    SpriteSet::SpriteReference icon;
    SpriteSet::SpriteReference gray_icon;

    SpellPanelModel * model;
    
    SpellChessLib::SpellIdentifator id;

    QPushButton * btn;
    QPushButton * cancelBtn;
    QLayout * layout;

    public: 

    SpellButton(SpellPanelModel & model, SpellChessLib::SpellIdentifator id);

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

    SpellPanelModel * model;

    public:

    SpellPanel(SpellPanelModel & model , QWidget * parent = nullptr);
};
