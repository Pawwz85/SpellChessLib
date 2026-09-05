#include <SpellChess/GameAnnotations.h>
#include <SpellChess/MoveGen.h>
#include <SpellChess/PGN4Exporter.h>
#include <SpellChess/PGNExporter.h>
#include <fstream>
#include <iostream> // for std::cerr
#include <chrono>
#include <time.h>

const unsigned int& SpellChessLib::Annotation::MoveIndication::getValue() const {
    return value;
}

const unsigned int& SpellChessLib::Annotation::NumericAnnotationGlyph::getGlyphCode() const {
    return glyphCode;
}

SpellChessLib::Annotation::NumericAnnotationGlyph 
SpellChessLib::Annotation::NumericAnnotationGlyph::fromMoveClassification(MoveClassification classification){
    switch (classification) {
        case MoveClassification::NoClassification:
            return 0;
        case MoveClassification::Blunder:
            return 4;
        case MoveClassification::Mistake:
            return 2;
        case MoveClassification::Inaccuraccy:
            return 6;
        case MoveClassification::GoodMove:
            return 1;
        case MoveClassification::BrilliantMove:
            return 3;
    };
};

std::string
SpellChessLib::Annotation::toToken(MoveClassification cathegory) {
    switch (cathegory)
    {
        case MoveClassification::Blunder:
            return "??";
        case MoveClassification::Mistake:
            return "?";
        case MoveClassification::Inaccuraccy:
            return "?!";
        case MoveClassification::GoodMove:
            return "!";
        case MoveClassification::BrilliantMove:
            return "!!";
        default:
            return "";
    }
};


bool SpellChessLib::Annotation::NumericAnnotationGlyph::operator==(const NumericAnnotationGlyph & other) const {
    return glyphCode == other.glyphCode;
};

bool SpellChessLib::Annotation::NumericAnnotationGlyph::operator!=(const NumericAnnotationGlyph & other) const {
    return glyphCode != other.glyphCode;
};

SpellChessLib::Annotation::TagSection::iterator SpellChessLib::Annotation::TagSection::begin() {
    return tagPairs.begin();
}
SpellChessLib::Annotation::TagSection::iterator SpellChessLib::Annotation::TagSection::end() {
    return tagPairs.end();
}

SpellChessLib::Annotation::TagSection::const_iterator SpellChessLib::Annotation::TagSection::cbegin() const {
    return tagPairs.cbegin();
}
SpellChessLib::Annotation::TagSection::const_iterator SpellChessLib::Annotation::TagSection::cend() const {
    return tagPairs.cend();
}

SpellChessLib::Annotation::TagSection::TagSection(const TagSection & other) : tagPairs(other.tagPairs) {}

SpellChessLib::Annotation::TagSection::TagSection(const std::vector<TagPair> & tagPairs) : tagPairs(tagPairs) {}

SpellChessLib::Annotation::TagSection::TagSection(const SevenTagRoster & roster) : tagPairs(
    {{"Event", roster.event},
     {"Site", roster.site},
     {"Date", roster.date},
     {"Round", roster.round},
     {"White", roster.white},
     {"Black", roster.black},
     {"Result", roster.result}}
) {};

SpellChessLib::Annotation::TagSection & 
SpellChessLib::Annotation::TagSection::operator=(const TagSection & other) {
    this->tagPairs = other.tagPairs;
    return *this;
};

SpellChessLib::Annotation::TagSection::iterator SpellChessLib::Annotation::TagSection::find(const std::string& tagName)
{
    auto it = tagPairs.begin();
    while (it != tagPairs.end()) {
        if (it->name == tagName)
            return it;
        ++it;
    }

    return it;
}

SpellChessLib::Annotation::TagSection::const_iterator SpellChessLib::Annotation::TagSection::find(const std::string& tagName) const
{
    auto it = tagPairs.cbegin();
    while (it != tagPairs.cend()) {
        if (it->name == tagName)
            return it;
        ++it;
    }
        
    return it;
}

SpellChessLib::Annotation::AnnotatedSequence::iterator SpellChessLib::Annotation::AnnotatedSequence::begin() {
    return nodes.begin();
}

SpellChessLib::Annotation::AnnotatedSequence::iterator SpellChessLib::Annotation::AnnotatedSequence::end() {
    return nodes.end();
}

SpellChessLib::Annotation::AnnotatedSequence::const_iterator SpellChessLib::Annotation::AnnotatedSequence::begin() const { 
    return cbegin();
}

SpellChessLib::Annotation::AnnotatedSequence::const_iterator SpellChessLib::Annotation::AnnotatedSequence::end() const {
    return cend();
}

SpellChessLib::Annotation::AnnotatedSequence::const_iterator SpellChessLib::Annotation::AnnotatedSequence::cbegin() const {
    return nodes.cbegin();
}

SpellChessLib::Annotation::AnnotatedSequence::const_iterator SpellChessLib::Annotation::AnnotatedSequence::cend() const {
    return nodes.cend();
}

void SpellChessLib::Annotation::TagSection::push_back(const TagPair& tagPair){
    tagPairs.push_back(tagPair);
}

void SpellChessLib::Annotation::TagSection::emplace_back(const std::string& tagName, const std::string& value){
    tagPairs.push_back({ tagName, value });
}

std::string& SpellChessLib::Annotation::TagSection::operator[](const std::string& tagName)
{
    auto it = find(tagName);
    if (it != end())
        return it->value;
    tagPairs.push_back({ tagName, "" });
    return tagPairs.back().value;
}

bool SpellChessLib::Annotation::TagSection::operator==(const TagSection & other) const {
    if (tagPairs.size() != other.tagPairs.size())
        return false;

    for(const auto & pair : tagPairs) {
        auto it = other.find(pair.name);
        if(it == other.end() || it->value != pair.value)
            return false;
    };

    return true;
};

bool SpellChessLib::Annotation::TagSection::operator!=(const TagSection & other) const {
    return !(*this == other);
};

SpellChessLib::Annotation::AnnotatedSequence::AnnotatedSequence() {};

void SpellChessLib::Annotation::AnnotatedSequence::deepcopy(const AnnotatedSequence & other) {
    nodes.clear();
    for(auto & node : other.nodes) {
        switch (node.index())
        {
        case (size_t)AnnotatedSequenceNodeType::Variation:
            this->nodes.push_back(AnnotatedSequence(std::get<Annotation::AnnotatedSequence>(node)));
            break;
        case (size_t)AnnotatedSequenceNodeType::Element:
            this->nodes.push_back(std::get<Annotation::AnnotatedSequenceElement>(node));
        default:
            break;
        }
    };
};

SpellChessLib::Annotation::AnnotatedSequence::AnnotatedSequence(const AnnotatedSequence & other) {
    deepcopy(other);
};

SpellChessLib::Annotation::AnnotatedSequence & SpellChessLib::Annotation::AnnotatedSequence::operator=(const AnnotatedSequence & other) {
    deepcopy(other);
    return *this;
};

bool SpellChessLib::Annotation::AnnotatedSequence::operator==(const AnnotatedSequence & other) const {

    if (nodes.size() != other.nodes.size())
        return false;

    auto it = this->begin();
    auto it2 = other.begin();

    for(; it != this->end(); ++it, ++it2)
        if((*it) != (*it2)) 
            return false;

    return true;
};

bool SpellChessLib::Annotation::AnnotatedSequence::operator!=(const AnnotatedSequence & other) const {
    return !(*this == other);
};

void SpellChessLib::Annotation::AnnotatedSequence::insertVariationAfter(iterator pos, AnnotatedSequence&& variation){
    ++pos;
    this->nodes.insert(pos, variation);
}

void SpellChessLib::Annotation::AnnotatedSequence::insertAnnotationGlyphAfter(iterator pos, const NumericAnnotationGlyph& glyph){
    ++pos;
    this->nodes.insert(pos, glyph);
}

void SpellChessLib::Annotation::AnnotatedSequence::insertCommentAfter(iterator pos, const std::string& comment){
    ++pos;
    this->nodes.insert(pos, comment);
}

void SpellChessLib::Annotation::AnnotatedSequence::appendVariation(AnnotatedSequence&& variation){
    this->nodes.push_back(variation);
}

void SpellChessLib::Annotation::AnnotatedSequence::appendGlyph(const NumericAnnotationGlyph & glyph) {
    this->nodes.push_back(glyph);
}

void SpellChessLib::Annotation::AnnotatedSequence::appendMove(const SpellChessMove & move) {
    this->nodes.push_back(move);
}

void SpellChessLib::Annotation::AnnotatedSequence::appendComment(const std::string& comment){
    this->nodes.push_back(comment);
}

void SpellChessLib::Annotation::validateAnnotatedSequenceMoveLegality(const GameState& root, const Annotation::AnnotatedSequence& seq){
    GameState currentPos = root;
    GameState parentPos = currentPos;
    AnnotatedSequenceElement element;
    for (const auto& node : seq) {
        switch (node.index())
        {
        case (size_t)AnnotatedSequenceNodeType::Variation:
            validateAnnotatedSequenceMoveLegality(parentPos, std::get<Annotation::AnnotatedSequence>(node));
            break;
        case (size_t)AnnotatedSequenceNodeType::Element:
            element = std::get<AnnotatedSequenceElement>(node);

            if (element.index() == (size_t)AnnotatedSequenceElementType::Move) {
                MoveGenerator gen(currentPos);
                SpellChessMove m = std::get<SpellChessMove>(element);
                if (!gen.isMoveLegal(m))
                    throw IllegalMoveException(m, currentPos.toFen());
                parentPos = currentPos;
                currentPos.makeMove(m);
            }
            break;
        default:
            break;
        }
    }
}

SpellChessLib::Annotation::SevenTagRoster SpellChessLib::Annotation::beginSevenTagRooster(const std::string& event, const std::string& site, const std::string& round)
{
    SevenTagRoster result;
    result.event = event;
    result.site = site;
    result.date = makeCurrentDateTag();
    result.round = round;
    result.white = result.black = "?";
    result.result = "*";
    return result;
}

SpellChessLib::Annotation::SevenTagRoster SpellChessLib::Annotation::makePlaceHolderRoster() {
    SevenTagRoster result;
    result.event = "?";
    result.site = "?";
    result.date = "??.??.????";
    result.round = "?";
    result.white = result.black = "?";
    result.result = "*";
    return result;
};

void SpellChessLib::Annotation::addPlayerTagsToRoster(SevenTagRoster& roster, const std::string& whitePlayer, const std::string& blackPlayer)
{
    roster.black = blackPlayer;
    roster.white = whitePlayer;
}

std::string SpellChessLib::Annotation::makeCurrentDateTag() {
    auto now = std::chrono::system_clock::now();
    const std::time_t currentTime = std::chrono::system_clock::to_time_t(now);

    // Microsoft is a silly goose that marks localtime as "unsafe" and deprecates it while making the "localtime_s" not compliant with posix systems due to reverse order of arguments.
#pragma warning(suppress : 4996)
    std::tm * calendar = std::localtime(&currentTime);
    
    if (!calendar)
        return "????.??.??";

    return std::to_string(calendar->tm_year +  1900) + "." + std::to_string(calendar->tm_mon) + "." + std::to_string(calendar->tm_mday);
}

std::string SpellChessLib::Annotation::makeResultTag(const GameTermination& reason)
{
    switch (reason) {
    case WhiteWon:
        return "1-0";
    case BlackWon:
        return "0-1";
    case Draw:
        return "1/2-1/2";
    default:
        return "*";
    }
}

std::string SpellChessLib::Annotation::makeIllegalMoveComment(const PlayerColor& color, const SpellChessMove& move){
    std::string playerSide[3] = { "-", "White", "Black" };
    std::string token;
    try {
        token = spellChessMoveToString(move);
    }
    catch (InvalidMoveException e) {
        token = "?";
    }
   
    return playerSide[color] + " tried to play an invalid move: " + token + "";
}

void SpellChessLib::Annotation::exportAnnotatedGame(const std::string& filePath, const GameAnnotations& annotations){
    
    std::string extension;
    std::ofstream file;
    if (annotations.root.isSpellChessGame()) {
        extension = ".pgn4";
        file.open(filePath + extension);
        if (file.is_open()) {
            PGN4Exporter exporter(&file);
            exporter.exportPGN4(annotations.root, annotations);
        }
        else
            std::cerr << "Couldn't write to a file: " << filePath + extension << "\n";
        file.close();
    }
    else {
        extension = ".pgn";
        file.open(filePath + extension);
        if (file.is_open()) {
            PGNExporter exporter(&file);
            exporter.exportPGN(annotations);
        }
        else
            std::cerr << "Couldn't write to a file: " << filePath + extension << "\n";
        file.close();
    }

}
