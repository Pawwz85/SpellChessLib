#include <App.hpp>
#include <GameRunnerConfigurationDialog.hpp>
#include <LiveAnalisis.hpp>
#include <LiveAnalisisEngineDialog.hpp>
#include <EmptyBoardWidget.hpp>
#include <EngineRepository.hpp>
#include <PastePGN4Dialog.hpp>

#include <BoardViewer/GameViewerModel.hpp>
#include <BoardViewer/BoardViewerWidget.hpp>
#include <BoardViewer/DocumentViewerModel.hpp>


#include <QMenuBar>
#include <QMenu>
#include <QDialogButtonBox>
#include <QMessageBox>
#include <QProgressDialog>

#include <EngineTools/EngineLogger.hpp>
#include <EngineTools/EngineControlPanel.hpp>
#include <EngineTools/PrincipalVariationsPanel.hpp>
#include <EngineTools/EvalThermometer.hpp>

#include <SpellChess/PGN4Parser.h>

#include <optional>

void App::analizeDocument(BoardViewer::Model * model, const EngineAnalisysConfig & config) {
    try {
        auto entry = getEnginePlayerRepositoryInstance().findById(config.engineId);
        auto runningEngineEntry = RunningEnginesService::getInstance().openEngine({entry.pathToEngine}, ".");
        
        SpellChessLib::PositionAnalizer analizer = (config.mode == EngineAnalisysMode::FixedDepth) ?
            SpellChessLib::makeFixedDetphAnalizer(runningEngineEntry.engine, config.depthLimit) :
            SpellChessLib::makeFixedTimeAnalizer(runningEngineEntry.engine, std::chrono::milliseconds(config.timeLimitMs));

        SpellChessLib::MoveClassifier moveClassifier = SpellChessLib::SimplePositionClassifier;

        QProgressDialog * pb = new QProgressDialog(this->mainWindow->centralWidget());
        pb->setRange(0, model->getDocument()->size());
        pb->setValue(0);
        pb->setWindowModality(Qt::WindowModality::NonModal);
        pb->setCancelButton(nullptr);
        pb->setLabelText("Analising document");
        pb->setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
        
        pb->show();

        auto annotator = std::make_shared<SpellChessLib::DocumentAnnotator>(analizer, moveClassifier);

        annotator->setPositionAnalizedCallback([pb](){
            int x = pb->value();
            QMetaObject::invokeMethod(pb, &QProgressDialog::setValue, Qt::QueuedConnection, x + 1);
        });

        annotator->setFinishCallback([pb](){
            QMetaObject::invokeMethod(pb, &QProgressDialog::close, Qt::QueuedConnection);
        });

        model->analyzeDocument(annotator);
    } catch (const std::exception & e) {
        QMessageBox * alert = new QMessageBox(this->mainWindow->centralWidget());
        alert->setText(QObject::tr("Error while trying to start engine: %1").arg(e.what()));
        alert->exec();
    }
}

void App::openBoardViewer(BoardViewer::Model * model) {
    // Remove unused engines lingering from the last game
    BoardViewer::BoardViewerWidget * view = new BoardViewer::BoardViewerWidget(model);

    connect(model, &BoardViewer::Model::changed, &LiveAnalisisService::getInstance(), &LiveAnalisisService::onBoardViewerModelChanged);
    connect(copyFen, &QAction::triggered, model, &BoardViewer::Model::copyFenToClipBoard);
    connect(copyPgn, &QAction::triggered, model, &BoardViewer::Model::copyPgnToClipBoard);
    connect(flipBoard, &QAction::triggered, view, &BoardViewer::BoardViewerWidget::toggleBoardOrientation);

    connect(this, &App::onAnalisisRequested, model, [this, model](const EngineAnalisysConfig & config) {
        this->analizeDocument(model, config);
    });

    QString pgnVersion = model->getCurrentState().isSpellChessGame() ? "PGN4" : "PGN";
    copyPgn->setText("Copy " + pgnVersion);

    this->mainWindow->setCentralWidget(view);

    if (!engineToolsLoaded) {
        toolHostController->loadSettings();
        engineToolsLoaded = true;
    }
}


void App::openSession(const BoardViewer::OpenSessionRequest &request){
    RunningEnginesService::getInstance().clear();
    LiveAnalisisService::getInstance().restoreConfiguredEngine();
    BoardViewer::Model * model = new BoardViewer::GameViewerModel(timerService, request, coreQApp);
    openBoardViewer(model);
}

void App::openDocument(std::shared_ptr<SpellChessLib::SpellChessDocument> document) {
    RunningEnginesService::getInstance().clear();
    LiveAnalisisService::getInstance().restoreConfiguredEngine();
    BoardViewer::Model * model = new BoardViewer::DocumentViewerModel(document);
    openBoardViewer(model);
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

 void App::showAnalisisConfigDialog() {
    EngineAnalisisConfigDialog * dialog = new EngineAnalisisConfigDialog(mainWindow->centralWidget());

    if (dialog->exec() != QDialog::Accepted)
        return;

    auto config = dialog->getAnalisisConfig();

    onAnalisisRequested(config);
 }

void App::showLiveAnalisisEngineDialog() {
    LiveAnalisisEngineDialog dialog(mainWindow->centralWidget());
    if (dialog.exec() != QDialog::Accepted)
        return;

    const auto selectedEngineId = dialog.getSelectedEngineId();
    if (!selectedEngineId.has_value())
        return;

    try {
        const auto &entry = getEnginePlayerRepositoryInstance().findById(selectedEngineId.value());
        LiveAnalisisService::getInstance().openEngine({entry.pathToEngine}, ".");
    } catch (const std::exception &e) {
        QMessageBox::critical(mainWindow->centralWidget(), tr("Live analysis"),
                              tr("Error while trying to start the live analysis engine: %1").arg(e.what()));
    }
}

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

void App::principalVariationPanel(EngineServiceEntry engine) {
    PrincipalVariationPanel * panel = new PrincipalVariationPanel(engine.engine, 3);
    panel->setWindowTitle(QObject::tr("Engine variations: %1").arg(engine.name));
    panel->resize(400, 300);
    panel->show();
}

void App::evalThermometer(EngineServiceEntry engine) {
    EvalThermometer * thermometer = new EvalThermometer(engine.engine);
    thermometer->setAttribute(Qt::WA_DeleteOnClose);
    thermometer->setWindowTitle(QObject::tr("Evaluation thermometer: %1").arg(engine.name));
    thermometer->resize(220, 500);
    thermometer->show();
}

void App::showPGN4Pastebin() {
    PastePGN4Dialog * dialog = new PastePGN4Dialog(mainWindow->centralWidget());
    auto result = dialog->exec();

    if (result != QDialog::Accepted)
        return;

    std::optional<SpellChessLib::Annotation::GameAnnotations> annotations = std::nullopt;

    try {
        annotations = SpellChessLib::PGN4Parser::parsePgn4(dialog->getPGN4());
    } catch (SpellChessLib::InvalidPGN4 e) {
        QMessageBox * alert = new QMessageBox(dialog);
        alert->setText(e.what());
        alert->exec();
        return;
    }

    if (annotations.has_value()) {
        auto document = std::make_shared<SpellChessLib::SpellChessDocument>(annotations.value());
        openDocument(document);
    };

}


App::App(QCoreApplication * coreApp) {
    coreQApp = coreApp;
    coreQApp->setOrganizationName(QStringLiteral("SpellChessLib"));
    coreQApp->setApplicationName(QStringLiteral("SpellChessArena"));
    timerService = std::make_shared<SpellChessLib::TimerService>();
    gameScheduler =  std::make_shared<SpellChessLib::Scheduler::GameScheduler>();
    gameRunnerPool = std::make_shared<SpellChessLib::GameRunnerPool>(timerService, gameScheduler);
    
    mainWindow = new QMainWindow();
    mainWindow->setObjectName(QStringLiteral("spellChessArenaMainWindow"));
    toolHostController = std::make_unique<ToolHostController>(mainWindow, this);
    mainWindow->setCentralWidget(new EmptyBoardWidget());
    QMenuBar * menuBar = new QMenuBar();

    QMenu * playMenu = menuBar->addMenu(tr("Game"));
    
    QAction * newGameAction = playMenu->addAction(tr("New game", "Play new game"));
    QAction * openPGN4Action = playMenu->addAction(tr("Load PGN4", "Load PGN4 document"));
    copyFen = playMenu->addAction(tr("Copy FEN"));
    copyPgn = playMenu->addAction(tr("Copy PGN"));
    QAction * analiseGameAction = playMenu->addAction(tr("Analise game", "Request computer analisis"));

    connect(newGameAction, &QAction::triggered, this, &App::showNewSessionBuilder);
    connect(openPGN4Action, &QAction::triggered, this, &App::showPGN4Pastebin);
    connect(analiseGameAction,  &QAction::triggered, this, &App::showAnalisisConfigDialog);

    QMenu * boardMenu = menuBar->addMenu(tr("Board", "Menu for chessboard customizations"));
    flipBoard = boardMenu->addAction(tr("Flip board", "Switch board perspective"));

    QMenu * toolsMenu = menuBar->addMenu(tr("Views"));

    QMenu * embeddedToolsMenu = toolsMenu->addMenu(tr("Embedded engine tools"));

    const auto addEmbeddedToolActions = [this, embeddedToolsMenu](EngineTools::ToolTypeId type) {
        const auto *descriptor = EngineTools::descriptorFor(type);
        QAction *addAction = embeddedToolsMenu->addAction(tr("Add %1").arg(descriptor->displayName));
        connect(addAction, &QAction::triggered, this, [this, type]() {
            const auto engine = querryUserForEngineInstance();
            if (engine.has_value())
                toolHostController->addEmbedded(type, engine.value());
        });

        QAction *standaloneAction = embeddedToolsMenu->addAction(tr("Open %1 standalone").arg(descriptor->displayName));
        connect(standaloneAction, &QAction::triggered, this, [this, type]() {
            const auto engine = querryUserForEngineInstance();
            if (engine.has_value())
                toolHostController->openStandalone(type, engine.value());
        });

        QAction *removeAction = embeddedToolsMenu->addAction(tr("Remove %1").arg(descriptor->displayName));
        connect(removeAction, &QAction::triggered, this, [this, type]() {
            toolHostController->removeEmbedded(type);
        });
    };

    addEmbeddedToolActions(EngineTools::ToolTypeId::PrincipalVariations);
    addEmbeddedToolActions(EngineTools::ToolTypeId::EngineLogs);
    addEmbeddedToolActions(EngineTools::ToolTypeId::EngineControlPanel);
    embeddedToolsMenu->addSeparator();
    QAction *resetEmbeddedTools = embeddedToolsMenu->addAction(tr("Reset embedded tool layout"));
    connect(resetEmbeddedTools, &QAction::triggered, toolHostController.get(), &ToolHostController::resetLayout);

    QAction * showEngineLogs = toolsMenu->addAction(tr("Debug engine", "Show UCI protocol of an engine"));
    connect(showEngineLogs, &QAction::triggered, this, openTool(&App::engineLogs));

    QAction * showEngineOptions = toolsMenu->addAction(tr("Engine Options", "Show detected options of UCI engine"));
    connect(showEngineOptions, &QAction::triggered, this, openTool(&App::engineControlPanel));

    QAction * showPvPanel = toolsMenu->addAction(tr("Principal variation"));
    connect(showPvPanel, &QAction::triggered, this, openTool(&App::principalVariationPanel));

    QAction * showEvalThermometer = toolsMenu->addAction(tr("Evaluation thermometer"));
    connect(showEvalThermometer, &QAction::triggered, this, openTool(&App::evalThermometer));

    QMenu * settingsMenu = menuBar->addMenu(tr("Settings"));

    QAction * manageEnginesAction = settingsMenu->addAction(tr("Manage engines"));

    connect(manageEnginesAction, &QAction::triggered, this, &App::showEngineManager);

    QAction * selectLiveAnalisisEngineAction = settingsMenu->addAction(tr("Select live analysis engine"));
    connect(selectLiveAnalisisEngineAction, &QAction::triggered, this, &App::showLiveAnalisisEngineDialog);

    mainWindow->setMenuBar(menuBar);
    
};


#include <moc_App.cpp>
