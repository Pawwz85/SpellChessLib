#include <LiveAnalisis.hpp>
#include <EngineRepository.hpp>

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

        request = controlBlock->requestQueue.back();
        
        while (controlBlock->requestQueue.size())
            controlBlock->requestQueue.pop();

        if (request.engine == nullptr) {
            controlBlock->onError(LiveAnalisisErrorCode::NoEngine);
            continue;
        }

        if (currentConnection != nullptr) {
            currentConnection->stop();

            // UCILoader 1.2.1-alpha waits correctly after stop(), avoiding a
            // polling loop while the engine emits the terminating bestmove.
            lock.unlock();
            currentConnection->waitFor(std::chrono::seconds(2));
            lock.lock();

            auto status = currentConnection->getStatus();
            if (status != UCILoader::SearchStatusCode::ResultReady &&
                status != UCILoader::SearchStatusCode::Terminated &&
                status != UCILoader::SearchStatusCode::Stopped) {
                controlBlock->onError(LiveAnalisisErrorCode::FailedToSwitchPosition);
                currentConnection = nullptr;
                goto WaitForNextRequest;
            }

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

        } catch (const ChessEngine::TimeoutException & e) {
            controlBlock->onError(LiveAnalisisErrorCode::EngineTimedOut);
        } catch (const ChessEngine::EngineBusyException & e) {
            controlBlock->onError(LiveAnalisisErrorCode::FailedToSwitchPosition);
        };

        WaitForNextRequest:
        if (controlBlock->requestQueue.empty())
            controlBlock->conVar.wait_for(lock, std::chrono::seconds(5));
    };
}

LiveAnalisisService::LiveAnalisisService(Internal::LifeAnalisisThreadInfo * controlBlock) :
    worker(Internal::lifeAnalisisEntryPoint, controlBlock), controlBlock(controlBlock), engineId(-1) {
        controlBlock->onError = [this](LiveAnalisisErrorCode errorCode) {
            this->errorOccurred(errorCode);
        }; 
        tryLoadDefaults();
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
    engineService.makeEntryDurableById(engineEntry.id);
    this->engineId = engineEntry.id;
    this->configuredEngineArgs = args;
    this->configuredWorkingDirectory = workingDirectory;
    engineService.renameById(tr("Live Analisis"), engineId);
}

void 
LiveAnalisisService::tryLoadDefaults() {
    
    auto configuredEngines = getEnginePlayerRepositoryInstance().getAll();

    if (!configuredEngines.isEmpty()) {
        openEngine({configuredEngines[0].pathToEngine}, ".");
    };

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