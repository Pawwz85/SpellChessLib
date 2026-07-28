#pragma once

#include <SpellChess/SpellChessUciLoaderPort.h>
#include <SpellChess/UCIEnginePlayer.h>
#include <RunningEnginesService.hpp>
#include <algorithm>

#include <QLocale>

unsigned int RunningEnginesService::getNextId() {
    return this->idSequence.fetch_add(1);
}

EngineServiceEntry RunningEnginesService::openEngine(const std::vector<std::string> & args, const std::string & workingDirectory){
    auto logger = std::make_shared<EngineLogger>();

    auto logger_wrapper = UCILoader::Loggers::toCallback([logger](UCILoader::Logger::MessageDirection dir, const std::string & msg){
        logger->log(msg);
    });

    auto engineProcess = std::unique_ptr<UCILoader::ProcessWrapper>(UCILoader::openProcess(args, workingDirectory)); 

    using namespace UCILoader::LoggerTraits;
    ChessEngine * engineInstance = SpellChessLib::SpellChessEngineInstanceBuilder->build(engineProcess.release(),
        std::move(logger_wrapper)  | Timestamp | Pretty
    );

    unsigned int id = getNextId();

    EngineServiceEntry result;
    result.id = id;
    result.engine = std::shared_ptr<ChessEngine>(engineInstance);
    result.logger = logger;
    result.name = "Unnamed engine";

    std::scoped_lock<std::mutex> guard(mut);
    entries.push_back(result);
    return result;
};

bool RunningEnginesService::removeById(unsigned int id) {
    std::scoped_lock<std::mutex> guard(mut);
    auto newEnd = std::remove_if(entries.begin(), entries.end(), [id](const EngineServiceEntry & item){
        return item.id == id;
    });
    bool hasAnyItemBeenErased = newEnd != entries.end();
    entries.erase(newEnd, entries.end());
    return hasAnyItemBeenErased;
};

bool RunningEnginesService::renameById(const QString & newName, unsigned int id) {
    std::scoped_lock<std::mutex> guard(mut);
    unsigned int modifiedEntriesCount = 0;
    for(auto & item: entries)
        if (item.id == id){
            modifiedEntriesCount++;
            item.name = newName;
        }
    return modifiedEntriesCount > 0;
};


void RunningEnginesService::clear(){
    std::scoped_lock<std::mutex> guard(mut);
    entries.clear();
}

std::optional<EngineServiceEntry> RunningEnginesService::getById(unsigned int id) const{
    std::scoped_lock<std::mutex> guard(mut);
    for (const auto & item : entries )
        if (item.id == id)
            return item;
    return std::nullopt;
};

const std::vector<EngineServiceEntry> RunningEnginesService::getAll() const{
    return entries;
}

RunningEnginesService & RunningEnginesService::getInstance() {
    static RunningEnginesService service;
    return service;
};

std::shared_ptr<SpellChessLib::Factory<SpellChessLib::Player>> 
RunningEnginesService::makeEnginePlayerFactory(const std::vector<std::string> & args, const std::string & workingDirectory){
    return makeEnginePlayerFactory("", args, workingDirectory);
};


class EnginePlayerFactory: public SpellChessLib::Factory<SpellChessLib::Player> {
    QString label;
    std::vector<std::string> args;
    std::string workingDirectory;

    public:

    EnginePlayerFactory(const QString & label, const std::vector<std::string> & args, const std::string workingDirectory):
        label(label), args(args), workingDirectory(workingDirectory) {};

	std::unique_ptr<SpellChessLib::Player> makeObject() override {
        auto & service = RunningEnginesService::getInstance();
        auto entry = service.openEngine(args, workingDirectory);

        auto engineInstance = entry.engine;
        engineInstance->sync();

        QString entryLabel = label.isEmpty() ? QString::fromStdString(engineInstance->getName()) : label;

        service.renameById(entryLabel, entry.id);

        return std::make_unique<SpellChessLib::UCIEnginePlayer>(engineInstance);
    };
    
};

std::shared_ptr<SpellChessLib::Factory<SpellChessLib::Player>> 
RunningEnginesService::makeEnginePlayerFactory(const QString & label, const std::vector<std::string> & args, const std::string & workingDirectory){
    return std::make_shared<EnginePlayerFactory>(label, args, workingDirectory);
};