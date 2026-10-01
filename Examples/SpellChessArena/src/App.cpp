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
        int maxValue = config.recursive ? model->getDocument()->size() : model->getDocument()->mainlineSize(); 
        pb->setRange(0, maxValue);
        pb->setValue(0);
        pb->setWindowModality(Qt::WindowModality::NonModal);
        pb->setCancelButton(nullptr);
        pb->setLabelText("Analising document");
        pb->setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
        
        pb->show();

        auto annotator = std::make_shared<SpellChessLib::DocumentAnnotator>(analizer, moveClassifier, config.recursive);

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
    LiveAnalisisService::getInstance().onBoardViewerModelChanged(*model);
    connect(model, &BoardViewer::Model::changed, &LiveAnalisisService::getInstance(), &LiveAnalisisService::onBoardViewerModelChanged);
    connect(copyFen, &QAction::triggered, model, &BoardViewer::Model::copyFenToClipBoard);
    connect(copyPgn, &QAction::triggered, model, &BoardViewer::Model::copyPgnToClipBoard);

    connect(this, &App::onAnalisisRequested, model, [this, model](const EngineAnalisysConfig & config) {
        this->analizeDocument(model, config);
    });

    QString pgnVersion = model->getCurrentState().isSpellChessGame() ? "PGN4" : "PGN";
    copyPgn->setText("Copy " + pgnVersion);

    boardViewer->setModel(model);

    if (!engineToolsLoaded) {
        QTimer::singleShot(0, this, [this](){
            toolHostController->loadSettings();
            engineToolsLoaded = true;
        });
    }
}

void App::openSession(const BoardViewer::OpenSessionRequest &request){
    RunningEnginesService::getInstance().clear();

    EngineTools::EngineRolesService::getInstance().clearRole(EngineTools::WhiteSideEngine);
    EngineTools::EngineRolesService::getInstance().clearRole(EngineTools::BlackSideEngine);

    BoardViewer::Model * model = new BoardViewer::GameViewerModel(timerService, request, coreQApp);
    openBoardViewer(model);
}

void App::openDocument(std::shared_ptr<SpellChessLib::SpellChessDocument> document) {
    RunningEnginesService::getInstance().clear();
    BoardViewer::Model * model = new BoardViewer::DocumentViewerModel(document);

    EngineTools::EngineRolesService::getInstance().clearRole(EngineTools::WhiteSideEngine);
    EngineTools::EngineRolesService::getInstance().clearRole(EngineTools::BlackSideEngine);

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
    (void)view->exec();
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

std::optional<EngineTools::EngineRole> querryUserForEngineRole() {
    QDialog dialog;
    dialog.setWindowTitle(QObject::tr("Select engine"));
    dialog.setModal(true);

    QVBoxLayout * layout = new QVBoxLayout(&dialog);

    QLabel * label = new QLabel(QObject::tr("Select one of the following:"), &dialog);
    layout->addWidget(label);

    QListWidget * roleList = new QListWidget(&dialog);
    roleList->setSelectionMode(QListWidget::SingleSelection);

    for (size_t i = 0; i < EngineTools::EngineRoleEnum_Size; ++i) {
        QString entryText;

        if (i == (size_t)EngineTools::LiveAnalisisEngine)
            entryText = QObject::tr("Analysis");
        else if (i == (size_t)EngineTools::WhiteSideEngine)
            entryText = QObject::tr("White");
        else if (i == (size_t)EngineTools::BlackSideEngine)
            entryText = QObject::tr("Black");

        roleList->addItem(entryText);
    }

    roleList->setCurrentRow(0);

    layout->addWidget(roleList);

    QDialogButtonBox * buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, &dialog);
    layout->addWidget(buttons);

    QObject::connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    QObject::connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    QObject::connect(roleList, &QListWidget::itemDoubleClicked, &dialog, &QDialog::accept);

    if (dialog.exec() != QDialog::Accepted)
        return std::nullopt;

    auto selectedItem = roleList->currentItem();
    if (!selectedItem)
        return std::nullopt;

    int index = roleList->row(selectedItem);
    if (index < 0 || index >= EngineTools::EngineRoleEnum_Size)
        return std::nullopt;

    return static_cast<EngineTools::EngineRole>(index);
}

 std::function<void()> App::openTool(void (App::* tool)(EngineTools::EngineRole)) {
    return [this, tool](){
        auto role = querryUserForEngineRole();
        if (role.has_value())
            std::invoke(tool, this, role.value());
    };
 };

void App::engineLogs(EngineTools::EngineRole role){
    EngineLogsWidget * logsWidget = new EngineLogsWidget(role);
    auto engine = EngineTools::EngineRolesService::getInstance().getEngineByRole(role);
    QString engineName = engine.has_value() ? engine->name : "";

    logsWidget->setWindowTitle(QObject::tr("Engine logs: %1").arg(engineName));
    logsWidget->resize(800, 600);
    logsWidget->show();
}

void App::engineControlPanel(EngineTools::EngineRole role) {
    EngineControlPanel * controlPanel = new EngineControlPanel(role);
    auto engine = EngineTools::EngineRolesService::getInstance().getEngineByRole(role);
    QString engineName = engine.has_value() ? engine->name : "";

    controlPanel->setWindowTitle(QObject::tr("Engine options: %1").arg(engineName));
    controlPanel->resize(800, 600);
    controlPanel->show();
};

void App::principalVariationPanel(EngineTools::EngineRole role) {
    PrincipalVariationPanel * panel = new PrincipalVariationPanel(role, 3);
    auto engine = EngineTools::EngineRolesService::getInstance().getEngineByRole(role);
    QString engineName = engine.has_value() ? engine->name : "";
    
    panel->setWindowTitle(QObject::tr("Engine variations: %1").arg(engineName));
    panel->resize(400, 300);
    panel->show();
}

void App::evalThermometer(EngineTools::EngineRole role) {
    EvalThermometer * thermometer = new EvalThermometer(role);
    auto engine = EngineTools::EngineRolesService::getInstance().getEngineByRole(role);
    QString engineName = engine.has_value() ? engine->name : "";

    thermometer->setAttribute(Qt::WA_DeleteOnClose);
    thermometer->setWindowTitle(QObject::tr("Evaluation thermometer: %1").arg(engineName));
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

App::~App() {
    toolHostController.reset();
    delete mainWindow;
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

    const auto addToolAction = [this, toolsMenu](EngineTools::ToolTypeId type) {
        const auto *descriptor = EngineTools::descriptorFor(type);
        QAction *addAction = toolsMenu->addAction(tr("Add %1").arg(descriptor->displayName));
        connect(addAction, &QAction::triggered, this, [this, type]() {
            const auto role = querryUserForEngineRole();
            if (role.has_value())
                toolHostController->addEmbedded(type, role.value());
        });
    };

    addToolAction(EngineTools::ToolTypeId::PrincipalVariations);
    addToolAction(EngineTools::ToolTypeId::EngineLogs);
    addToolAction(EngineTools::ToolTypeId::EngineControlPanel);
    addToolAction(EngineTools::ToolTypeId::EvalThermomether);

    toolsMenu->addSeparator();
    QAction *resetViewLayout = toolsMenu->addAction(tr("Reset view layout"));
    connect(resetViewLayout, &QAction::triggered, toolHostController.get(), &ToolHostController::resetLayout);

    QMenu * settingsMenu = menuBar->addMenu(tr("Settings"));

    QAction * manageEnginesAction = settingsMenu->addAction(tr("Manage engines"));

    connect(manageEnginesAction, &QAction::triggered, this, &App::showEngineManager);

    QAction * selectLiveAnalisisEngineAction = settingsMenu->addAction(tr("Select live analysis engine"));
    connect(selectLiveAnalisisEngineAction, &QAction::triggered, this, &App::showLiveAnalisisEngineDialog);

    mainWindow->setMenuBar(menuBar);

    BoardViewer::Model * model = new BoardViewer::DocumentViewerModel(SpellChessLib::createEmptySpellChessDocument());
    boardViewer = new BoardViewer::BoardViewerWidget(model);
    connect(flipBoard, &QAction::triggered, boardViewer, &BoardViewer::BoardViewerWidget::toggleBoardOrientation);
    
    mainWindow->setCentralWidget(boardViewer);
};


#include <moc_App.cpp>
