#pragma once

#include <SpellChess/GameRunnerConfiguration.h>
#include <BoardViewer/OpenSessionRequest.hpp> 
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

    BoardViewer::OpenSessionRequestBuilder * requestBuilder;
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

    PlayerSelectionWidget(BoardViewer::OpenSessionRequestBuilder * reqBuilder, QWidget* parent = nullptr);

};

class TimeSelectionWidget : public Form {
    std::shared_ptr<SpellChessLib::GameRunnerConfigurationBuilder> builder;

    QLabel      *   playerLabel[2];
    QButtonGroup*   timeControlType[2];
    QSpinBox    *   baseTime[2];
    QSpinBox    *   increment[2];
    QSpinBox    *   margin[2];

    struct TimeModeForm {
        bool infiniteMode;
        uint32_t baseTime;
        uint32_t increment;
        uint32_t margin;
    };

    TimeModeForm forms[2];
    void post() override;

    public:
    TimeSelectionWidget(std::shared_ptr<SpellChessLib::GameRunnerConfigurationBuilder> builder, QWidget * parent = nullptr);

};


class EngineOptionsWidget : public QWidget {
    std::shared_ptr<SpellChessLib::GameRunnerConfigurationBuilder> builder;
    QSpinBox * moveTimeLimit;
    QSpinBox * depthLimit;
    QSpinBox * nodeLimit; 
    QCheckBox* allowPondering;

    public:
    EngineOptionsWidget(std::shared_ptr<SpellChessLib::GameRunnerConfigurationBuilder> builder, QWidget * parent = nullptr);
};

class OpeningSuiteWidget : public Form {
    BoardViewer::OpenSessionRequestBuilder * builder;
    QLineEdit   *   startingFen; 
    QLineEdit   *   grimoireFilePath;
    QPushButton *   browse;

    struct Data {
        QString fen;
        QString grimoireFile;
    };
    Data data;

    void post() override;
    public:
    OpeningSuiteWidget(BoardViewer::OpenSessionRequestBuilder * builder, QWidget * parent = nullptr);

};

class GameRunnerConfigurationDialog : public QDialog {
    Q_OBJECT
    QTabWidget  * tabs;
    QPushButton * cancelBtn;
    QPushButton * submitBtn;

    BoardViewer::OpenSessionRequestBuilder requestBuilder;
    bool alreadySubmitted = false;

    PlayerSelectionWidget * playerForm;
    TimeSelectionWidget * timerForm;
    EngineOptionsWidget * engineOptionsForm;
    OpeningSuiteWidget  * openingSuiteForm;

    private slots:

    void trySubmit();

    public:
    GameRunnerConfigurationDialog(QWidget * parent = nullptr);
    
    BoardViewer::OpenSessionRequest buildRequest();
};
