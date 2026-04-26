#include <SpellChess/PlayerRegistry.h>
#include <GameRunnerConfigurationDialog.hpp>

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QRadioButton>

PlayerSelectionWidget::PlayerSelectionWidget(GameSession::OpenSessionRequestBuilder * reqBuilder, QWidget *parent) : Form(parent),
 requestBuilder(reqBuilder) {
    QVBoxLayout * layout = new QVBoxLayout();
    availableEngines.append(getEnginePlayerRepositoryInstance().getAll());
    for (size_t i = 0; i < 2; ++i) {
        playerLabels[i] = new QLabel();
        isPlayerEngine[i] = new QButtonGroup(this);
        selectedEngine[i] = new QComboBox(this);
        playerForms[i] = {false, uint32_t(-1)};

        playerLabels[i]->setText(i == 0 ? "White player" : "Black player");

        
        connect(isPlayerEngine[i], &QButtonGroup::idClicked, [this, i](int id) {
            bool isEngine = id == 1;
            this->playerForms[i].isEngine = isEngine;
            this->selectedEngine[i]->setDisabled(!isEngine);
        });
        
        isPlayerEngine[i]->addButton(new QRadioButton("Human"), 0);
        isPlayerEngine[i]->addButton(new QRadioButton("Engine"), 1);
        isPlayerEngine[i]->button(0)->setChecked(true);
        isPlayerEngine[i]->setExclusive(true);

        connect(selectedEngine[i], &QComboBox::currentIndexChanged, [this, i](int index){
                this->playerForms[i].id = availableEngines[index].entry_id;
        });

        selectedEngine[i]->setDisabled(true);
        selectedEngine[i]->setPlaceholderText("Engine player");
        
        for(const auto & engine : availableEngines)
            selectedEngine[i]->addItem(QString::fromStdString(engine.name));
        
        if(availableEngines.size() > 0) {
            selectedEngine[i]->setCurrentIndex(0);
        } else
            isPlayerEngine[i]->button(1)->setDisabled(true);


        layout->addWidget(playerLabels[i]);
        layout->addWidget(isPlayerEngine[i]->button(0));
        layout->addWidget(isPlayerEngine[i]->button(1));
        layout->addWidget(selectedEngine[i]);

    };


    setLayout(layout);
}

void PlayerSelectionWidget::post() {
    using CPUdef = GameSession::CPUPlayerDefinition;
    using BuilderMethod = GameSession::OpenSessionRequestBuilder & (GameSession::OpenSessionRequestBuilder::*)(std::shared_ptr<GameSession::PlayerDefinition>);
    BuilderMethod method;
    try {
        for(size_t i = 0; i < 2; ++i){
            method = (i == 0) ? 
                    &GameSession::OpenSessionRequestBuilder::withWhitePlayer:
                    &GameSession::OpenSessionRequestBuilder::withBlackPlayer;

            if (!playerForms[i].isEngine) 
                (*requestBuilder.*method)(GameSession::defaultHumanPlayerDef);
            else {
                const auto & entry = SpellChessLib::PlayerRegistry->get(playerForms[i].id);
                (*requestBuilder.*method)(std::make_shared<CPUdef>(entry));
            }
        }
   

    } catch(SpellChessLib::PlayerRegistry_t::EntryDoesNotExistError e) {
        raiseError(e.what());
    };
    
};

TimeSelectionWidget::TimeSelectionWidget(std::shared_ptr<SpellChessLib::GameRunnerConfigurationBuilder> builder, QWidget *parent) :
    builder(builder), Form(parent) {
    QVBoxLayout * layout = new QVBoxLayout(this);
    for(size_t i = 0; i < 2; ++i) {
        playerLabel[i] = new QLabel( (i == 0) ? "White player" : "Black player");
        timeControlType[i] = new QButtonGroup(this);
        baseTime[i] = new QSpinBox();
        increment[i] = new QSpinBox();
        forms[i] = {true, 0, 0};
        
        timeControlType[i]->addButton(new QRadioButton("Infinite"), 0);
        timeControlType[i]->addButton(new QRadioButton("Simple"),   1);
        timeControlType[i]->button(0)->setChecked(true);
        connect(timeControlType[i], &QButtonGroup::idClicked, [this, i](int id){
            bool isInfinite = id == 0;
            this->baseTime[i]->setDisabled(isInfinite);
            this->increment[i]->setDisabled(isInfinite);
            this->forms[i].infiniteMode = isInfinite;
        });

        baseTime[i]->setDisabled(true);
        connect(baseTime[i], &QSpinBox::valueChanged, [this, i](int v){
            this->forms[i].baseTime = v;
        });
        
        
        increment[i]->setDisabled(true);
        connect(increment[i], &QSpinBox::valueChanged, [this, i](int v){
            this->forms[i].increment = v;
        });

        layout->addWidget(playerLabel[i]);
        layout->addWidget(timeControlType[i]->button(0));
        layout->addWidget(timeControlType[i]->button(1)); 

        QHBoxLayout * timeFieldsContainer = new QHBoxLayout(this); 
        
        QVBoxLayout * baseTimeCard = new QVBoxLayout(this);
        baseTimeCard->addWidget(new QLabel("Initial Time"));
        baseTimeCard->addWidget(baseTime[i]);
        timeFieldsContainer->addLayout(baseTimeCard);

        QVBoxLayout * incrementCard = new QVBoxLayout(this);
        incrementCard->addWidget(new QLabel("Increment"));
        incrementCard->addWidget(increment[i]);
        timeFieldsContainer->addLayout(incrementCard);

        layout->addLayout(timeFieldsContainer);

    };

    setLayout(layout);
}

void TimeSelectionWidget::post(){
    using PlayerColor = SpellChessLib::GameRunnerConfigurationBuilder::PlayerColor;

    for (size_t i = 0; i < 2; ++i) {

        PlayerColor currentColor = i == 0 ? PlayerColor::White : PlayerColor::Black;
        if (forms[i].infiniteMode) 
            builder->withInfiniteTime(currentColor);
        else {
            builder->withClassicalTimer(forms[i].baseTime, forms[i].increment, currentColor);
        }

    };

}

EngineOptionsWidget::EngineOptionsWidget(std::shared_ptr<SpellChessLib::GameRunnerConfigurationBuilder> builder, QWidget *parent) :
    QWidget(parent) {
    QVBoxLayout* layout = new QVBoxLayout(this);
    
    startingFen = new QLineEdit();
    moveTimeLimit = new QSpinBox();
    nodeLimit = new QSpinBox();
    depthLimit = new QSpinBox();
    allowPondering = new QCheckBox();
    this->builder = builder;
    connect(startingFen, &QLineEdit::textChanged, [builder](const QString & str) {
        builder->withPosition(str.toStdString());});
    startingFen->setText("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1 - - F50/f50/J20/j20");
  
    connect(moveTimeLimit, &QSpinBox::valueChanged, [builder](int v){
        builder->withMoveTimeLimit((uint32_t)v);});
    moveTimeLimit->setValue(0);
    moveTimeLimit->setMinimum(0);
    moveTimeLimit->setMaximum(3600000);
    moveTimeLimit->setSuffix(" ms");
    moveTimeLimit->setSpecialValueText("No limit");
    moveTimeLimit->setSingleStep(500);

    connect(nodeLimit, &QSpinBox::valueChanged, [builder](int v){
        builder->withNodeLimit(1000 * (uint32_t)v);});
    nodeLimit->setValue(0);
    nodeLimit->setMinimum(0);
    nodeLimit->setMaximum((INT_MAX) / 1000);
    nodeLimit->setSuffix(" knodes");
    nodeLimit->setSpecialValueText("No limit");
    nodeLimit->setSingleStep(500);
    
    connect(depthLimit, &QSpinBox::valueChanged, [builder](int v){
        builder->withDepthLimit((uint32_t)v);});
    depthLimit->setValue(0);
    depthLimit->setMinimum(0);
    depthLimit->setMaximum(99);
    depthLimit->setSuffix(" ply");
    depthLimit->setSpecialValueText("No Limit");

    connect(allowPondering, &QCheckBox::checkStateChanged, [builder](Qt::CheckState state){
        builder->withPondering(state != Qt::Unchecked);});
    allowPondering->setText("think on opponent turn");
    allowPondering->setChecked(false);
    
    layout->addWidget(new QLabel("Starting position"));
    layout->addWidget(startingFen);
    layout->addWidget(new QLabel("Move time limit"));
    layout->addWidget(moveTimeLimit);
    layout->addWidget(new QLabel("Node searched limit"));
    layout->addWidget(nodeLimit);
    layout->addWidget(new QLabel("Depth reached limit"));
    layout->addWidget(depthLimit);
    layout->addWidget(allowPondering);

    setLayout(layout);
}

GameRunnerConfigurationDialog::GameRunnerConfigurationDialog(QWidget *parent) : QDialog(parent) {
    
    auto gameRunnerConfigBuilder = requestBuilder.getGameRunnerConfigBuilder(); 
    
    QVBoxLayout * mainLayout = new QVBoxLayout(this);
    QHBoxLayout * buttonLayout = new QHBoxLayout(this);

    playerForm = new PlayerSelectionWidget(&requestBuilder, this);
    timerForm = new TimeSelectionWidget(gameRunnerConfigBuilder, this);
    engineOptionsForm = new EngineOptionsWidget(gameRunnerConfigBuilder, this);

    tabs = new QTabWidget();
    
    tabs->addTab(playerForm, "Players");
    tabs->addTab(timerForm, "Time Format");
    tabs->addTab(engineOptionsForm, "Engine Settings");

    cancelBtn = new QPushButton;
    cancelBtn->setText("cancel");

    submitBtn = new QPushButton;
    submitBtn->setText("submit");

    buttonLayout->addWidget(cancelBtn);
    buttonLayout->addWidget(submitBtn);
    mainLayout->addWidget(tabs);
    mainLayout->addLayout(buttonLayout);

    connect(submitBtn, &QPushButton::clicked, this, &GameRunnerConfigurationDialog::trySubmit);
    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);

    setLayout(mainLayout);
}

void GameRunnerConfigurationDialog::trySubmit() {
    playerForm->submit();
    timerForm->submit();
    
    bool anyError = playerForm->resetErrorFlag() || timerForm->resetErrorFlag();

    if (!anyError) {
        QDialog::accept();
    }
}

GameSession::OpenSessionRequest GameRunnerConfigurationDialog::buildRequest() {
    return requestBuilder.build();
}


#include <moc_GameRunnerConfigurationDialog.cpp>