#pragma once
#include <SpellChess/SpellChessUciLoaderPort.h>
#include <SpellChess/PlayerEntry.h>
#include <SpellChess/Board.h>
#include <SpellChess/Player.h>

#include <QWidget>

#include "EngineLogger.hpp"

#include <vector>
#include <optional>
#include <mutex>
#include <atomic>

using ChessEngine = typename UCILoader::EngineInstance<SpellChessLib::SpellChessMove>;

struct EngineServiceEntry {
    unsigned int id;    // Id of the engine stored in the service
    QString name;       // Name of the entry
    std::shared_ptr<ChessEngine> engine;
    std::shared_ptr<EngineLogger> logger;
};

// Class responsible for management of running engines
class RunningEnginesService {
    std::vector<EngineServiceEntry> entries;
    mutable std::mutex mut;
    std::atomic<unsigned int> idSequence = 0;
    unsigned int getNextId();

    public:
    
    // Create new engine instance
    EngineServiceEntry openEngine(const std::vector<std::string> & args, const std::string & workingDirectory = "/"); 

    // Remove an engine entry from the listing
    bool removeById(unsigned int id);
    bool renameById(const QString & newName, unsigned int id);
    void clear();
    std::optional<EngineServiceEntry> getById(unsigned int id) const;
    const std::vector<EngineServiceEntry> getAll() const;
    static std::shared_ptr<SpellChessLib::Factory<SpellChessLib::Player>> makeEnginePlayerFactory(const std::vector<std::string> & args, const std::string & workingDirectory = "/");
    static std::shared_ptr<SpellChessLib::Factory<SpellChessLib::Player>> makeEnginePlayerFactory(const QString & label, const std::vector<std::string> & args, const std::string & workingDirectory = "/");
    static RunningEnginesService & getInstance();
};