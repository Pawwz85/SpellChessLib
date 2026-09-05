#include <BoardViewerModel.hpp>
#include <QClipboard>
#include <QGuiApplication>
#include <SpellChess/PGNExporter.h>
#include <SpellChess/PGN4Exporter.h>
#include <sstream>
#include <thread>

BoardViewer::Model::Model(QObject * parent) : QObject(parent){
    timers.whiteTimer.running = timers.blackTimer.running = false;
    timers.whiteTimer.showValue = timers.blackTimer.showValue = false;
    timers.whiteTimer.timeLeft = timers.blackTimer.timeLeft = std::chrono::milliseconds(0);
};

void 
BoardViewer::Model::setCurrentPosition(SpellChessLib::SpellChessDocument::Handle newPosition) {
    int oldId = -1, newId = -1;
    if(auto old = currentPosition.lock())
        oldId = old->getNodeId();

    if(auto newPos = newPosition.lock())
        newId = newPos->getNodeId();

    if (document->has(newPosition) && newId != oldId) {
        currentPosition = newPosition;
        changed(*this);
    };
};

SpellChessLib::SpellChessDocument::Handle
getNextPosition(SpellChessLib::SpellChessDocument::Handle h) {
    if (auto node = h.lock()) 
        return node->getFirstChildNode();
    else
        return SpellChessLib::SpellChessDocument::Handle(); 
};

SpellChessLib::SpellChessDocument::Handle
getPreviousPosition(SpellChessLib::SpellChessDocument::Handle h) {
    if (auto node = h.lock()) 
        return node->getParentNode();
    else
        return SpellChessLib::SpellChessDocument::Handle(); 
};

void
BoardViewer::Model::traverse(uint16_t plies, NextFunctor next) {
    uint16_t distance_traveled = 0;
    auto pos = currentPosition;

    while (!pos.expired() && distance_traveled++ < plies)
       pos = next(pos);
    
    if (!pos.expired()) {
        currentPosition = pos;
        changed(*this);
    }
};

void
BoardViewer::Model::goForward(uint16_t plies) {
    traverse(plies, getNextPosition);
};

void
BoardViewer::Model::goBackward(uint16_t plies) {
    traverse(plies, getPreviousPosition);
};

void
BoardViewer::Model::goToDocumentRoot() {
    setCurrentPosition(document->getRoot());
};

void
BoardViewer::Model::goToDocumentTail() {
    setCurrentPosition(document->getTail());
};

void
BoardViewer::Model::setComment(const std::string & newComment) {
    document->setComment(currentPosition, newComment);
    documentChanged(*document);
}


SpellChessLib::GameState
BoardViewer::Model::getCurrentState() const {
    return document->getGameState(getCurrentPosition());
};


void
BoardViewer::Model::makeMove(SpellChessLib::SpellChessMove move) {
    auto oldPos = currentPosition;
    handleMakeMove(move);

    if(currentPosition.lock() != oldPos.lock())
        changed(*this);

    if (document->clearDirtyFlag())
        documentChanged(*document);
};

void
BoardViewer::Model::copyFenToClipBoard() {
    const SpellChessLib::GameState & state = currentPosition.lock()->getGameState();
    QString fen = QString::fromStdString(state.toFen()); 

    QClipboard * clipboard = QGuiApplication::clipboard();
    clipboard->setText(fen);
}

void BoardViewer::Model::copyPgnToClipBoard() {
    const SpellChessLib::GameState & state = currentPosition.lock()->getGameState();
    QString result;

    std::stringstream sstream;

    SpellChessLib::Annotation::GameAnnotations annotations = document->toAnnotations();

    if (state.isSpellChessGame()) {
        SpellChessLib::PGN4Exporter exporter(&sstream);
        exporter.exportPGN4(annotations.root, annotations);
    } else {
        SpellChessLib::PGNExporter exporter(&sstream);
        exporter.exportPGN(annotations);
    }

    result = QString::fromStdString(sstream.str());

    QClipboard * clipboard = QGuiApplication::clipboard();
    clipboard->setText(result);
};

bool
BoardViewer::Model::canUserAnalizeDocumentNow() const {
    // Default behabiour:
    // User can analyze document if it is not empty

    bool isEmpty = !document || document->getRoot().lock()->isLeaf();
    return !isEmpty;
}

void
BoardViewer::Model::analyzeDocument(std::shared_ptr<SpellChessLib::DocumentAnnotator> annotator) {

    assert(canUserAnalizeDocumentNow());
    
    auto doc = this->document;
    std::thread analysisThread([this, doc, annotator](){
        (*annotator)(doc);
    });

    analysisThread.detach();
}

#include <moc_BoardViewerModel.cpp>