#pragma once

#include <memory>
#include <functional>
#include <mutex>
#include <cstdint>

#include <SpellChess/SpellChessDocument.h>
#include <SpellChess/SpellChessGame.h> // for TimerPairState

#include <QObject>

#include "SpellChessMoveBuilder.hpp"

namespace BoardViewer {

    class Model : public QObject {
        Q_OBJECT

        virtual void handleMakeMove(SpellChessLib::SpellChessMove move) = 0;

        protected:
        std::recursive_mutex mut;
        std::shared_ptr<SpellChessLib::SpellChessDocument> document;
        SpellChessLib::SpellChessDocument::Handle currentPosition;
        
        SpellChessLib::TimerPairState timers;

        using NextFunctor = typename std::function<SpellChessLib::SpellChessDocument::Handle(SpellChessLib::SpellChessDocument::Handle)>;

        void traverse(uint16_t plies, NextFunctor next);
        public:

        Model(QObject * parent = nullptr);
        
        // sets new position in the current document to view
        virtual void setCurrentPosition(SpellChessLib::SpellChessDocument::Handle newPosition);
        
        // make move in current position and emit signals on change
        void makeMove(SpellChessLib::SpellChessMove move);
        
        // Go *exactly* 'plies' forward or noop
        void goForward(uint16_t plies);
       
        // Go *exactly* 'plies' backward or noop
        void goBackward(uint16_t plies);
       
        // Go to the document's tail
        void goToDocumentTail();

        // Go to document's root
        void goToDocumentRoot();
        

        virtual SpellChessLib::SpellChessDocument::Handle getCurrentPosition() const = 0;
        virtual SpellChessLib::GameState getCurrentState() const; 
        virtual std::shared_ptr<const SpellChessLib::SpellChessDocument> getDocument() const = 0;
        virtual bool canUserSubmitMove() = 0;

        signals:
        void changed(const Model & model);
        void documentChanged(const SpellChessLib::SpellChessDocument & document);
        void onTimerChanged(const SpellChessLib::TimerPairState & timers);
    };

    
};