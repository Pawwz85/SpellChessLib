#pragma once

#include <SpellChess/SpellChessDocument.h>
#include <QObject>
#include <QWidget>
#include <QTextDocument>
#include <QTextBrowser>
#include <QLayout>

namespace BoardViewer {
    class MoveListView : public QWidget {
        Q_OBJECT
        QLayout * layout;

        int lastScrollValue = 0;

        void storeScrollValue();
        void scrollToTail();
        void restoreScrollValue();

        protected:
        QTextDocument * doc;
        QTextBrowser * browser;

        protected slots:
        void anchorClicked(const QUrl & anchor);

        public:
        MoveListView(QWidget * parent = nullptr);

        public slots:
        void documentChanged(const SpellChessLib::SpellChessDocument & document, SpellChessLib::SpellChessDocument::Handle currentPos);
    
        signals:
        void nodeClicked(int nodeId);
    };
};