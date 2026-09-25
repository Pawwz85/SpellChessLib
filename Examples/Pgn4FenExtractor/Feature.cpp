#include "Feature.h"

#include <SpellChess/ZobristHash.h>

#include <algorithm>
#include <cctype>
#include <functional>
#include <stdexcept>
#include <sstream>

namespace {
std::string trim(const std::string & value) {
    auto first = std::find_if_not(value.begin(), value.end(), [](unsigned char c) { return std::isspace(c) != 0; });
    auto last = std::find_if_not(value.rbegin(), value.rend(), [](unsigned char c) { return std::isspace(c) != 0; }).base();
    return first < last ? std::string(first, last) : std::string();
}
}

namespace Extractor {

namespace {
class StringFeature final : public Feature {
    std::string value;
public:
    explicit StringFeature(std::string value) : value(std::move(value)) {}
    std::string toString() const override { return value; }
};

class LambdaFeatureDefinition final : public FeatureDefinition {
    std::function<std::unique_ptr<Feature>(const SpellChessLib::SpellChessDocument &, SpellChessLib::SpellChessDocument::Handle)> extractor;
public:
    LambdaFeatureDefinition(const std::string & name, const std::string & description,
        decltype(extractor) extractor)
        : FeatureDefinition(name, description), extractor(std::move(extractor)) {}
    std::unique_ptr<Feature> extract(const SpellChessLib::SpellChessDocument & doc,
        SpellChessLib::SpellChessDocument::Handle handle) const override { return extractor(doc, handle); }
};

std::vector<std::shared_ptr<FeatureDefinition>> & registry() {
    static std::vector<std::shared_ptr<FeatureDefinition>> definitions;
    return definitions;
}

void registerBuiltInFeatures() {
    static bool initialized = false;
    if (initialized) return;
    initialized = true;
    registerFeature("fen", [](const SpellChessLib::SpellChessDocument & doc, auto handle) {
        return std::make_unique<StringFeature>(doc.getGameState(handle).toFen());
    });
    registerFeature("result", [](const SpellChessLib::SpellChessDocument & doc, auto) {
        return std::make_unique<StringFeature>(doc.getTag("Result").value_or("*"));
    });
    registerFeature("hash", [](const SpellChessLib::SpellChessDocument & doc, auto handle) {
        return std::make_unique<StringFeature>(std::to_string(SpellChessLib::calculateHash(doc.getGameState(handle))));
    });
}
}

void registerFeature(std::unique_ptr<FeatureDefinition> feature) {
    registry().push_back(std::shared_ptr<FeatureDefinition>(std::move(feature)));
}

void registerFeature(const std::string & name,
    std::function<std::unique_ptr<Feature>(const SpellChessLib::SpellChessDocument &, SpellChessLib::SpellChessDocument::Handle)> extractor) {
    registerFeature(std::make_unique<LambdaFeatureDefinition>(name, "", std::move(extractor)));
}

void registerFeature(const std::string & name, std::function<std::unique_ptr<Feature>(const SpellChessLib::GameState &)> extractor) {
    registerFeature(name, [extractor = std::move(extractor)](const auto & doc, auto handle) {
        return extractor(doc.getGameState(handle));
    });
}

void registerFeature(const std::string & name, std::function<std::unique_ptr<Feature>(SpellChessLib::SpellChessDocument::Handle)> extractor) {
    registerFeature(name, [extractor = std::move(extractor)](const auto &, auto handle) {
        return extractor(handle);
    });
}

const std::vector<std::shared_ptr<FeatureDefinition>> & featureDefinitions() {
    registerBuiltInFeatures();
    return registry();
}

FeatureNotFound::FeatureNotFound(const std::string & featureName)
    : message("feature not found: " + featureName) {}

const char * FeatureNotFound::what() const noexcept { return message.c_str(); }

const FeatureDefinition & definitionOf(const std::string & name) {
    const auto & definitions = featureDefinitions();
    const auto found = std::find_if(definitions.begin(), definitions.end(), [&](const auto & definition) {
        return definition->name == name;
    });
    if (found == definitions.end()) throw FeatureNotFound(name);
    return **found;
}

std::unique_ptr<Feature> extract(const std::string & featureName,
    const SpellChessLib::SpellChessDocument & doc, SpellChessLib::SpellChessDocument::Handle handle) {
    return definitionOf(featureName).extract(doc, handle);
}

std::vector<std::string> parseFeaturePattern(const std::string & pattern) {
    std::vector<std::string> result;
    size_t begin = 0;
    while (begin <= pattern.size()) {
        const size_t end = pattern.find(',', begin);
        const std::string name = trim(pattern.substr(begin, end == std::string::npos ? std::string::npos : end - begin));
        if (name.empty()) throw std::invalid_argument("empty feature name");
        (void)definitionOf(name);
        result.push_back(name);
        if (end == std::string::npos) break;
        begin = end + 1;
    }
    return result;
}

std::string escapeCsvField(const std::string & value) {
    if (value.find_first_of(",\"\r\n") == std::string::npos) return value;
    std::string escaped = "\"";
    for (char c : value) escaped += c == '"' ? "\"\"" : std::string(1, c);
    return escaped + "\"";
}

}