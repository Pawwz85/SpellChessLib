#include "pch.h"

#include <Feature.h>
#include <PGN4Processor.h>
#include <PGN4Supplier.h>

#include <SpellChess/SpellChessDocument.h>

#include <filesystem>
#include <fstream>
#include <algorithm>
#include <memory>
#include <sstream>

namespace {
class TestSupplier final : public Extractor::PGN4Supplier {
    std::vector<std::string> values;
    mutable size_t index = 0;
public:
    explicit TestSupplier(std::vector<std::string> values) : values(std::move(values)) {}
    bool hasNext() const override { return index < values.size(); }
    std::string getNext() const override { return values.at(index++); }
};
}

TEST(Pgn4ExtractorFeatures, parsesAndPreservesOrder) {
    const auto features = Extractor::parseFeaturePattern(" hash, fen ,result ");
    ASSERT_EQ(features, (std::vector<std::string>{"hash", "fen", "result"}));
}

TEST(Pgn4ExtractorFeatures, rejectsEmptyAndUnknownNames) {
    EXPECT_THROW(Extractor::parseFeaturePattern("fen,,hash"), std::invalid_argument);
    EXPECT_THROW(Extractor::parseFeaturePattern("fen,unknown"), Extractor::FeatureNotFound);
}

TEST(Pgn4ExtractorFeatures, escapesCsvFields) {
    EXPECT_EQ(Extractor::escapeCsvField("plain"), "plain");
    EXPECT_EQ(Extractor::escapeCsvField("a,b\"c\nd"), "\"a,b\"\"c\nd\"");
}

TEST(Pgn4ExtractorFeatures, extractsFenResultAndHash) {
    SpellChessLib::FenParser parser;
    SpellChessLib::SpellChessDocument document(
        parser.fromSpellChessFen(SpellChessLib::SPELL_START_POS),
        SpellChessLib::Annotation::makePlaceHolderRoster(),
        SpellChessLib::Annotation::Draw);
    const auto root = document.getRoot();
    EXPECT_FALSE(Extractor::extract("fen", document, root)->toString().empty());
    EXPECT_EQ(Extractor::extract("result", document, root)->toString(), "*");
    EXPECT_FALSE(Extractor::extract("hash", document, root)->toString().empty());
}

TEST(Pgn4ExtractorFiles, discoversSortedAndRecursiveFiles) {
    const auto root = std::filesystem::temp_directory_path() / "spellchess-pgn4-extractor-test";
    std::filesystem::remove_all(root);
    std::filesystem::create_directories(root / "nested");
    std::ofstream(root / "z.pgn4") << "";
    std::ofstream(root / "a.pgn") << "";
    std::ofstream(root / "nested" / "m.pgn4") << "";

    const auto direct = Extractor::FilesystemPGN4Supplier::discover(root, false);
    ASSERT_EQ(direct.size(), 2u);
    EXPECT_EQ(direct[0].filename().string(), "a.pgn");
    EXPECT_EQ(direct[1].filename().string(), "z.pgn4");
    EXPECT_EQ(Extractor::FilesystemPGN4Supplier::discover(root, true).size(), 3u);

    std::filesystem::remove_all(root);
}
