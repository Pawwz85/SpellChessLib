#include <MoveList.hpp>
#include <string>
#include <QVBoxLayout>
#include <QScrollBar>

#include <SpellChess/SpellChessDocumentHTMLRenderer.h>

using namespace BoardViewer;


void BoardViewer::MoveListView::storeScrollValue() {
    QScrollBar * scrollBar = browser->verticalScrollBar();
    if (scrollBar)
        lastScrollValue = scrollBar->value();
}

void BoardViewer::MoveListView::restoreScrollValue() {
    QScrollBar * scrollBar = browser->verticalScrollBar();
    if (scrollBar)
        scrollBar->setValue(lastScrollValue);
};

void BoardViewer::MoveListView::scrollToTail() {
    QScrollBar * scrollBar = browser->verticalScrollBar();
    if (scrollBar)
        scrollBar->setValue(scrollBar->maximum());
}

void BoardViewer::MoveListView::documentChanged(const SpellChessLib::SpellChessDocument &document, SpellChessLib::SpellChessDocument::Handle currentPos){
    auto HtmlDocument = SpellChessLib::toHtml(document);
    HtmlDocument->onCurrentPosChanged(currentPos);
    QString html = QString::fromStdString(HtmlDocument->toText());

    browser->setHtml(html);

    std::string currentAnchor = "node" + std::to_string(currentPos.lock()->getNodeId()); 

    browser->scrollToAnchor(QString::fromStdString(currentAnchor));

    /*
    if (currentPos.lock() != document.getTail().lock())
        restoreScrollValue();
    else
        scrollToTail();  */

    storeScrollValue();

}

void MoveListView::anchorClicked(const QUrl & anchor) {
    QString link = anchor.toString();

    if (link.startsWith("node")) {
        QString strId = link.mid(4);
        nodeClicked(strId.toInt());
    };
};


MoveListView::MoveListView(QWidget *parent) : QWidget(parent){
    layout = new QVBoxLayout(this);
    browser = new QTextBrowser(this);
    doc = new QTextDocument(this);

    browser->setOpenLinks(false);
    browser->setFocusPolicy(Qt::NoFocus);
    
    connect(browser, &QTextBrowser::anchorClicked, this, &MoveListView::anchorClicked);

    layout->addWidget(browser);
    setLayout(layout);
}

#include <moc_MoveList.cpp>
