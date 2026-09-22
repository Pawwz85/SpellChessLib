#pragma once

#include <mutex>
#include <memory>
#include <atomic>
#include <condition_variable>
#include <vector>
#include <queue>
#include <functional>
#include <thread>
#include <optional>
#include <string>
#include <BoardViewer/BoardViewerModel.hpp>
#include <EngineTools/RunningEnginesService.hpp>
#include <SpellChess/Board.h>

#include <QObject> // For signal & slot mechanism

enum class LiveAnalisisErrorCode {
    NoEngine,
    FailedToSwitchPosition,
    EngineTimedOut
};

namespace Internal {

    struct LifeAnalisisRequest {
        SpellChessLib::GameState root;
        std::vector<SpellChessLib::SpellChessMove> movesSinceRoot;
        std::shared_ptr<ChessEngine> engine;
    };

    // Shared data between worker and master thread 
    struct LifeAnalisisThreadInfo {
        mutable std::mutex mut;
        std::condition_variable conVar;
        std::atomic_bool run;
        std::function<void(LiveAnalisisErrorCode)> onError;
        std::queue<LifeAnalisisRequest> requestQueue;
    };

    extern void lifeAnalisisEntryPoint(LifeAnalisisThreadInfo * controlBlock);

};

class LiveAnalisisService : public QObject {
    Q_OBJECT
    unsigned int engineId;
    std::optional<std::vector<std::string>> configuredEngineArgs;
    std::optional<Internal::LifeAnalisisRequest> lastRequest;
    std::string configuredWorkingDirectory;
    std::thread worker;

    Internal::LifeAnalisisThreadInfo * controlBlock;

    void tryLoadDefaults();
    void analize(const SpellChessLib::GameState root, const std::vector<SpellChessLib::SpellChessMove> & movesSinceRoot);

    LiveAnalisisService(Internal::LifeAnalisisThreadInfo * controlBlock);

    static Internal::LifeAnalisisThreadInfo * initControlBlock();

    public:
    ~LiveAnalisisService();

    void openEngine(const std::vector<std::string> & args, const std::string & workingDirectory = "/");

    static LiveAnalisisService & getInstance();

    public slots:
    void onBoardViewerModelChanged(const BoardViewer::Model & model); 

    signals:
    void errorOccurred(LiveAnalisisErrorCode errorCode);
    void liveEngineChanged(unsigned int oldEngineId, unsigned int newEngineId);
};


