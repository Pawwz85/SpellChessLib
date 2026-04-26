#pragma once

#include <SpellChess/GameRunnerConfiguration.h>
#include <GameSession/OpenSessionRequest.hpp> 
#include <EngineRepository.hpp>
#include <Form.hpp>

#include <memory>
#include <map>
#include <cstdint>

#include <QWidget>
#include <QString>
#include <QList>
#include <QTabWidget>
#include <QLabel>
#include <QLineEdit>
#include <QCheckBox>
#include <QPushButton>
#include <QComboBox>
#include <QButtonGroup>
#include <QSpinBox>
#include <QDialog>

class PlayerSelectionWidget : public Form {

    GameSession::OpenSessionRequestBuilder * requestBuilder;
    QList<EngineListEntry> availableEngines;

    QLabel       * playerLabels[2];
    QButtonGroup * isPlayerEngine[2];
    QComboBox    * selectedEngine[2];

    struct PlayerDefForm {
        bool isEngine;
        uint32_t id;
    };

    PlayerDefForm playerForms[2];

    void post() override;

    public:

    PlayerSelectionWidget(GameSession::OpenSessionRequestBuilder * reqBuilder, QWidget* parent = nullptr);

    };

class TimeSelectionWidget : public Form {
    std::shared_ptr<SpellChessLib::GameRunnerConfigurationBuilder> builder;

    QLabel      *   playerLabel[2];
    QButtonGroup*   timeControlType[2];
    QSpinBox    *   baseTime[2];
    QSpinBox    *   increment[2];

    struct TimeModeForm {
        bool infiniteMode;
        uint32_t baseTime;
        uint32_t increment;
    };

    TimeModeForm forms[2];
    void post() override;

    public:
    TimeSelectionWidget(std::shared_ptr<SpellChessLib::GameRunnerConfigurationBuilder> builder, QWidget * parent = nullptr);

};


class EngineOptionsWidget : public QWidget {
    std::shared_ptr<SpellChessLib::GameRunnerConfigurationBuilder> builder;
    QLineEdit* startingFen; 
    QSpinBox * moveTimeLimit;
    QSpinBox * depthLimit;
    QSpinBox * nodeLimit; 
    QCheckBox* allowPondering;

    public:
    EngineOptionsWidget(std::shared_ptr<SpellChessLib::GameRunnerConfigurationBuilder> builder, QWidget * parent = nullptr);
};

class GameRunnerConfigurationDialog : public QDialog {
    Q_OBJECT
    QTabWidget  * tabs;
    QPushButton * cancelBtn;
    QPushButton * submitBtn;

    GameSession::OpenSessionRequestBuilder requestBuilder;
    bool alreadySubmitted = false;

    PlayerSelectionWidget * playerForm;
    TimeSelectionWidget * timerForm;
    EngineOptionsWidget * engineOptionsForm;

    private slots:

    void trySubmit();

    public:
    GameRunnerConfigurationDialog(QWidget * parent = nullptr);
    
    GameSession::OpenSessionRequest buildRequest();
};
