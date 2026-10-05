#include <SpellChess/PlayerRegistry.h>
#include <EngineRepository.hpp>
#include <EngineTools/RunningEnginesService.hpp>

#include <QIcon>
#include <QSettings>
#include <QWidget>
#include <QLabel>
#include <QToolBar>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QProgressDialog>
#include <QFuture>
#include <QFutureWatcher>
#include <QtConcurrent>
#include <QMessageBox>
#include <QFileDialog>
#include <stdexcept>

void EnginePlayerRepository::tryLoadFromTheDisk(){

    QSettings settings(QStringLiteral("SpellChessLib"), QStringLiteral("SpellChessArena"));
    const qsizetype engineCount = settings.beginReadArray(QStringLiteral("engines"));

    for (qsizetype index = 0; index < engineCount; ++index) {
        settings.setArrayIndex(index);

        EngineListEntry entry;
        entry.name = settings.value(QStringLiteral("name")).toString().toStdString();
        entry.pathToEngine = settings.value(QStringLiteral("executable")).toString().toStdString();
        entry.simGamesPolicy = static_cast<SpellChessLib::PlayerEntry::SimultaneousGamesPolicy>(
            settings.value(QStringLiteral("simultaneousGamesPolicy")).toInt());
        registerEntry(entry);
    }

    settings.endArray();
}

void EnginePlayerRepository::persist(){
    QSettings settings(QStringLiteral("SpellChessLib"), QStringLiteral("SpellChessArena"));
    settings.remove(QStringLiteral("engines"));
    settings.beginWriteArray(QStringLiteral("engines"));

    for (qsizetype index = 0; index < entries.size(); ++index) {
        const auto &entry = entries.at(index);
        settings.setArrayIndex(index);
        settings.setValue(QStringLiteral("name"), QString::fromStdString(entry.name));
        settings.setValue(QStringLiteral("executable"), QString::fromStdString(entry.pathToEngine));
        settings.setValue(QStringLiteral("simultaneousGamesPolicy"), static_cast<int>(entry.simGamesPolicy));
    }

    settings.endArray();
    settings.sync();
}

SpellChessLib::PlayerEntry EnginePlayerRepository::playerEntryFromEngineEntryList(const EngineListEntry &engineEntry)
{
     return {
        -1, // id, will be set by player registry repository
        engineEntry.simGamesPolicy, // simoultanous game policy
        engineEntry.name, // name
        engineEntry.pathToEngine, // executable
        RunningEnginesService::makeEnginePlayerFactory(QString::fromStdString(engineEntry.name), {engineEntry.pathToEngine})
    };
}

EnginePlayerRepository::EnginePlayerRepository(){
    tryLoadFromTheDisk();
}

void EnginePlayerRepository::addNewEngine(const EngineListEntry &newEntry){
    registerEntry(newEntry);
    persist();
    listChanged(entries);
}

void EnginePlayerRepository::registerEntry(const EngineListEntry &newEntry)
{
    auto playerEntry = playerEntryFromEngineEntryList(newEntry);
    SpellChessLib::PlayerRegistry->registerEntry(playerEntry);
    newEntry.entry_id = playerEntry.id;
    entries.append(newEntry);
}

void EnginePlayerRepository::removeEngine(const EngineListEntry &entryToRemove){
    try {
        SpellChessLib::PlayerRegistry->eraseEntry(entryToRemove.entry_id);
        auto count = entries.removeIf([&entryToRemove](const EngineListEntry & other){
            return entryToRemove.entry_id == other.entry_id;
        });
        if (count > 0) {
            persist();
            listChanged(entries);
        }
    } catch(SpellChessLib::PlayerRegistry_t::EntryDoesNotExistError ignored) {};
}

void EnginePlayerRepository::swapEntriesAt(qsizetype index1, qsizetype index2){
    entries.swapItemsAt(index1, index2);
    persist();
    listChanged(entries);
}

void EnginePlayerRepository::replaceEntryAt(qsizetype index, const EngineListEntry & entry) {
    if (index < 0 || index >= entries.size())
        throw std::out_of_range("Engine repository index is out of range");

    const uint32_t oldEntryId = entries.at(index).entry_id;
    // Check the old registry entry before registering anything. This keeps a
    // failed replacement non-destructive and prevents an orphan replacement.
    SpellChessLib::PlayerRegistry->get(oldEntryId);

    auto playerEntry = playerEntryFromEngineEntryList(entry);
    SpellChessLib::PlayerRegistry->registerEntry(playerEntry);

    try {
        SpellChessLib::PlayerRegistry->eraseEntry(oldEntryId);
    } catch (...) {
        try {
            SpellChessLib::PlayerRegistry->eraseEntry(playerEntry.id);
        } catch (...) {
        }
        throw;
    }

    EngineListEntry replacement = entry;
    replacement.entry_id = playerEntry.id;

    entries[index] = replacement;
    persist();
    listChanged(entries);
};

EngineIdentificationTask::Result EngineIdentificationTask::validate(const std::string & executablePath) {
    Result result;
    try {
        auto * process = UCILoader::openProcess({executablePath}, "/");
        result = probe(process);
    } catch (const std::exception & e) {
        result.success = false;
        result.errorMsg = e.what();
    } catch (...) {
        result.success = false;
        result.errorMsg = "Unable to open or probe the engine";
    }
    return result;
}

const EngineListEntry &EnginePlayerRepository::findById(uint32_t id) const{
    for(const auto & someEntry : entries )
        if (someEntry.entry_id == id) 
            return someEntry;
    throw EnginePlayerRepository::EntryNotFoundError(id);
}

EnginePlayerRepository &getEnginePlayerRepositoryInstance(){
    static EnginePlayerRepository repo;
    return repo;
}

void EnginePlayerRepositoryView::promptForNewEntry(){
    if (!newEngineDialog)
        newEngineDialog = new AddNewEngineDialog(this);
    else {
        newEngineDialog->reset();
    }
    newEngineDialog->show();
    connect(newEngineDialog, &QDialog::accepted, this, &EnginePlayerRepositoryView::onNewEngineDialogAccepted);
}

void EnginePlayerRepositoryView::onEngineIdentificationFinished(const EngineIdentificationTask::Result &res, const std::string & path){

    if(!res.success) {
        QMessageBox msg(this);
        msg.setText(tr("Invalid chess engine."));
        msg.setInformativeText(QString::fromStdString(res.errorMsg));
        msg.setStandardButtons(QMessageBox::Ok);
        msg.setDefaultButton(QMessageBox::Ok);
        msg.exec();
        return;
    };

    EngineListEntry entry;
    
    if(res.name != "<empty>")
        entry.name = res.name;
    else if(res.author != "<empty>")
        entry.name = tr("Chess engine by ").toStdString() + res.author;
    else
        entry.name = path;

    entry.pathToEngine = path;
    entry.simGamesPolicy = SpellChessLib::PlayerEntry::SIMULTANEOUS_GAMES_SUPPORTED;

    getEnginePlayerRepositoryInstance().addNewEngine(entry);
}

void EnginePlayerRepositoryView::removeCurrentItem()
{
    int currRow = listWidget->currentRow();
    auto & repo = getEnginePlayerRepositoryInstance();

    if (currRow >= 0 && currRow < listWidget->count()) {
        EngineListEntry entry = repo.getAll()[currRow];
        repo.removeEngine(entry);
    }
}

void EnginePlayerRepositoryView::moveCurrentItemUp(){
    int currRow = listWidget->currentRow();
    auto & repo = getEnginePlayerRepositoryInstance();
    if (currRow > 0) {
        repo.swapEntriesAt(currRow, currRow - 1);
        listWidget->setCurrentRow(currRow - 1);
    }
}

void EnginePlayerRepositoryView::moveCurrentItemDown(){
    int currRow = listWidget->currentRow();
    auto & repo = getEnginePlayerRepositoryInstance();
    if (currRow  + 1 < listWidget->count()) {
        repo.swapEntriesAt(currRow, currRow + 1);
        listWidget->setCurrentRow(currRow + 1);
    }
}

void EnginePlayerRepositoryView::editCurrentItem() {
    int currRow = listWidget->currentRow();
    auto & repo = getEnginePlayerRepositoryInstance();

    if (currRow >= 0 && currRow < listWidget->count()) {
        EngineListEntry entry = repo.getAll()[currRow];
        
        EditEngineDialog * dialog = new EditEngineDialog(entry, this);

        connect(dialog, &QDialog::accepted, this, [this, dialog, currRow](){
            auto & repo = getEnginePlayerRepositoryInstance();
            const EngineListEntry editedEntry = dialog->getResult();

            if (!dialog->hasPathChanged()) {
                repo.replaceEntryAt(currRow, editedEntry);
                dialog->deleteLater();
                return;
            }

            QProgressDialog * progressDialog = new QProgressDialog(
                tr("Scanning the engine..."), QString(), 0, 0, this);
            progressDialog->setWindowModality(Qt::WindowModal);
            progressDialog->setWindowTitle(tr("Please wait"));
            progressDialog->setMinimumDuration(500);

            using Watcher = QFutureWatcher<EngineIdentificationTask::Result>;
            auto * watcher = new Watcher(this);
            const std::string executablePath = editedEntry.pathToEngine;

            connect(watcher, &Watcher::finished, this,
                [this, watcher, progressDialog, editedEntry, currRow]() {
                    const auto result = watcher->result();
                    progressDialog->close();
                    progressDialog->deleteLater();

                    if (!result.success) {
                        QMessageBox msg(this);
                        msg.setText(tr("Invalid chess engine."));
                        msg.setInformativeText(QString::fromStdString(result.errorMsg));
                        msg.setStandardButtons(QMessageBox::Ok);
                        msg.setDefaultButton(QMessageBox::Ok);
                        msg.exec();
                    } else {
                        getEnginePlayerRepositoryInstance().replaceEntryAt(currRow, editedEntry);
                    }

                    watcher->deleteLater();
                });

            QFuture<EngineIdentificationTask::Result> future = QtConcurrent::run(
                [executablePath]() {
                    return EngineIdentificationTask::validate(executablePath);
                });
            watcher->setFuture(future);
            dialog->deleteLater();
            progressDialog->exec();
        });

        dialog->open();
    }
}

void EnginePlayerRepositoryView::onNewEngineDialogAccepted(){
    
    if(newEngineDialog == nullptr) 
        return;
    
    QProgressDialog progessDialog(tr("Scanning the engine..."), QString(), 0, 0, this);
    progessDialog.setWindowModality(Qt::WindowModal);
    progessDialog.setWindowTitle(tr("Please wait"));
    progessDialog.setMinimumDuration(500);

    std::string executablePath = newEngineDialog->getResultEntry().pathToEngine;

    using Watcher = QFutureWatcher<EngineIdentificationTask::Result>;
    Watcher * watcher = new Watcher(this);    

    connect(watcher, &Watcher::finished, this, [this, watcher, &progessDialog, executablePath](){
        auto result = watcher->result();
        progessDialog.accept();
        this->onEngineIdentificationFinished(result, executablePath);
        watcher->deleteLater(); 
    });

    QFuture<EngineIdentificationTask::Result> future = QtConcurrent::run([executablePath]() {
        return EngineIdentificationTask::validate(executablePath);
    });

    watcher->setFuture(future);
    progessDialog.exec();
}

void EnginePlayerRepositoryView::onListChanged(const QList<EngineListEntry> &newList){
    listWidget->clear();

    for (const auto & entry : newList)
        listWidget->addItem(QString::fromStdString(entry.name));

}

EnginePlayerRepositoryView::EnginePlayerRepositoryView(QWidget *parent) : QDialog(parent) {
    layout = new QVBoxLayout(this);
    QHBoxLayout * listLayout = new QHBoxLayout(this);
    listWidget = new QListWidget(this);
    
    QToolBar * toolBar = new QToolBar(this);
    QAction * newEntryAction = toolBar->addAction(QIcon::fromTheme(QIcon::ThemeIcon::ListAdd),     tr("&Add"));
    QAction * removeEntryAction = toolBar->addAction(QIcon::fromTheme(QIcon::ThemeIcon::ListRemove),  tr("&Remove"));
    QAction * moveUpAction = toolBar->addAction(QIcon::fromTheme(QIcon::ThemeIcon::GoUp),        tr("&Up"));
    QAction * moveDownAction = toolBar->addAction(QIcon::fromTheme(QIcon::ThemeIcon::GoDown),        tr("&Down"));
    QAction * editItemAction = toolBar->addAction(QIcon::fromTheme(QIcon::ThemeIcon::DocumentProperties), tr("&Edit"));

    toolBar->setOrientation(Qt::Vertical);

    connect(newEntryAction, &QAction::triggered, this, &EnginePlayerRepositoryView::promptForNewEntry);
    connect(removeEntryAction, &QAction::triggered, this, &EnginePlayerRepositoryView::removeCurrentItem);
    connect(moveUpAction, &QAction::triggered, this, &EnginePlayerRepositoryView::moveCurrentItemUp);
    connect(moveDownAction, &QAction::triggered, this, &EnginePlayerRepositoryView::moveCurrentItemDown);
    connect(editItemAction, &QAction::triggered, this, &EnginePlayerRepositoryView::editCurrentItem);

    connect(&getEnginePlayerRepositoryInstance(), &EnginePlayerRepository::listChanged, this, &EnginePlayerRepositoryView::onListChanged);
    onListChanged(getEnginePlayerRepositoryInstance().getAll());

    layout->addWidget(new QLabel(tr("Engine Management")));
    listLayout->addWidget(listWidget);
    listLayout->addWidget(toolBar);
    layout->addItem(listLayout);
    setLayout(layout);
}

AddNewEngineDialog::AddNewEngineDialog(QWidget *parent) : QDialog(parent){
    QVBoxLayout * layout  = new QVBoxLayout(this);
    QDialog::setWindowTitle(tr("Choose engine"));
    QPushButton * browse = new QPushButton(this);
    enginePathEdit = new QLineEdit(this);
    enginePathEdit->setPlaceholderText(tr("Path to engine executable"));
    
    resultEntry.entry_id = (unsigned int)-1;
    resultEntry.simGamesPolicy = SpellChessLib::PlayerEntry::SIMULTANEOUS_GAMES_SUPPORTED;
    resultEntry.name  = tr("<empty>").toStdString();
    resultEntry.pathToEngine = "";

    // browse->setIcon(QIcon::fromTheme(QIcon::ThemeIcon::DocumentOpen));
    browse->setText(tr("browse", "browse a filesystem"));
    connect(browse, &QPushButton::clicked, this, [this](){
        QString path = QFileDialog::getOpenFileName(this, tr("Select engine"), QString(), tr("Engines (*.exe)"));
        this->enginePathEdit->setText(path);
    });

    connect(enginePathEdit, &QLineEdit::textChanged, [this](const QString & val) {
        this->resultEntry.pathToEngine = val.toStdString();
    });

    

    layout->addWidget(enginePathEdit);
    layout->addWidget(browse);

    QPushButton * cancelButton = new QPushButton(tr("cancel"), this);
    QPushButton * acceptButton = new QPushButton(tr("accept"), this);
    QHBoxLayout * buttonLayout = new QHBoxLayout(this);
    buttonLayout->addWidget(cancelButton);
    buttonLayout->addWidget(acceptButton);
    layout->addLayout(buttonLayout);

    connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);
    connect(acceptButton, &QPushButton::clicked, this, &QDialog::accept);
    setLayout(layout);
}

EngineListEntry AddNewEngineDialog::getResultEntry() const{
    return resultEntry;
}

void AddNewEngineDialog::reset(){
    enginePathEdit->setText("");
}

EngineListEntry EditEngineDialog::getResult() const {
    return resultEntry;
}

bool EditEngineDialog::hasPathChanged() const {
    return resultEntry.pathToEngine != originalPath;
}

EditEngineDialog::EditEngineDialog(const EngineListEntry & entry, QWidget * parent) :
    QDialog(parent), originalPath(entry.pathToEngine), resultEntry(entry) {
    QVBoxLayout *  layout = new QVBoxLayout(this);

    nameEdit = new QLineEdit(this);
    pathToEngineEdit = new QLineEdit(this);
    QPushButton * browseButton = new QPushButton(tr("browse", "browse a filesystem"), this);
    QPushButton * acceptButton = new QPushButton(tr("accept"), this);
    QPushButton * cancelButton = new QPushButton(tr("cancel"), this);


    nameEdit->setText(QString::fromStdString(entry.name));
    pathToEngineEdit->setText(QString::fromStdString(entry.pathToEngine));

    connect(nameEdit, &QLineEdit::textChanged, this, [this](const QString & val){
        this->resultEntry.name = val.toStdString();
    });

    connect(pathToEngineEdit, &QLineEdit::textChanged, this, [this](const QString & val){
        this->resultEntry.pathToEngine = val.toStdString();
    });

    auto enginePath = pathToEngineEdit;

    connect(browseButton, &QPushButton::clicked, this, [this, enginePath]{
        QString path = QFileDialog::getOpenFileName(this, tr("Select engine"), enginePath->text(), tr("Engines (*.exe)"));
        enginePath->setText(path);
    });


    layout->addWidget(new QLabel(tr("Engine's name"), this));
    layout->addWidget(nameEdit);
    layout->addWidget(new QLabel(tr("Engine's path"), this));
    layout->addWidget(pathToEngineEdit);
    layout->addWidget(browseButton);

    QHBoxLayout * buttonLayout = new QHBoxLayout(this);
    buttonLayout->addWidget(cancelButton);
    buttonLayout->addWidget(acceptButton);
    layout->addLayout(buttonLayout);

    connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);
    connect(acceptButton, &QPushButton::clicked, this, &QDialog::accept);

    setLayout(layout);
}

#include <moc_EngineRepository.cpp>

