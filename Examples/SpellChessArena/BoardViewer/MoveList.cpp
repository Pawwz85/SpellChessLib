#include <MoveList.hpp>
#include <string>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QScrollBar>
#include <QIcon>
#include <QToolButton>
#include <QToolBar>
#include <QLabel>
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

    QString comment = QString::fromStdString(currentPos.lock()->getComment());

    if (commentEdit->toPlainText() != comment) {
        commentEdit->blockSignals(true);
        commentEdit->setPlainText(comment);
        commentEdit->blockSignals(false);
    }
    
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

    QLabel * moveListLabel = new QLabel(tr("Moves", "Game history"), this);
    moveListLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(moveListLabel);
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

    commentEdit = new QTextEdit(this);
    commentEdit->setPlaceholderText(tr("Insert comment here"));
    commentEdit->setMaximumHeight(75);
    
    commentEdit->setLineWrapMode(QTextEdit::LineWrapMode::WidgetWidth);
    layout->addWidget(commentEdit);

    connect(commentEdit, &QTextEdit::textChanged, this, [this](){
        currentCommentEdited(this->commentEdit->toPlainText());
    });

    setLayout(layout);

    HtmlDocument.reset(SpellChessLib::toHtml(*SpellChessLib::createEmptySpellChessDocument()).release());
}

#include <moc_MoveList.cpp>
