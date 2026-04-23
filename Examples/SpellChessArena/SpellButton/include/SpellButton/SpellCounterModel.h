#pragma once
#include <QObject>
#include <QPixmap>
#include <QString>

namespace SpellCounter {

    struct Config {
        int maxSpellCount;
        const QPixmap* icon;
        QString fallbackSpellText;
        QString cancelButtonLabel;
    };

    extern Config makeDefaultSpellCounterConfig();

    class SpellCounterModel : public QObject {
        Q_OBJECT
        Q_PROPERTY(int spell_count MEMBER m_spell_count NOTIFY onSpellCountChanged)
        Q_PROPERTY(int cooldown MEMBER m_cooldown NOTIFY onCooldownChanged)
        Q_PROPERTY(bool interactable MEMBER m_interactable);

        Config config;

        public:
        SpellCounterModel();

        void setSpellCount(int v);
        void setCooldown(int v);
        void setConfig(const Config & config);

        inline const Config & getConfig() const;

        signals:
        void onSpellCountChanged(int v);
        void onCooldownChanged(int v);
        void onConfigChanged();

        private:
        int m_spell_count;
        int m_cooldown;
        bool m_interactable;
    };

    const Config & SpellCounterModel::getConfig() const {
        return config;
    }
};