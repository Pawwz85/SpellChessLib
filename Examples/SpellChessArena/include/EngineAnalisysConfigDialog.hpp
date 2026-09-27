#pragma once

#include <cstdint>

#include <QDialog>
#include <QLabel>
#include <QSpinBox>
#include <QCheckBox>

enum class EngineAnalisysMode {
    FixedDepth,
    FixedTimePerNode
};

struct EngineAnalisysConfig {
    uint32_t engineId;
    EngineAnalisysMode mode;
    uint32_t depthLimit; 
    uint32_t timeLimitMs;
    bool recursive;
};


class EngineAnalisisConfigDialog : public QDialog {
    Q_OBJECT
    EngineAnalisysConfig config;
    QLabel * detailsLabel;
    QSpinBox * detailsSelector;
    QCheckBox * recursiveMode;

    void initConfigWithDefaults() noexcept;
    void modeChanged(EngineAnalisysMode mode);

    public:
    EngineAnalisisConfigDialog(QWidget * parent = nullptr);
    EngineAnalisysConfig getAnalisisConfig() const noexcept;
};