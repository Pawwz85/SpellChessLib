#pragma once 

#include <SpellChess/TimerService.h>
#include <SpellChess/Scheduler/GameScheduler.h>
#include <SpellChess/GameRunnerPool.h>

#include <QMainWindow> 
#include <QCoreApplication>
#include <QObject>

#include <GameSession/OpenSessionRequest.hpp>
#include <GameSession/GameRunnerBridge.hpp>


/*!
    Class responsible for storing configuration of current application instance
    and controls its runtime
*/
class App : public QObject {
    Q_OBJECT

    // runtime services provided by SpellChessLib library
    std::shared_ptr<SpellChessLib::TimerService> timerService;
    std::shared_ptr<SpellChessLib::Scheduler::GameScheduler> gameScheduler;
    std::shared_ptr<SpellChessLib::GameRunnerPool> gameRunnerPool;

    QMainWindow * mainWindow;
    QCoreApplication * coreQApp;

    public:

    App(QCoreApplication * coreApp);

    
    void openSession(const GameSession::OpenSessionRequest & request); // create new game session from request

    inline QMainWindow * getWindow() {
        return mainWindow;
    }

    public slots:
    void showNewSessionBuilder();  
};