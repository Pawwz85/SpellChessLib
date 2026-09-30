#include <EngineControlPanel.hpp>
#include <map>
#include <QVBoxLayout>
#include <QCheckBox>
#include <QSpinBox>
#include <QComboBox>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>

void EngineControlPanel::disconnectEngine() {
    delete layout();
}

void EngineControlPanel::connectEngine(const EngineServiceEntry & engineEntry) {
    using Method =  QWidget * (EngineControlPanel::*)(UCILoader::EngineOptionProxy &);
    static std::map<UCILoader::OptionType, Method> optionMethods = {
        {UCILoader::Button, &EngineControlPanel::addButton},
        {UCILoader::Spin, &EngineControlPanel::addSpinBox},
        {UCILoader::Combo, &EngineControlPanel::addComboBox},
        {UCILoader::Check, &EngineControlPanel::addCheckBox},
        {UCILoader::String, &EngineControlPanel::addStringInput}
    };

    QVBoxLayout * widgetLayout = new QVBoxLayout(this);

    for (auto & option : engineEntry.engine->options) {
        
        if (option.type() != UCILoader::Button) 
            widgetLayout->addWidget(new QLabel(QString::fromStdString(option.id())));
        
        Method method = optionMethods[option.type()];
        widgetLayout->addWidget(std::invoke(method, this, option));
    };

    setLayout(widgetLayout);
}


QWidget * EngineControlPanel::addCheckBox(UCILoader::EngineOptionProxy & option) {
    QCheckBox * result = new QCheckBox(this);
    result->setChecked((bool)option);
    connect(result, &QCheckBox::checkStateChanged, [&option](Qt::CheckState state){
        bool newValue = state == Qt::Checked;
        option = newValue;
    });
    return result;
}

QWidget * EngineControlPanel::addSpinBox(UCILoader::EngineOptionProxy & proxy) {
    QSpinBox * result = new QSpinBox(this);
    const UCILoader::Option & option = proxy.getAsOption();

    result->setMinimum(option.spin_content().min);
    result->setMaximum(option.spin_content().max);
    result->setValue(option.spin_content().value);

    connect(result, &QSpinBox::valueChanged, [&proxy](int val){
        proxy = val;
    });

    return result;
}

QWidget * EngineControlPanel::addComboBox(UCILoader::EngineOptionProxy & proxy) {
    QComboBox * result = new QComboBox(this);
    const UCILoader::Option & option = proxy.getAsOption();

    for (const auto & value : option.combo_content().supported_values) {
        result->addItem(QString::fromStdString(value));
    };

    result->setCurrentText(QString::fromStdString(option.combo_content().value));

    connect(result, &QComboBox::currentTextChanged, [&proxy](const QString & str){
        proxy = str.toStdString();
    });

    return result;
}

QWidget * EngineControlPanel::addStringInput(UCILoader::EngineOptionProxy & proxy) {
    QLineEdit * result = new QLineEdit(this);
    result->setText(QString::fromStdString(proxy));
    connect(result, &QLineEdit::textChanged, [&proxy](const QString & str){
        proxy = str.toStdString();
    });
    return result;
}

QWidget * EngineControlPanel::addButton(UCILoader::EngineOptionProxy & proxy) {
    QPushButton * button = new QPushButton(QString::fromStdString(proxy.id()), this);
    connect(button, &QPushButton::clicked, [&proxy](){
        proxy.click();
    });
    return button;
}

EngineControlPanel::EngineControlPanel(const EngineServiceEntry & engineEntry, QWidget * parent) :
    QWidget(parent){
        // Keep the control panel usable when docked or resized small
        setMinimumSize(320, 240);
        setEngine(engineEntry);
    }