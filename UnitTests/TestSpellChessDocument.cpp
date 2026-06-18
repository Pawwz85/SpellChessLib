#include "pch.h"

#include <SpellChess/SpellChessDocument.h>

using namespace SpellChessLib;

TEST(SpellChessDocument, createEmptyNoParams) {
    auto doc = createEmptySpellChessDocument();

    ASSERT_EQ(doc->getRoot().lock(), doc->getTail().lock());
    ASSERT_TRUE(doc->getTail().lock()->isLeaf());
    GameState rootState = doc->getRoot().lock()->getGameState();
    ASSERT_EQ(rootState.toFen(), SPELL_START_POS);
};

TEST(SpellChessDocument, makeMove) {
    auto doc = createEmptySpellChessDocument();
    SpellChessDocument::Handle rootHandle = doc->getRoot();
    SpellChessDocument::Handle nextPos = doc->makeMoveAndAdvance(rootHandle, parseMove("e2e4"));

    ASSERT_EQ(doc->getTail().lock(), nextPos.lock());
    ASSERT_EQ(rootHandle.lock()->getChildByMove(parseMove("e2e4")).lock(), nextPos.lock());
    ASSERT_TRUE(doc->getTail().lock()->isLeaf());
    ASSERT_FALSE(doc->getRoot().lock()->isLeaf());
    
    GameState tailState = nextPos.lock()->getGameState();
    ASSERT_EQ(tailState.toFen(), "rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1 - - F50/J20/f50/j20");
};

TEST(SpellChessDocument, makeMoveIsIdempotent) {
    auto doc = createEmptySpellChessDocument();
    SpellChessDocument::Handle rootHandle = doc->getRoot();
    SpellChessDocument::Handle nextPos = doc->makeMoveAndAdvance(rootHandle, parseMove("e2e4"));
    (void)doc->clearDirtyFlag(); // clear dirty flag
    SpellChessDocument::Handle nextPos2 = doc->makeMoveAndAdvance(rootHandle, parseMove("e2e4"));

    ASSERT_EQ(nextPos.lock(), nextPos2.lock());
    ASSERT_FALSE(doc->isDirty());
};

TEST(SpellChessDocument, makeMoveSetsDirtyFlag) {
    auto doc = createEmptySpellChessDocument();
    SpellChessDocument::Handle rootHandle = doc->getRoot();
    SpellChessDocument::Handle nextPos = doc->makeMoveAndAdvance(rootHandle, parseMove("e2e4"));
    ASSERT_TRUE(doc->isDirty());
};

TEST(SpellChessDocument, getChildByMoveReturnsEmptyIfNoCHildPresent) {
    auto doc = createEmptySpellChessDocument();
    SpellChessDocument::Handle rootHandle = doc->getRoot();
    auto h = rootHandle.lock()->getChildByMove(parseMove("e2e4"));

    ASSERT_FALSE(h.lock());
};

TEST(SpellChessDocument, makeMoveThrowsOnIllegalMove) {
    auto doc = createEmptySpellChessDocument();
    ASSERT_ANY_THROW(doc->makeMoveAndAdvance(doc->getRoot(), parseMove("e1e2")));
};

TEST(SpellChessDocument, badHandle1) {
    auto doc = createEmptySpellChessDocument();
    ASSERT_THROW((void)doc->getComment(SpellChessDocument::Handle()), SpellChessDocument::BadHandle);
};

TEST(SpellChessDocument, badHandle2) {
    auto doc = createEmptySpellChessDocument();
    auto otherDoc = createEmptySpellChessDocument();

    ASSERT_THROW((void)doc->setComment(otherDoc->getRoot(), "Test"), SpellChessDocument::BadHandle);
};

TEST(SpellChessDocument, setComment) {
    auto doc = createEmptySpellChessDocument();
    doc->setComment(doc->getRoot(), "Starting position");
    ASSERT_EQ(doc->getComment(doc->getRoot()), "Starting position");
    ASSERT_TRUE(doc->isDirty());
}

TEST(SpellChessDocument, clearDirtyFlag) {
    auto doc = createEmptySpellChessDocument();
    doc->setComment(doc->getRoot(), "Starting position");
    ASSERT_TRUE(doc->clearDirtyFlag());
    ASSERT_FALSE(doc->clearDirtyFlag());
}

TEST(SpellChessDocument, setTag) {
    auto doc = createEmptySpellChessDocument();

    doc->setTag("Date", "17.06.2026");
    ASSERT_EQ(doc->getTag("Date").value(), "17.06.2026");
    ASSERT_TRUE(doc->isDirty());
};

TEST(SpellChessDocument, setTag2) {
    auto doc = createEmptySpellChessDocument();

    doc->setTag("Date", "17.06.2026");
    ASSERT_EQ(doc->getTag("Date").value(), "17.06.2026");
    ASSERT_TRUE(doc->isDirty());
};

TEST(SpellChessDocument, sevenTagRosterValuesExistOutOfTheBox) {
    auto doc = createEmptySpellChessDocument();

    ASSERT_TRUE(doc->getTag("Event").has_value());
    ASSERT_TRUE(doc->getTag("Site").has_value());
    ASSERT_TRUE(doc->getTag("Date").has_value());
    ASSERT_TRUE(doc->getTag("Round").has_value());
    ASSERT_TRUE(doc->getTag("White").has_value());
    ASSERT_TRUE(doc->getTag("Black").has_value());
    ASSERT_TRUE(doc->getTag("Result").has_value());
};

TEST(SpellChessDocument, onlySevenTagRosterTagsExistAtTheBegining) {
    auto doc = createEmptySpellChessDocument();
    ASSERT_FALSE(doc->getTag("SomeOptionalValue").has_value());
};

TEST(SpellChessDocument, enumeratePositions) {
    auto doc = createEmptySpellChessDocument();

    auto root = doc->getRoot();
    auto pos1 = doc->makeMoveAndAdvance(root, parseMove("e2e4"));
    auto pos2 = doc->makeMoveAndAdvance(root, parseMove("d2d4"));
    auto pos3 = doc->makeMoveAndAdvance(root, parseMove("c2c4"));

    auto moves = doc->getChildren(root);
    ASSERT_EQ(moves.size(), 3);
    ASSERT_EQ(moves[0].lock(), pos1.lock());
    ASSERT_EQ(moves[1].lock(), pos2.lock());
    ASSERT_EQ(moves[2].lock(), pos3.lock());
};

Annotation::GameAnnotations createModelAnnotations() {
    Annotation::AnnotatedSequence seq;
    Annotation::AnnotatedSequence variation;

    // some base
    seq.pushBack(parseMove("e2e4"));
    seq.pushBack(parseMove("e7e5"));
    seq.pushBack(parseMove("f1b5"));
    seq.pushBack("Attacking the king");

    variation.pushBack(parseMove("freeze@b5&c7c6"));
    variation.pushBack("blunder");
    variation.pushBack(parseMove("freeze@g7&d1h5"));

    // main variation
    seq.pushBack(parseMove("freeze@b5&f8b4"));

    // put variation
    seq.pushBack(std::move(variation));


    auto roster = Annotation::beginSevenTagRooster("ExampleEvent", "localhost", "1");
    roster.date = "17.06.2026";
    roster.white = "White Player";
    roster.black = "Black Player";
    roster.result = "*";

    FenParser parser;
    GameState s = parser.fromSpellChessFen(SPELL_START_POS);

    return { {roster}, 
            seq,
            Annotation::Unknown,
            s    
        };
};

TEST(SpellChessDocument, fromAnnotations_topology) {
    auto annotations = createModelAnnotations();
    auto doc = std::make_shared<SpellChessDocument>(annotations);

    auto currentNode = doc->getRoot();

    ASSERT_EQ(currentNode.lock()->getGameState().toFen(), SPELL_START_POS);
    currentNode = doc->getNextNode(currentNode);
    ASSERT_EQ(currentNode.lock()->getGameState().toFen(), "rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1 - - F50/J20/f50/j20");
    currentNode = doc->getNextNode(currentNode);
    ASSERT_EQ(currentNode.lock()->getGameState().toFen(), "rnbqkbnr/pppp1ppp/8/4p3/4P3/8/PPPP1PPP/RNBQKBNR w KQkq e6 0 2 - - F50/J20/f50/j20");
    currentNode = doc->getNextNode(currentNode);
    ASSERT_EQ(currentNode.lock()->getGameState().toFen(), "rnbqkbnr/pppp1ppp/8/1B2p3/4P3/8/PPPP1PPP/RNBQK1NR b KQkq - 1 2 - - F50/J20/f50/j20");

    ASSERT_EQ(doc->getComment(currentNode), "Attacking the king");
    auto children = doc->getChildren(currentNode);

    ASSERT_EQ(children.size(), 2);
    
    auto mainVariation = children[0];

    ASSERT_EQ(mainVariation.lock()->getGameState().toFen(), "rnbqk1nr/pppp1ppp/8/1B2p3/1b2P3/8/PPPP1PPP/RNBQK1NR w KQkq - 2 3 b5 - F50/J20/f42/j20");

    auto variation = children[1];

    ASSERT_EQ(variation.lock()->getGameState().toFen(), "rnbqkbnr/pp1p1ppp/2p5/1B2p3/4P3/8/PPPP1PPP/RNBQK1NR w KQkq - 0 3 b5 - F50/J20/f42/j20");
    ASSERT_EQ(variation.lock()->getComment(), "blunder");
};

TEST(SpellChessDocument, formAnnotations_tags) {
    auto annotations = createModelAnnotations();
    auto doc = std::make_shared<SpellChessDocument>(annotations);

    ASSERT_EQ(doc->getTag("Event").value(), "ExampleEvent");
    ASSERT_EQ(doc->getTag("Site").value(),  "localhost");
    ASSERT_EQ(doc->getTag("Round").value(), "1");
    ASSERT_EQ(doc->getTag("Date").value(), "17.06.2026");
    ASSERT_EQ(doc->getTag("White").value(), "White Player");
    ASSERT_EQ(doc->getTag("Black").value(), "Black Player");
    ASSERT_EQ(doc->getTag("Result").value(), "*");
};

TEST(SpellChessDocument, exportAnnotatedSequence) {
    auto annotations = createModelAnnotations();
    auto doc = std::make_shared<SpellChessDocument>(annotations);

    auto exportedAnnotations = doc->toAnnotations();

    ASSERT_EQ(annotations.tagSection, exportedAnnotations.tagSection);
    ASSERT_EQ(annotations.annotatedSequence, exportedAnnotations.annotatedSequence);
    ASSERT_EQ(annotations.terminationReason, exportedAnnotations.terminationReason);
    ASSERT_EQ(annotations.root.toFen(), exportedAnnotations.root.toFen());
};

TEST(SpellChessDocument, settingGameResultSetsResultTag) {
    auto doc = createEmptySpellChessDocument();

    doc->setGameResult(Annotation::GameTermination::Draw);
    ASSERT_EQ(doc->getTag("Result").value(), "1/2-1/2");

    doc->setGameResult(Annotation::GameTermination::WhiteWon);
    ASSERT_EQ(doc->getTag("Result").value(), "1-0");

    doc->setGameResult(Annotation::GameTermination::BlackWon);
    ASSERT_EQ(doc->getTag("Result").value(), "0-1");

    doc->setGameResult(Annotation::GameTermination::Unknown);
    ASSERT_EQ(doc->getTag("Result").value(), "*");
};

TEST(SpellChessDocument, settingResultTagThrowsException) {
    auto doc = createEmptySpellChessDocument();

    // Setting 'Result' tag manually is prohibited. Use doc->setGameResult instead
    ASSERT_ANY_THROW(doc->setTag("Result", "1-0"));
};

