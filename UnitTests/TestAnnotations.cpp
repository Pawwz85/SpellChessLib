#include "pch.h"

#include <SpellChess/GameAnnotations.h>
#include <SpellChess/PGNExporter.h>

using namespace SpellChessLib;

TEST(AnnotatedSequence, constructingSequence) {
	FenParser parser;
	GameState root = parser.fromStandardChessFen(START_POS);

	Annotation::GameAnnotations annotations = {
		{},
		{},
		Annotation::Unknown,
		root
	};

	annotations.annotatedSequence.appendMove("e2e4");
	annotations.annotatedSequence.appendMove("e7e5");
	annotations.annotatedSequence.appendMove("g1f3");
	
	ASSERT_EQ(toInlinePgnMoveText(annotations), "1.e4 e5 2.Nf3 *");
}

TEST(AnnotatedSequence, foolMate) {
	FenParser parser;
	GameState root = parser.fromStandardChessFen(START_POS);

	Annotation::GameAnnotations annotations = {
		{},
		{},
		Annotation::Unknown,
		root
	};

	annotations.annotatedSequence.appendMove("f2f4");
	annotations.annotatedSequence.appendMove("e7e5");
	annotations.annotatedSequence.appendMove("g2g4");
	annotations.annotatedSequence.appendMove("d8h4");
	annotations.terminationReason = Annotation::BlackWon;
	ASSERT_EQ(toInlinePgnMoveText(annotations), "1.f4 e5 2.g4 Qh4# 0-1");
}

TEST(AnnotatedSequence, check) {
	FenParser parser;
	GameState root = parser.fromStandardChessFen(START_POS);

	Annotation::GameAnnotations annotations = {
		{},
		{},
		Annotation::Unknown,
		root
	};

	annotations.annotatedSequence.appendMove("f2f4");
	annotations.annotatedSequence.appendMove("e7e5");
	annotations.annotatedSequence.appendMove("c2c3");
	annotations.annotatedSequence.appendMove("d8h4");

	ASSERT_EQ(toInlinePgnMoveText(annotations), "1.f4 e5 2.c3 Qh4+ *");
}


TEST(AnnotatedSequence, illegalMove) {
	FenParser parser;
	GameState root = parser.fromStandardChessFen(START_POS);

	Annotation::GameAnnotations annotations = {
		{},
		{},
		Annotation::Unknown,
		root
	};
	annotations.annotatedSequence.appendMove("e1e3");
	ASSERT_THROW(Annotation::validateAnnotatedSequenceMoveLegality(annotations.root, annotations.annotatedSequence), IllegalMoveException);
}

TEST(AnnotatedSequence, pushingBackComment) {
	FenParser parser;
	GameState root = parser.fromStandardChessFen(START_POS);

	Annotation::GameAnnotations annotations = {
		{},
		{},
		Annotation::Unknown,
		root
	};

	annotations.annotatedSequence.appendMove("e2e4");
	annotations.annotatedSequence.appendMove("e7e5");
	annotations.annotatedSequence.appendComment("Book Move");
	ASSERT_EQ(toInlinePgnMoveText(annotations), "1.e4 e5 {Book Move} *");
}

TEST(AnnotatedSequence, pushingBackNag) {
	FenParser parser;
	GameState root = parser.fromStandardChessFen(START_POS);

	Annotation::GameAnnotations annotations = {
		{},
		{},
		Annotation::Unknown,
		root
	};

	annotations.annotatedSequence.appendMove("e2e4");
	annotations.annotatedSequence.appendMove("e7e5");
	annotations.annotatedSequence.appendGlyph(11);
	ASSERT_EQ(toInlinePgnMoveText(annotations), "1.e4 e5 $11 *");
}

TEST(AnnotatedSequence, addingRecursiveVariation) {
	FenParser parser;
	GameState root = parser.fromStandardChessFen(START_POS);

	Annotation::GameAnnotations annotations = {
		{},
		{},
		Annotation::Unknown,
		root
	};

	annotations.annotatedSequence.appendMove("e2e4");
	annotations.annotatedSequence.appendMove("e7e5");
	annotations.annotatedSequence.appendMove("g1f3");
	annotations.annotatedSequence.appendMove("b8c6");


	Annotation::AnnotatedSequence alternative;
	alternative.appendMove("f1c4");
	alternative.appendMove("f8c5");

	auto i = annotations.annotatedSequence.begin();

	for ((void)i; i != annotations.annotatedSequence.end(); ++i) {
		auto& element = std::get<Annotation::AnnotatedSequenceElement>(*i);
		if (spellChessMoveToString(std::get<SpellChessMove>(element)) == "g1f3") {
			annotations.annotatedSequence.insertVariationAfter(i, std::move(alternative));
			break;
		}
	}
	ASSERT_EQ(toInlinePgnMoveText(annotations), "1.e4 e5 2.Nf3 ( 2.Bc4 Bc5 ) Nc6 *");
}

TEST(AnnotatedSequence, addingRecursiveVariationWithIllegalLeadingMove) {
	FenParser parser;
	GameState root = parser.fromStandardChessFen(START_POS);

	Annotation::GameAnnotations annotations = {
		{},
		{},
		Annotation::Unknown,
		root
	};

	annotations.annotatedSequence.appendMove("e2e4");
	annotations.annotatedSequence.appendMove("e7e5");
	annotations.annotatedSequence.appendMove("g1f3");
	annotations.annotatedSequence.appendMove("b8c6");


	Annotation::AnnotatedSequence alternative;
	alternative.appendMove("f1f2");
	alternative.appendMove("f8c5");

	auto i = annotations.annotatedSequence.begin();

	for ((void)i; i != annotations.annotatedSequence.end(); ++i) {
		auto& element = std::get<Annotation::AnnotatedSequenceElement>(*i);
		if (spellChessMoveToString(std::get<SpellChessMove>(element)) == "e7e5") {
			annotations.annotatedSequence.insertVariationAfter(i, std::move(alternative));
			break;
		}
	}

	ASSERT_THROW(Annotation::validateAnnotatedSequenceMoveLegality(annotations.root, annotations.annotatedSequence), IllegalMoveException);
}

TEST(AnnotatedSequence, blackMovesFirst) {
	FenParser parser;
	GameState root = parser.fromStandardChessFen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR b KQkq - 0 1");

	Annotation::GameAnnotations annotations = {
		{},
		{},
		Annotation::Unknown,
		root
	};

	annotations.annotatedSequence.appendMove("e7e5");
	annotations.annotatedSequence.appendMove("e2e4");
	annotations.annotatedSequence.appendMove("b8c6");

	ASSERT_EQ(toInlinePgnMoveText(annotations), "1...e5 2.e4 Nc6 *");
}
