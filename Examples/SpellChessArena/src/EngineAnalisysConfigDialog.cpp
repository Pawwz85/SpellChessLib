#include <EngineAnalisysConfigDialog.hpp>
#include <EngineRepository.hpp>

#include <QVBoxLayout>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QLabel>

void
EngineAnalisisConfigDialog::initConfigWithDefaults() noexcept {
    config.engineId = -1u;
    config.mode     = EngineAnalisysMode::FixedDepth;
    config.depthLimit = 6;
    config.timeLimitMs = 250;
    config.recursive = false;
}

EngineAnalisysConfig 
EngineAnalisisConfigDialog::getAnalisisConfig() const noexcept {
    return config;
}

void 
EngineAnalisisConfigDialog::modeChanged(EngineAnalisysMode mode) {
    int targetValue;
    switch (mode)
    {
    case EngineAnalisysMode::FixedDepth:
        detailsLabel->setText(tr("Analisis depth:"));
        targetValue = config.depthLimit;
        detailsSelector->setMinimum(1);
        detailsSelector->setMaximum(50);
        detailsSelector->setSingleStep(1);
        detailsSelector->setValue(targetValue);
        break;
    
    case EngineAnalisysMode::FixedTimePerNode :
        detailsLabel->setText(tr("Analisis time (milliseconds per move):"));
        targetValue = config.timeLimitMs;
        detailsSelector->setMinimum(50);
        detailsSelector->setMaximum(1000*3600);
        detailsSelector->setSingleStep(50);
        detailsSelector->setValue(targetValue);
        break;

    default:
        break;
    }
}

EngineAnalisisConfigDialog::EngineAnalisisConfigDialog(QWidget * parent): QDialog(parent){
    initConfigWithDefaults();
    auto layout = new QVBoxLayout();

    auto & repo = getEnginePlayerRepositoryInstance();
    auto engineList = repo.getAll();
    
    QComboBox * engineSelector = new QComboBox(this);

    for (auto & engine : engineList) 
        engineSelector->addItem(QString::fromStdString(engine.name));

    connect(engineSelector, &QComboBox::currentIndexChanged, this, [this, engineList] (int index){
        if (index >= 0 && index < engineList.size())
            this->config.engineId = engineList[index].entry_id;
    });

    if (!engineList.isEmpty()) {
        engineSelector->setCurrentIndex(0);
        this->config.engineId = engineList[0].entry_id;
    }

    QComboBox * modeSelector = new QComboBox(this);
    modeSelector->addItems({tr("Fixed depth mode", "Computer analisys with fixed depth"),
                            tr("Fixed time mode", "Computer analisys with fixed time per position")});
    
    config.mode = EngineAnalisysMode::FixedTimePerNode;
    modeSelector->setCurrentIndex((int)config.mode);

    connect(modeSelector, &QComboBox::currentIndexChanged, this, [this](int index){
        auto oldMode = this->config.mode;
        auto & newMode = this->config.mode;
        switch (index)
        {
        case 0:
            newMode = EngineAnalisysMode::FixedDepth;
            break;
        case 1:
            newMode = EngineAnalisysMode::FixedTimePerNode;
            break;
        default:
            break;
        }

        if (oldMode != newMode)
            modeChanged(newMode);
    });

    detailsLabel = new QLabel(this);
    detailsSelector = new QSpinBox(this);
    modeChanged(config.mode);

    connect(detailsSelector, &QSpinBox::valueChanged, this, [this](int value){
        if (this->config.mode == EngineAnalisysMode::FixedDepth)
            this->config.depthLimit = value;
        else
            this->config.timeLimitMs = value;
    });

    recursiveMode = new QCheckBox(this);
    connect(recursiveMode, &QCheckBox::checkStateChanged, this, [this](Qt::CheckState s){
        this->config.recursive = s != Qt::Unchecked;
    });
    recursiveMode->setChecked(false);

    QDialogButtonBox * buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &EngineAnalisisConfigDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &EngineAnalisisConfigDialog::reject);

    layout->addWidget(new QLabel(tr("Engine"), this));
    layout->addWidget(engineSelector);
    layout->addWidget(new QLabel(tr("Analisis mode"), this));
    layout->addWidget(modeSelector);
    layout->addWidget(detailsLabel);
    layout->addWidget(detailsSelector);
    layout->addWidget(new QLabel(tr("Recursive mode", "Recursively annotate variation")));
    layout->addWidget(recursiveMode);
    layout->addWidget(buttonBox);
    setLayout(layout);
    setModal(true);
};

#include <moc_EngineAnalisysConfigDialog.cpp>