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

    connect(copyFen, &QAction::triggered, model, &BoardViewer::Model::copyFenToClipBoard);
    connect(copyPgn, &QAction::triggered, model, &BoardViewer::Model::copyPgnToClipBoard);

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

    QMenu * playMenu = menuBar->addMenu(tr("Game"));
    
    QAction * newGameAction = playMenu->addAction(tr("New game", "Play new game"));
    copyFen = playMenu->addAction(tr("Copy FEN"));
    copyPgn = playMenu->addAction(tr("Copy PGN"));

    connect(newGameAction, &QAction::triggered, this, &App::showNewSessionBuilder);

    QMenu * settingsMenu = menuBar->addMenu(tr("Settings"));

    QAction * manageEnginesAction = settingsMenu->addAction(tr("Manage engines"));

    connect(manageEnginesAction, &QAction::triggered, this, &App::showEngineManager);

    mainWindow->setMenuBar(menuBar);
    
};


#include <moc_App.cpp>
