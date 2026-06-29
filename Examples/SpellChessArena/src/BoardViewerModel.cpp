#include <BoardViewerModel.hpp>
#include <iostream>

BoardViewer::Model::Model(QObject * parent) : QObject(parent){};

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
BoardViewer::Model::makeMove(SpellChessLib::SpellChessMove move) {
    auto oldPos = currentPosition;
    handleMakeMove(move);

    if(currentPosition.lock() != oldPos.lock())
        changed(*this);

    if (document->clearDirtyFlag())
        documentChanged(*document);
};


#include <moc_BoardViewerModel.cpp>