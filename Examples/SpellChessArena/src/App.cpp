#include <App.hpp>
#include <GameRunnerConfigurationDialog.hpp>
#include <EmptyBoardWidget.hpp>
#include <EngineRepository.hpp>

#include <BoardViewer/GameViewerModel.hpp>
#include <BoardViewer/BoardViewerWidget.hpp>


#include <QMenuBar>
#include <QMenu>
#include <QDialogButtonBox>

#include <EngineTools/EngineLogger.hpp>
#include <EngineTools/EngineControlPanel.hpp>
#include <optional>

void App::openSession(const BoardViewer::OpenSessionRequest &request){

    // Remove unused engines lingering from the last game
    RunningEnginesService::getInstance().clear();

    BoardViewer::Model * model = new BoardViewer::GameViewerModel(timerService, request, coreQApp);
    BoardViewer::BoardViewerWidget * view = new BoardViewer::BoardViewerWidget(model);

    connect(copyFen, &QAction::triggered, model, &BoardViewer::Model::copyFenToClipBoard);
    connect(copyPgn, &QAction::triggered, model, &BoardViewer::Model::copyPgnToClipBoard);

    QString pgnVersion = model->getCurrentState().isSpellChessGame() ? "PGN4" : "PGN";
    copyPgn->setText("Copy " + pgnVersion);

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

std::optional<EngineServiceEntry> querryUserForEngineInstance() {
    const auto entries = RunningEnginesService::getInstance().getAll();

    QDialog dialog;
    dialog.setWindowTitle(QObject::tr("Select engine"));
    dialog.setModal(true);

    QVBoxLayout * layout = new QVBoxLayout(&dialog);

    QLabel * label = new QLabel(QObject::tr("Select one of the currently running engines:"), &dialog);
    layout->addWidget(label);

    QListWidget * engineList = new QListWidget(&dialog);
    engineList->setSelectionMode(QListWidget::SingleSelection);

    for (const auto & entry : entries) {
        QString entryText = entry.name.isEmpty()
            ? QObject::tr("Engine %1").arg(entry.id)
            : entry.name;
        engineList->addItem(entryText + " (" + QString::number(entry.id) + ")");
    }

    if (!entries.empty())
        engineList->setCurrentRow(0);

    layout->addWidget(engineList);

    QDialogButtonBox * buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, &dialog);
    layout->addWidget(buttons);

    QObject::connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    QObject::connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    QObject::connect(engineList, &QListWidget::itemDoubleClicked, &dialog, &QDialog::accept);

    if (dialog.exec() != QDialog::Accepted)
        return std::nullopt;

    auto selectedItem = engineList->currentItem();
    if (!selectedItem)
        return std::nullopt;

    int index = engineList->row(selectedItem);
    if (index < 0 || index >= static_cast<int>(entries.size()))
        return std::nullopt;

    return entries[index];
}

 std::function<void()> App::openTool(void (App::* tool)(EngineServiceEntry)) {
    return [this, tool](){
        auto result = querryUserForEngineInstance();
        if (result.has_value())
            std::invoke(tool, this, result.value());
    };
 };

void App::engineLogs(EngineServiceEntry engine){
    EngineLogsWidget * logsWidget = new EngineLogsWidget(*engine.logger);
    logsWidget->setWindowTitle(QObject::tr("Engine logs: %1").arg(engine.name));
    logsWidget->resize(800, 600);
    logsWidget->show();
}

void App::engineControlPanel(EngineServiceEntry engine) {
    EngineControlPanel * controlPanel = new EngineControlPanel(engine.engine);
    controlPanel->setWindowTitle(QObject::tr("Engine options: %1").arg(engine.name));
    controlPanel->resize(800, 600);
    controlPanel->show();
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

    QMenu * toolsMenu = menuBar->addMenu(tr("Views"));

    QAction * showEngineLogs = toolsMenu->addAction(tr("Debug engine", "Show UCI protocol of an engine"));
    connect(showEngineLogs, &QAction::triggered, this, openTool(&App::engineLogs));

    QAction * showEngineOptions = toolsMenu->addAction(tr("Engine Options", "Show detected options of UCI engine"));
    connect(showEngineOptions, &QAction::triggered, this, openTool(&App::engineControlPanel));

    QMenu * settingsMenu = menuBar->addMenu(tr("Settings"));

    QAction * manageEnginesAction = settingsMenu->addAction(tr("Manage engines"));

    connect(manageEnginesAction, &QAction::triggered, this, &App::showEngineManager);

    mainWindow->setMenuBar(menuBar);
    
};


#include <moc_App.cpp>
