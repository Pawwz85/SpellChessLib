#pragma once

#include <SpellChess/SpellChessDocument.h>
#include <SpellChess/SpellChessDocumentHTMLRenderer.h>
#include <QObject>
#include <QWidget>
#include <QTextDocument>
#include <QTextBrowser>
#include <QLayout>

namespace BoardViewer {

    class MoveListView : public QWidget {
        Q_OBJECT
        QLayout * layout;

        std::unique_ptr<SpellChessLib::HTML::Element> HtmlDocument;

        protected:
        QTextDocument * doc;
        QTextBrowser * browser;

        protected slots:
        void anchorClicked(const QUrl & anchor);

        public:

        enum NavigationDirective {
            GoToRoot,
            GoToPreviousState,
            GoToNextState,
            GoToTail
        };

        MoveListView(QWidget * parent = nullptr);

        public slots:
        void documentChanged(const SpellChessLib::SpellChessDocument & document, SpellChessLib::SpellChessDocument::Handle currentPos);
        void currentPosChanged(SpellChessLib::SpellChessDocument::Handle currentPos);

        signals:
        void nodeClicked(int nodeId);
        void navButtonClicked(NavigationDirective dir);
    };
};