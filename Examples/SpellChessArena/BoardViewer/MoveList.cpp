#include <MoveList.hpp>
#include <QTextCursor>
#include <sstream>
#include <vector>
#include <string>
#include <QVBoxLayout>

using namespace BoardViewer;

MoveListModel::MoveListModel() {
    SpellChessLib::FenParser parser;
    initialPosition = parser.fromSpellChessFen(SpellChessLib::SPELL_START_POS);
};

void MoveListView::writeAnnotationGlyph(QTextCursor &cursor, const SpellChessLib::Annotation::NumericAnnotationGlyph &glyph){
    // TODO: implement
}

void MoveListView::writeMoveToken(QTextCursor &cursor, const SpellChessLib::SpellChessMove *move){
    std::string token = move ? SpellChessLib::spellChessMoveToString(*move) : "...";
    cursor.insertText(QString::fromStdString(token));
}

void MoveListView::writeVariation(QTextCursor &cursor, const SpellChessLib::Annotation::AnnotatedSequence &variation){
    // TODO: implement
}

void MoveListView::writeComment(QTextCursor &cursor, const std::string &comment)
{
    cursor.insertText(QString::fromStdString(comment));
}

void MoveListView::writeTab(QTextCursor &cursor)
{
    cursor.insertText("\t");
}

void MoveListView::writeNumberIndicator(QTextCursor &cursor, int indicator){
    std::string token = std::to_string(indicator) + ".";
    cursor.insertText(QString::fromStdString(token));
}



void MoveListView::writeExtras(QTextCursor &cursor, MoveListModel::Sequence::const_iterator begin, MoveListModel::Sequence::const_iterator end)
{

    for(auto it = begin; it != end; ++it) {
        const SpellChessLib::Annotation::AnnotatedSequenceNode & node = *it;
        
        if (node.index() == 0) {
            cursor.insertText("{");
            writeVariation(cursor, std::get<SpellChessLib::Annotation::AnnotatedSequence>(node));
            cursor.insertText("}");
            return;
        }
        const auto & element = std::get<SpellChessLib::Annotation::AnnotatedSequenceElement>(node);

        switch (element.index())
        {
        case 0:
            writeComment(cursor, std::get<std::string>(element));
            break;
        case 1:
            writeAnnotationGlyph(cursor, std::get<SpellChessLib::Annotation::NumericAnnotationGlyph>(element));
            break;
        default:
            // move will be written by different function
            break;
        }

    };
}

MoveListView::MoveListView(std::shared_ptr<MoveListModel> model, QWidget *parent) : QWidget(parent), model(model)
{
    layout = new QVBoxLayout(this);
    browser = new QTextBrowser(this);
    doc = new QTextDocument(this);

    connect(model.get(), &MoveListModel::changed, this, &MoveListView::handleModelChanged);
    layout->addWidget(browser);
    setLayout(layout);
    handleModelChanged();
}

enum MoveListColumn {
    WhiteSideColumn,
    BlackSideColumn
};

struct ChunkInfo {
    const SpellChessLib::SpellChessMove * move; // main sequence move
    MoveListModel::Sequence::const_iterator chunkBegin;
    MoveListModel::Sequence::const_iterator chunkEnd;
};

inline bool isChunkEmpty(const ChunkInfo & info) {
    return info.chunkBegin == info.chunkEnd;
};

ChunkInfo getNextChunk(MoveListModel::Sequence::const_iterator & it, const MoveListModel & model, bool & firstNode) {
    const auto & cend = model.getSequence().cend();
    if (it == cend)
        return {  nullptr, it, it };

    bool stopBeforeNextMove = false;

    // If the game has started by a black player, we should append return posibly empty chunk. 
    if (firstNode && model.getInitialState().getSideToMove() == SpellChessLib::Black) {
        stopBeforeNextMove = true; 
        firstNode = false;
    }

    ChunkInfo result = {nullptr, it, it};

    while ( it != cend) { 
        // is node an element?
        if((*it).index() == 1) {
            const auto & element = std::get<SpellChessLib::Annotation::AnnotatedSequenceElement>(*it); 
            // is an element a move?
            if (element.index() == 2) {
                if (!stopBeforeNextMove) {
                    result.move = &std::get<SpellChessLib::SpellChessMove>(element);
                    stopBeforeNextMove = true;
                    ++it;
                } else {
                    break;
                }
            };
        } else ++it;
    }

    result.chunkEnd = it;
    return result;
};

std::vector<ChunkInfo> splitIntoChunks(const MoveListModel & model) {
    std::vector<ChunkInfo> result;
    auto it = model.getSequence().cbegin();
    const auto cend = model.getSequence().cend();
    bool firstNode = true;
    do {
        auto chunk = getNextChunk(it, model, firstNode);
        if (chunk.chunkBegin != cend)
            result.push_back(chunk);
        else
            break;
    } while(it != cend);
    return result;
};

void MoveListView::handleModelChanged() {
    doc->clear();
    QTextCursor cursor(doc);
    std::vector<ChunkInfo> chunks = splitIntoChunks(*model);

    size_t i = 0;
    for (const ChunkInfo & chunk : chunks) {
        if (i % 2 == 0) 
            writeNumberIndicator(cursor, 1 + (i/2));
        else
            writeTab(cursor);
        
        writeMoveToken(cursor, chunk.move);
        writeExtras(cursor, chunk.chunkBegin, chunk.chunkEnd);

        if (i % 2 == 1)
            cursor.insertBlock();
        ++i;
    };

    browser->setHtml(doc->toHtml());
};

#include <moc_MoveList.cpp>
