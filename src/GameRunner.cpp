#include <SpellChess/GameRunner.h>

void SpellChessLib::GameRunner::setCurrentPlayer(Player* p){
    std::lock_guard<std::mutex> guard(lock);
    
    if (p == nullptr){
        currentPlayer = nullptr;
        flowManager->unregisterBlockingTask();
      
        if (blockingTask != nullptr) {
            delete blockingTask;
            blockingTask = nullptr;
        }
    }  
    else if (blockingTask == nullptr) {
        currentPlayer = p;
        blockingTask = new _PlayerWrapper(p);
        flowManager->registerBlockingTask(blockingTask);
    }
   
}

void SpellChessLib::GameRunner::stop() {
    do {
        std::lock_guard<std::mutex> guard(lock);
        running = false;
    } while (false);
   
    flowManager->askBlockingTaskToReturnControlToParentThread();
}

void SpellChessLib::GameRunner::attachGameMonitor(std::shared_ptr<GameMonitor> monitor){
    monitor->onGameRunnerConnect();
    gameMonitor = monitor;
    gameMonitor->onGameValueChanged(*game);
}


SpellChessLib::Game* SpellChessLib::GameRunner::gameFromConfig(const GameRunnerConfiguration& config, std::shared_ptr<TimerService> timeServ)
{
    FenParser parser;
    StandardChessFenValidator fenValidator;
    SpellChessFenValidator spellChessValidator;

    GameState state;

    try {
        state = parser.fromSpellChessFen(config.rootFen.c_str());
    }
    catch ( FenValidator::InvalidFenException e) {
        state = parser.fromStandardChessFen(config.rootFen.c_str());
    } 

    std::unique_ptr<TimerController> whTimer = GameRunnerConfiguration::createTimerFromInfo(timeServ, config.whPlayerTimerInfo);
    std::unique_ptr<TimerController> blTimer = GameRunnerConfiguration::createTimerFromInfo(timeServ, config.blPlayerTimerInfo);
    auto result = new Game(state, std::move(whTimer), std::move(blTimer));
    
    result->getGoParamsBuilder()
        .withMoveTime(config.stopConditions.moveTime)
        .withDepth(config.stopConditions.depth)
        .withNodes(config.stopConditions.nodes);

    auto sevenTagRoster = Annotation::beginSevenTagRooster(config.event, config.site, config.round);
    result->setSevenTagRooster(sevenTagRoster);
    return result;
}

SpellChessLib::GameRunner::~GameRunner(){
    if (blockingTask)
        delete blockingTask;

    if (gameMonitor != nullptr)
        gameMonitor->onGameRunnerDisconnect();
}

void SpellChessLib::GameRunner::run(){
    GameState root = game->getGameState();
    std::vector<SpellChessMove> moves;
    running = true;
    
    auto stop_clb = std::static_pointer_cast<Game::ResultObserver>(std::make_shared<_StopCallback>(this));

    if (!whPlayer->isAlive())
        whPlayer.realoc();
    if (!blPlayer->isAlive())
        blPlayer.realoc();
    whPlayer->onNewGame();
    blPlayer->onNewGame();
    game->addResultObserver(stop_clb);
    game->startCurrentPlayerTimer();
    PlayerColor movingSide;

    if (gameMonitor) 
        gameMonitor->onGameValueChanged(*game);

    while (!game->isFinished() && running) {
        movingSide = game->getGameState().getSideToMove();
        auto& currentPlayer = movingSide == White ? whPlayer : blPlayer;
        setCurrentPlayer(currentPlayer.get());
        
        try {
            Player::Result result = currentPlayer->search(*game);
            setCurrentPlayer(nullptr);
            game->makeMove(result.getBestMove());
       
            if (gameMonitor) 
                gameMonitor->onGameValueChanged(*game);

            if (game->isFinished())
                break;

            try {
                if (allowPondering && currentPlayer->supportsPondering() && result.hasPonderMove())
                    currentPlayer->ponder(*game, result.getPonderMove());
            }
            catch (std::exception e) {
                game->resign(flipColor(movingSide), ExceptionThrownByAPlayer);
                break;
            }  
        }
        catch (std::exception e) {
            game->resign(movingSide, ExceptionThrownByAPlayer);
            break;
        }

    }
    setCurrentPlayer(nullptr);

    if (whPlayer->isPondering())
        whPlayer->interrupt();

    if (blPlayer->isPondering())
        blPlayer->interrupt();

    if (gameMonitor)
        gameMonitor->onGameValueChanged(*game);
}

bool SpellChessLib::GameRunner::isRunning() const{
    return running;
}

void SpellChessLib::GameRunner::_StopCallback::onGameOutcome(const PlayerColor& winner, const GameStatusCode& outcome){
    // inform parent the game has ended, particularly useful to inform game runner about player being timed out
    parent->stop();
}

void SpellChessLib::GameRunner::_PlayerWrapper::interrupt(){
    p->interrupt();
}

void SpellChessLib::GameRunner::_PlayerWrapper::terminate(){
    p->kill();
}
