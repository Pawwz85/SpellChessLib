#include <SpellPanel.hpp>
#include <QVBoxLayout>

#include <QPainter>
#include <QBrush>

using namespace BoardViewer;

SpellPanelModel::SpellPanelModel(): spells({
                                        {SpellChessLib::FreezeSpell, {}},
                                        {SpellChessLib::JumpSpell, {}}
                                    }),
                                    isSpellSelected{
                                        {SpellChessLib::FreezeSpell, false},
                                        {SpellChessLib::JumpSpell, false}
                                    } {
    
}

bool SpellPanelModel::canBeSelected(SpellChessLib::SpellIdentificator spell) const {
    auto other = (spell == SpellChessLib::FreezeSpell) ? SpellChessLib::JumpSpell : SpellChessLib::FreezeSpell;
    bool isAvailable = spells.at(spell).canCastNow();
    bool otherSelected = isSpellSelected.at(other);
    return isAvailable && !otherSelected;
};

bool  SpellPanelModel::canBeUsed(SpellChessLib::SpellIdentificator spell) const {
    return spells.at(spell).canCastNow();
}

bool SpellPanelModel::trySelect(SpellChessLib::SpellIdentificator spell) {
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
bool SpellPanelModel::isSelected(SpellChessLib::SpellIdentificator spell) const{
    return isSpellSelected.at(spell);
};

SpellCoolDownBar::SpellCoolDownBar(int m, QWidget * parent) : QWidget(parent), maximum(m), value(0),
    emptyContainerColor(Qt::lightGray), fullContainerColor(Qt::yellow) {};

void SpellCoolDownBar::setMaximum(int maximum) {
    this->maximum = maximum;
}

void SpellCoolDownBar::setValue(int val) {
    this->value = val;
    update();
}

void SpellCoolDownBar::paintEvent(QPaintEvent * event) {
    QPainter painter(this);

    const auto & r = rect();

    int w = r.width();
    int gapSize = w / 20; 
    int rectSize = w - gapSize;
    if (maximum) {
        gapSize  /= (maximum - 1);
        rectSize /= maximum;
    }

    int begin = r.left();
    
    for (size_t i = 0; i < maximum; ++i) {
        QColor fill = (maximum - value <= i) ? emptyContainerColor : fullContainerColor;
        painter.setBrush(fill);
        painter.drawRect(begin, r.top(), rectSize, r.height());
        begin += rectSize + gapSize;    
    };
    
};


SpriteSet::SpriteSetScheme BoardViewer::makeSpellSetSchemeFromFolder(const QString & folderPath) {
        QString prefix = QString("file:") + folderPath + QString("/");

    std::vector<SpriteSet::SpriteSetScheme::SpriteDef> defs = {
        {"freeze",             prefix + "freeze.png"},
        {"freeze_inactive",    prefix + "freeze_inactive.png"},
        {"jump",               prefix + "jump.png"},
        {"jump_inactive",      prefix + "jump_inactive.png"}
    };

    return SpriteSet::SpriteSetScheme(defs);
}

SpriteSet::SpriteSetScheme BoardViewer::makeDefaultSpellSetScheme() {
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

const SpellChessLib::SpellCounter & SpellPanelModel::getSpell(SpellChessLib::SpellIdentificator spell) const {
    return spells.at(spell);
};

SpellButton::SpellButton(SpellPanelModel & model, SpellChessLib::SpellIdentificator id) :
    model(&model), id(id) {    

    QString spellStrId = (id == SpellChessLib::FreezeSpell) ? "freeze" : "jump";    
    icon        = SpellSpriteSet::getInstance().getSprite(spellStrId);
    gray_icon   = SpellSpriteSet::getInstance().getSprite(spellStrId + "_inactive");
    
    layout = new QVBoxLayout(this);
    btn = new QPushButton(this);
 
    cooldownBar = new SpellCoolDownBar(2, this);
    
    cooldownBar->setMaximum(2);
    
    connect(btn, &QPushButton::clicked, this, &SpellButton::onClick);

    connect(&model, &SpellPanelModel::spellChanged, this,  &SpellButton::onModelChanged);
    connect(&SpellSpriteSet::getInstance(), &SpriteSet::SpriteSet::onSpriteSetReloaded, this, &SpellButton::onModelChanged);

    layout->setContentsMargins(QMargins(0,0,0,0));
    layout->addWidget(cooldownBar);
    layout->addWidget(btn);

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

    cooldownBar->setValue(model->getSpell(id).cooldown);    
    /*if (model->getSpell(id).canCastNow()) 
        cooldownBar->hide();
    else
        cooldownBar->show();
    */

    update();
};

void SpellButton::onClick() {
    if (model->isSelected(id))
        model->deselect();
    else
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