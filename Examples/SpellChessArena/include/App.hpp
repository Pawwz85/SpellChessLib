#pragma once 

#include <SpellChess/TimerService.h>
#include <SpellChess/Scheduler/GameScheduler.h>
#include <SpellChess/GameRunnerPool.h>

#include <QMainWindow> 
#include <QCoreApplication>
#include <QObject>
#include <QAction>

#include <BoardViewer/OpenSessionRequest.hpp>
#include <BoardViewer/GameRunnerBridge.hpp>
#include <BoardViewer/DocumentViewerModel.hpp>


#include <EngineTools/RunningEnginesService.hpp>

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

    QAction * copyFen;
    QAction * copyPgn;
    
    void openBoardViewer(BoardViewer::Model * model);
    void engineLogs(EngineServiceEntry engine);
    void engineControlPanel(EngineServiceEntry engine);
    void principalVariationPanel(EngineServiceEntry engine);
    
    std::function<void()> openTool(void (App::*)(EngineServiceEntry));

    public:

    App(QCoreApplication * coreApp);

    
    void openSession(const BoardViewer::OpenSessionRequest & request); // create new game session from request
    void openDocument(std::shared_ptr<SpellChessLib::SpellChessDocument> document); // open document in viewer

    inline QMainWindow * getWindow() {
        return mainWindow;
    }

    public slots:
    void showNewSessionBuilder();  
    void showPGN4Pastebin();
    void showEngineManager();
};