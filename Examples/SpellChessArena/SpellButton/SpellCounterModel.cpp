#include <SpellCounterModel.h>
#include <cstring>
using namespace SpellCounter;

SpellCounter::Config SpellCounter::makeDefaultSpellCounterConfig() {
    return {
        3,
        nullptr,
        QString("Some Spell"),
        QString("Cancel")
    };
};

SpellCounterModel::SpellCounterModel() : config(makeDefaultSpellCounterConfig()) {

};

void SpellCounterModel::setSpellCount(int v){
    m_spell_count = v;
    onSpellCountChanged(v);
}

void SpellCounterModel::setCooldown(int v){
    m_cooldown = v; 
    onCooldownChanged(v);
}

void SpellCounterModel::setConfig(const Config & config) {
    this->config = config;
    this->onConfigChanged();
}

#include <moc_SpellCounterModel.cpp>