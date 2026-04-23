#include <SpellCounterWidget.h>

using namespace SpellCounter;


SpellCounterWidget::SpellCounterWidget(QWidget * parent) : QWidget(parent) {
    connect(&model, &SpellCounterModel::onSpellCountChanged, this, &SpellCounterWidget::onModelChanged);
    connect(&model, &SpellCounterModel::onCooldownChanged, this, &SpellCounterWidget::onModelChanged);
    connect(&model, &SpellCounterModel::onConfigChanged, this, &SpellCounterWidget::onModelChanged);
    layout.addWidget(&chooseSpellButton);
    layout.addWidget(&cancelSpellButton);
    setLayout(&layout);
    onModelChanged();
};

void SpellCounterWidget::onModelChanged() {
    const SpellCounter::Config & config = model.getConfig(); 

    if (config.icon != nullptr){
        chooseSpellButton.setIcon(*config.icon);
        chooseSpellButton.setText("not null");
    
    } else {
        chooseSpellButton.setIcon(QIcon());
        chooseSpellButton.setText(config.fallbackSpellText);
        chooseSpellButton.setWindowIconText(config.fallbackSpellText);
    }

    cancelSpellButton.setText(config.cancelButtonLabel);

    update();
};

#include <moc_SpellCounterWidget.cpp>