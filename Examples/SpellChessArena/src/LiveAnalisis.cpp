#include <LiveAnalisis.hpp>
#include <chrono>
#include <cassert>

#include <SpellChess/Arbiter.h>

using SearchConnection = typename UCILoader::SearchConnection<SpellChessLib::SpellChessMove>;

void
Internal::lifeAnalisisEntryPoint(LifeAnalisisThreadInfo * controlBlock){
    std::shared_ptr<SearchConnection> currentConnection = nullptr;
    auto const goParams = UCILoader::GoParamsBuilder<SpellChessLib::SpellChessMove>().withInfiniteMode(true).build();
    Internal::LifeAnalisisRequest request;

    while(controlBlock->run) {
        std::unique_lock<std::mutex> lock(controlBlock->mut);

        if (controlBlock->requestQueue.empty()) 
            goto WaitForNextRequest;

        request = controlBlock->requestQueue.front();
        controlBlock->requestQueue.pop();

        if (request.engine == nullptr) {
            controlBlock->onError(LiveAnalisisErrorCode::NoEngine);
            continue;
        }

        if (currentConnection != nullptr) {
            currentConnection->stop();
            currentConnection->waitFor(std::chrono::milliseconds(200));
            currentConnection = nullptr;
        }

        try {
            request.engine->sync();

            /*
                Start searching provided position in infinite mode. Note that awaiting for the result 
                doesn't need to happen here, as we are assuming that user will get output using engine
                repository tool's.
            */
            currentConnection = request.engine->search(goParams, 
                            SpellChessLib::StateFormatter(request.root),
                            request.movesSinceRoot);

        } catch (ChessEngine::TimeoutException e) {
            controlBlock->onError(LiveAnalisisErrorCode::EngineTimedOut);
        } catch (ChessEngine::EngineBusyException e) {
            controlBlock->onError(LiveAnalisisErrorCode::FailedToSwitchPosition);
        };

        WaitForNextRequest:
            controlBlock->conVar.wait_for(lock, std::chrono::seconds(5));
    };
}

LiveAnalisisService::LiveAnalisisService(Internal::LifeAnalisisThreadInfo * controlBlock) :
    worker(Internal::lifeAnalisisEntryPoint, controlBlock), controlBlock(controlBlock), engineId(-1) {
        controlBlock->onError = [this](LiveAnalisisErrorCode errorCode) {
            this->errorOccurred(errorCode);
        }; 
    }

LiveAnalisisService::~LiveAnalisisService() {
    controlBlock->run = false;
    worker.join();
    delete controlBlock;
}

void 
LiveAnalisisService::openEngine(const std::vector<std::string> & args, const std::string & workingDirectory) {
    auto & engineService = RunningEnginesService::getInstance();

    if (engineId != static_cast<unsigned int>(-1)) {
        engineService.removeById(engineId);
        engineId = static_cast<unsigned int>(-1);
    }

    auto engineEntry = engineService.openEngine(args, workingDirectory);
    this->engineId = engineEntry.id;
    this->configuredEngineArgs = args;
    this->configuredWorkingDirectory = workingDirectory;
    engineService.renameById(tr("Live Analisis"), engineId);
}

void
LiveAnalisisService::restoreConfiguredEngine() {
    if (!configuredEngineArgs.has_value())
        return;

    if (RunningEnginesService::getInstance().getById(engineId).has_value())
        return;

    openEngine(configuredEngineArgs.value(), configuredWorkingDirectory);
}

void 
LiveAnalisisService::analize(const SpellChessLib::GameState root, const std::vector<SpellChessLib::SpellChessMove> & movesSinceRoot) {
    
    if (SpellChessLib::arbite(root, movesSinceRoot) != SpellChessLib::ArbiterVerdict::None) 
        return;
    
    auto engineEntry = RunningEnginesService::getInstance().getById(engineId);

    if (!engineEntry.has_value()) {
        errorOccurred(LiveAnalisisErrorCode::NoEngine);
        return;
    }

    std::unique_lock<std::mutex> lock(controlBlock->mut);

    Internal::LifeAnalisisRequest request;

    request.engine = engineEntry->engine;
    request.root = root;
    request.movesSinceRoot = movesSinceRoot;
    controlBlock->requestQueue.emplace(request);

    lock.unlock();
    controlBlock->conVar.notify_one();
};


void 
LiveAnalisisService::onBoardViewerModelChanged(const BoardViewer::Model & model) {
    auto root = model.getDocument()->getRoot().lock()->getGameState();
    auto currentPos = model.getCurrentPosition();
    auto document = model.getDocument();
    analize(root, document->getMovesSinceRoot(currentPos));
};

LiveAnalisisService & 
LiveAnalisisService::getInstance(){
    static LiveAnalisisService instance(initControlBlock());
    return instance;
}

 Internal::LifeAnalisisThreadInfo * 
 LiveAnalisisService::initControlBlock() {
    Internal::LifeAnalisisThreadInfo * result = new Internal::LifeAnalisisThreadInfo;
    result->onError = [](LiveAnalisisErrorCode){};
    result->run.store(true);
    return result;
 }

#include <moc_LiveAnalisis.cpp>