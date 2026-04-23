#include <App.hpp>

#include <GameSession.hpp>

#include <GameRunnerConfigurationDialog.hpp>

#include <EmptyBoardWidget.hpp>

#include <QMenuBar>
#include <QMenu>

void App::openSession(const GameSession::OpenSessionRequest &request){
    GameSession::GameSessionModel * model = new GameSession::GameSessionModel(timerService, request, coreQApp);
    GameSession::GameSessionView * view = new GameSession::GameSessionView(model);
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

    mainWindow->setMenuBar(menuBar);
    
    
    connect(newGameAction, &QAction::triggered, this, &App::showNewSessionBuilder);
    // todo: addd toolbar and set default central widget
};


#include <moc_App.cpp>
