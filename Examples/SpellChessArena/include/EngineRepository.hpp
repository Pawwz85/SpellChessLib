#pragma once
#include <memory>
#include <string>
#include <chrono>

#include <cereal/archives/json.hpp>
#include <SpellChess/PlayerEntry.h>
#include <SpellChess/UCIEnginePlayer.h>
#include <SpellChess/PlayerRegistry.h> // for EntryNotFoundError
#include <SpellChess/SpellChessUciLoaderPort.h> // necessary to use UCILoader with spell chess 

#include <UCILoader/EngineConnection.h>

#include <QObject>
#include <QList>
#include <QWidget>
#include <QDialog>
#include <QLayout>
#include <QListWidget>
#include <QToolButton>
#include <QLineEdit>


class EnginePlayerRepository;

struct EngineListEntry {
    friend EnginePlayerRepository;    
    std::string name;
    std::string pathToEngine;
    SpellChessLib::PlayerEntry::SimultaneousGamesPolicy simGamesPolicy;
    mutable uint32_t entry_id; 
};

template <class Archive>
void serialize(Archive & ar, EngineListEntry & entry) {
    ar( cereal::make_nvp("name", entry.name),
        cereal::make_nvp("executable", entry.pathToEngine),
        cereal::make_nvp("simoultanousGamePolicy", entry.simGamesPolicy)
    );
};


class EnginePlayerRepository : public QObject {
    Q_OBJECT

    QList<EngineListEntry> entries;

    void tryLoadFromTheDisk();
    void persist();

    SpellChessLib::PlayerEntry playerEntryFromEngineEntryList(const EngineListEntry & engineEntry);

    void registerEntry(const EngineListEntry &newEntry);

    public:
    using EntryNotFoundError = SpellChessLib::PlayerRegistry_t::EntryDoesNotExistError;
    EnginePlayerRepository();

    void addNewEngine(const EngineListEntry &newEntry);
    void removeEngine(const EngineListEntry & entryToRemove);
    void swapEntriesAt(qsizetype index1, qsizetype index2);
    void replaceEntryAt(qsizetype index, const EngineListEntry & entry);

    const EngineListEntry & findById(uint32_t id) const;

    inline const QList<EngineListEntry> & getAll() const {
        return entries;
    }

    inline QList<EngineListEntry>::const_reference at(qsizetype i) const{
        return entries.at(i);
    }

    inline QList<EngineListEntry>::reference operator[](qsizetype i) {
        return entries[i];
    }

    signals:
    void listChanged(const QList<EngineListEntry> & entries);
};


extern EnginePlayerRepository & getEnginePlayerRepositoryInstance();

class EngineIdentificationTask {
    public:

    struct Result {
        bool success = false;
        std::string name;
        std::string author;
        // todo: add options collection
        std::string errorMsg;
    };

    static Result probe(UCILoader::ProcessWrapper * process) {
        Result result;
        using EngineInstance = typename UCILoader::EngineInstance<SpellChessLib::SpellChessMove>;
        EngineInstance * engine = nullptr;
        try {
            engine = SpellChessLib::SpellChessEngineInstanceBuilder->build(process);
            
            engine->sync(std::chrono::seconds(5));

            result.success  = true;
            result.name     = engine->getName();
            result.author   = engine->getAuthor();

        } catch(const EngineInstance::TimeoutException & e){
            result.success = false;
            result.errorMsg = "Engine timed out (not a UCI engine?)";
        } catch (const std::exception& e) {
            result.success = false;
            result.errorMsg = e.what();
        };

        if(engine) 
            delete engine;
        return result;
    };

};

class AddNewEngineDialog;
// class EditEngineDialog;

class EnginePlayerRepositoryView : public QWidget {
    Q_OBJECT
    QLayout * layout;

    QListWidget * listWidget;
    AddNewEngineDialog * newEngineDialog;

    void onEngineIdentificationFinished(const EngineIdentificationTask::Result & res, const std::string & path);
    private slots:
    void promptForNewEntry();
    void removeCurrentItem();
    void moveCurrentItemUp();
    void moveCurrentItemDown();
    void editCurrentItem();
    void onNewEngineDialogAccepted();
    void onListChanged(const QList<EngineListEntry> & newList);
    public:
    EnginePlayerRepositoryView(QWidget * parent = nullptr);
};

class AddNewEngineDialog : public QDialog {
     QLayout * layout;
     EngineListEntry resultEntry;

     QLineEdit * enginePathEdit;

    public:
     AddNewEngineDialog(QWidget * parent = nullptr);
     EngineListEntry getResultEntry() const;
     void reset();
};

class EditEngineDialog : public QDialog {
    QLineEdit * nameEdit;
    QLineEdit * pathToEngineEdit;
    
    bool pathChanged = false;
    EngineListEntry resultEntry;

    public:
    EditEngineDialog(const EngineListEntry & entry, QWidget * parent = nullptr);

    EngineListEntry getResult() const;
    bool hasPathChanged() const;
};  