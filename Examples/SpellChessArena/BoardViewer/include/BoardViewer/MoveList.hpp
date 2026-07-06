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

        protected:
        QTextDocument * doc;
        QTextBrowser * browser;

        public:
        MoveListView(QWidget * parent = nullptr);

        public slots:
        void documentChanged(const SpellChessLib::SpellChessDocument & document);
    };
};