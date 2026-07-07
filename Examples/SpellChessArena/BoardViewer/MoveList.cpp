#include <MoveList.hpp>
#include <string>
#include <QVBoxLayout>

#include <SpellChess/SpellChessDocumentHTMLRenderer.h>

using namespace BoardViewer;


void BoardViewer::MoveListView::documentChanged(const SpellChessLib::SpellChessDocument &document, SpellChessLib::SpellChessDocument::Handle currentPos){
    doc->clear();
    auto HtmlDocument = SpellChessLib::toHtml(document);
    HtmlDocument->onCurrentPosChanged(currentPos);
    QString html = QString::fromStdString(HtmlDocument->toText());
    browser->setHtml(html);
}

MoveListView::MoveListView(QWidget *parent) : QWidget(parent){
    layout = new QVBoxLayout(this);
    browser = new QTextBrowser(this);
    doc = new QTextDocument(this);

    layout->addWidget(browser);
    setLayout(layout);
}

#include <moc_MoveList.cpp>
