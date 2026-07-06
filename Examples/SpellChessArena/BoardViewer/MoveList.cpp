#include <MoveList.hpp>
#include <string>
#include <QVBoxLayout>

#include <SpellChess/SpellChessDocumentHTMLRenderer.h>

using namespace BoardViewer;


void BoardViewer::MoveListView::documentChanged(const SpellChessLib::SpellChessDocument &document){
    doc->clear();
    QString html = QString::fromStdString(SpellChessLib::toHtml(document)->toText());
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
