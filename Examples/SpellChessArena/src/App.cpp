#include <App.hpp>
#include <GameRunnerConfigurationDialog.hpp>
#include <EmptyBoardWidget.hpp>
#include <EngineRepository.hpp>

#include <BoardViewer/GameViewerModel.hpp>
#include <BoardViewer/BoardViewerWidget.hpp>


#include <QMenuBar>
#include <QMenu>

void App::openSession(const BoardViewer::OpenSessionRequest &request){
    BoardViewer::Model * model = new BoardViewer::GameViewerModel(timerService, request, coreQApp);
    BoardViewer::BoardViewerWidget * view = new BoardViewer::BoardViewerWidget(model);
    this->mainWindow->setCentralWidget(view);
}

void App::showNewSessionBuilder() {

    GameRunnerConfigurationDialog * dialog = new GameRunnerConfigurationDialog(mainWindow->centralWidget());
    dialog->show();

    connect(dialog, &QDialog::accepted, this, [this, dialog](){
        openSession(dialog->buildRequest()); 
        dialog->deleteLater();
    });
};

void App::showEngineManager() {
    EnginePlayerRepositoryView * view = new EnginePlayerRepositoryView();
    mainWindow->setCentralWidget(view);
};


App::App(QCoreApplication * coreApp) {
    coreQApp = coreApp;
    timerService = std::make_shared<SpellChessLib::TimerService>();
    gameScheduler =  std::make_shared<SpellChessLib::Scheduler::GameScheduler>();
    gameRunnerPool = std::make_shared<SpellChessLib::GameRunnerPool>(timerService, gameScheduler);
    
    mainWindow = new QMainWindow();
    mainWindow->setCentralWidget(new EmptyBoardWidget());
    QMenuBar * menuBar = new QMenuBar();

    QMenu * playMenu = menuBar->addMenu(tr("Play"));
    
    QAction * newGameAction = playMenu->addAction(tr("New game", "Play new game"));

    connect(newGameAction, &QAction::triggered, this, &App::showNewSessionBuilder);

    QMenu * settingsMenu = menuBar->addMenu(tr("Settings"));

    QAction * manageEnginesAction = settingsMenu->addAction(tr("Manage engines"));

    connect(manageEnginesAction, &QAction::triggered, this, &App::showEngineManager);

    mainWindow->setMenuBar(menuBar);
    
};


#include <moc_App.cpp>
