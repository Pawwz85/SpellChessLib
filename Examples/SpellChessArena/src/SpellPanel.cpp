#include <SpellPanel.hpp>
#include <QVBoxLayout>

SpellPanelModel::SpellPanelModel(): spells({
                                        {SpellChessLib::FreezeSpell, {}},
                                        {SpellChessLib::JumpSpell, {}}
                                    }),
                                    isSpellSelected{
                                        {SpellChessLib::FreezeSpell, false},
                                        {SpellChessLib::JumpSpell, false}
                                    } {
    
}

bool SpellPanelModel::canBeSelected(SpellChessLib::SpellIdentifator spell) const {
    auto other = (spell == SpellChessLib::FreezeSpell) ? SpellChessLib::JumpSpell : SpellChessLib::FreezeSpell;
    bool isAvailable = spells.at(spell).canCastNow();
    bool otherSelected = isSpellSelected.at(other);
    return isAvailable && !otherSelected;
};

bool  SpellPanelModel::canBeUsed(SpellChessLib::SpellIdentifator spell) const {
    return spells.at(spell).canCastNow();
}

bool SpellPanelModel::trySelect(SpellChessLib::SpellIdentifator spell) {
    if (!canBeSelected(spell))
        return false;

    isSpellSelected[spell] = true;
    spellSelected(spell); // update logic
    spellChanged(spell);  // update UI
    return true;
};

bool  SpellPanelModel::deselect() {
    bool anyChange = isSpellSelected[SpellChessLib::FreezeSpell] || isSpellSelected[SpellChessLib::JumpSpell];
    isSpellSelected[SpellChessLib::FreezeSpell] =  isSpellSelected[SpellChessLib::JumpSpell] = false;

    if (anyChange) {
        spellChanged(SpellChessLib::FreezeSpell);
        spellChanged(SpellChessLib::JumpSpell);
        spellDiscarded();
    }

    return anyChange;
}
bool SpellPanelModel::isSelected(SpellChessLib::SpellIdentifator spell) const{
    return isSpellSelected.at(spell);
};

SpriteSet::SpriteSetScheme makeSpellSetSchemeFromFolder(const QString & folderPath) {
        QString prefix = QString("file:") + folderPath + QString("/");

    std::vector<SpriteSet::SpriteSetScheme::SpriteDef> defs = {
        {"freeze",             prefix + "freeze.png"},
        {"freeze_inactive",    prefix + "freeze_inactive.png"},
        {"jump",               prefix + "jump.png"},
        {"jump_inactive",      prefix + "jump_inactive.png"}
    };

    return SpriteSet::SpriteSetScheme(defs);
}

SpriteSet::SpriteSetScheme makeDefaultSpellSetScheme() {
    return makeSpellSetSchemeFromFolder(QString("assets/spells/default"));
}

SpellSpriteSet::SpellSpriteSet() {
    setSchema(makeDefaultSpellSetScheme());
};

void SpellPanelModel::setSpell(const SpellChessLib::SpellCounter & counter) {
    spells[counter.spellId] = counter;
    isSpellSelected[counter.spellId] = false;
    spellChanged(counter.spellId);
};

const SpellChessLib::SpellCounter & SpellPanelModel::getSpell(SpellChessLib::SpellIdentifator spell) const {
    return spells.at(spell);
};

SpellButton::SpellButton(SpellPanelModel & model, SpellChessLib::SpellIdentifator id) :
    model(&model), id(id) {    

    QString spellStrId = (id == SpellChessLib::FreezeSpell) ? "freeze" : "jump";    
    icon        = SpellSpriteSet::getInstance().getSprite(spellStrId);
    gray_icon   = SpellSpriteSet::getInstance().getSprite(spellStrId + "_inactive");
    
    layout = new QVBoxLayout(this);
    btn = new QPushButton(this);
    cancelBtn = new QPushButton(tr("cancel", "Cancel spell casting"));
    cooldownBar = new QProgressBar(this);
    
    cooldownBar->setMaximum(2);
    cooldownBar->setMinimum(0);
    cooldownBar->setTextVisible(false);
    
    connect(btn, &QPushButton::clicked, this, &SpellButton::onClick);
    connect(cancelBtn, &QPushButton::clicked, this, &SpellButton::onCancel);

    connect(&model, &SpellPanelModel::spellChanged, this,  &SpellButton::onModelChanged);
    connect(&SpellSpriteSet::getInstance(), &SpriteSet::SpriteSet::onSpriteSetReloaded, this, &SpellButton::onModelChanged);

    layout->setContentsMargins(QMargins(0,0,0,0));
    layout->addWidget(cooldownBar);
    layout->addWidget(btn);
    layout->addWidget(cancelBtn);
    setLayout(layout);

    onModelChanged();

}

void SpellButton::onCancel() {
    model->deselect();
};

void SpellButton::onModelChanged () {
    if (model->canBeSelected(id)) {
        btn->setIcon(*icon);
    } else {
        btn->setIcon(*gray_icon);
    }

    btn->setText(QString(char('0' + model->getSpell(id).usagesLeft)));

    if (model->isSelected(id))
        cancelBtn->show();
    else
        cancelBtn->hide();

    cooldownBar->setValue(model->getSpell(id).cooldown);
    
    /*if (model->getSpell(id).canCastNow()) 
        cooldownBar->hide();
    else
        cooldownBar->show();
    */

    update();
};

void SpellButton::onClick() {
    model->trySelect(id);
};

SpellPanel::SpellPanel(SpellPanelModel & model , QWidget * parent) : QWidget(parent) {
    freeze = new SpellButton(model, SpellChessLib::FreezeSpell);
    jump = new SpellButton(model, SpellChessLib::JumpSpell);

    freeze->setParent(this);
    jump->setParent(this);

    layout = new QHBoxLayout(this);
    layout->addWidget(freeze);
    layout->addWidget(jump);
    layout->setContentsMargins(QMargins(0, 0, 0, 0));

    setLayout(layout);
};



#include <moc_SpellPanel.cpp>