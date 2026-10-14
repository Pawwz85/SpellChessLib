#pragma once

#include <SpellChess/SpellChessDocument.h>
#include <SpellChess/Board.h>

#include <string>
#include <exception>
#include <memory>
#include <functional>
#include <vector>

namespace Extractor {

    class Feature {
    public:
        virtual ~Feature() = default;
        virtual std::string toString() const = 0;
    };

    class FeatureDefinition {
    public:
        const std::string name;
        const std::string description;
        FeatureDefinition(const std::string & featureName, const std::string & featureDescription)
            : name(featureName), description(featureDescription) {}
        virtual ~FeatureDefinition() = default;
        virtual std::unique_ptr<Feature> extract(const SpellChessLib::SpellChessDocument & doc,
            SpellChessLib::SpellChessDocument::Handle handle) const = 0;
    };

    void registerFeature(std::unique_ptr<FeatureDefinition> feature);
    void registerFeature(const std::string & name,
        std::function<std::unique_ptr<Feature>(const SpellChessLib::SpellChessDocument &, SpellChessLib::SpellChessDocument::Handle)> extractor);
    void registerFeature(const std::string & name,
        std::function<std::unique_ptr<Feature>(const SpellChessLib::GameState &)> extractor);
    void registerFeature(const std::string & name,
        std::function<std::unique_ptr<Feature>(SpellChessLib::SpellChessDocument::Handle)> extractor);

    const std::vector<std::shared_ptr<FeatureDefinition>> & featureDefinitions();

    class FeatureNotFound : public std::exception {
        std::string message;
    public:
        explicit FeatureNotFound(const std::string & featureName);
        const char * what() const noexcept override;
    };

    const FeatureDefinition & definitionOf(const std::string & name);
    std::unique_ptr<Feature> extract(const std::string & featureName,
        const SpellChessLib::SpellChessDocument & doc,
        SpellChessLib::SpellChessDocument::Handle handle);

    std::vector<std::string> parseFeaturePattern(const std::string & pattern);
    std::string escapeCsvField(const std::string & value);
}