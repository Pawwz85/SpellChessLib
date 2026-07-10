#include <MoveList.hpp>
#include <string>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QScrollBar>
#include <QIcon>
#include <QToolButton>
#include <QToolBar>
#include <SpellChess/SpellChessDocumentHTMLRenderer.h>

using namespace BoardViewer;

const size_t navDirectiveCount = 4;

const QIcon::ThemeIcon navIcons[navDirectiveCount] = {
    QIcon::ThemeIcon::MediaSkipBackward,
    QIcon::ThemeIcon::MediaSeekBackward,
    QIcon::ThemeIcon::MediaSeekForward,
    QIcon::ThemeIcon::MediaSkipForward
};

void BoardViewer::MoveListView::documentChanged(const SpellChessLib::SpellChessDocument &document, SpellChessLib::SpellChessDocument::Handle currentPos){
    HtmlDocument.reset(SpellChessLib::toHtml(document).release());
    currentPosChanged(currentPos);
}

void BoardViewer::MoveListView::currentPosChanged(SpellChessLib::SpellChessDocument::Handle currentPos) {
    HtmlDocument->onCurrentPosChanged(currentPos);
    QString html = QString::fromStdString(HtmlDocument->toText());
    browser->setHtml(html);
    std::string currentAnchor = "node" + std::to_string(currentPos.lock()->getNodeId());
    browser->scrollToAnchor(QString::fromStdString(currentAnchor));
};

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

    QToolBar * navBox = new QToolBar(this);

    for(size_t i = 0; i < navDirectiveCount; ++i) {
        auto navButton = new QToolButton(this);
        navButton->setIcon(QIcon::fromTheme(navIcons[i]));

        connect(navButton, &QToolButton::clicked, this, [this, i](){
            this->navButtonClicked((NavigationDirective)i);
        });

        navBox->addWidget(navButton);
    }

    layout->addWidget(navBox);
    setLayout(layout);
}

#include <moc_MoveList.cpp>
