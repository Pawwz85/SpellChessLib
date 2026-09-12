#pragma once

#include "SpellChessDocument.h"
#include "GameAnnotations.h"
#include "SpellChessUciLoaderPort.h"

#include "UCIHelpers.h"

#include <optional>
#include <functional>
#include <memory>
#include <map>

namespace SpellChessLib {

    // What does annotator has to say about given position?
    struct PositionInsights {
        std::string         suggestedComment;  
        ScorePOV            positionScore;
        SpellChessMove      bestMove;
        std::vector<SpellChessMove> bestContinuation; // PV engine suggested
        std::vector<Annotation::NumericAnnotationGlyph> nags; // Any additional nags that annotator wants to add
    };

    using PositionAnalizer = std::function<PositionInsights(const GameState &, const std::vector<SpellChessMove> &)>;
    using MoveClassifier = std::function<Annotation::MoveClassification(const GameState &, const PositionInsights &, const PositionInsights &)>;

    class DocumentAnnotator {
        PositionAnalizer analizer;
        MoveClassifier moveClassifier;

        std::function<void()> onPositionAnalized = [](){};
        std::function<void()> onFinish = [](){};

        std::map<const SpellChessDocument::Node*, PositionInsights> analizerCache;

        std::optional<PositionInsights> lookupCache(SpellChessDocument::Handle pos);
        void updateCache(SpellChessDocument::Handle pos, const PositionInsights & insights);
        void analizeInternal(const SpellChessDocument & document, SpellChessDocument::Handle pos, const GameState & root, std::vector<SpellChessMove> & movesSinceRoot);
        void analize(const SpellChessDocument & document);
        void annotateInternal(SpellChessDocument & document, SpellChessDocument::Handle pos);
        void annotate(SpellChessDocument & document);
        void tryWritingAlternative(SpellChessDocument & document, SpellChessDocument::Handle pos);
        SpellChessDocument::Handle writeVariation(SpellChessDocument & document, SpellChessDocument::Handle pos, const std::vector<SpellChessMove> & moves);
        
        public:
        DocumentAnnotator(PositionAnalizer analizer, MoveClassifier moveClassifier);
        void setPositionAnalizedCallback(std::function<void()> callback);
        void setFinishCallback(std::function<void()> callback);
        void operator()(std::shared_ptr<SpellChessDocument> document);
    };

    extern PositionAnalizer makeFixedDetphAnalizer(std::shared_ptr<UCILoader::EngineInstance<SpellChessMove>> engine, uint32_t depth);
    extern PositionAnalizer makeFixedTimeAnalizer(std::shared_ptr<UCILoader::EngineInstance<SpellChessMove>> engine, std::chrono::milliseconds timePerPos);

    extern Annotation::MoveClassification SimplePositionClassifier (const GameState & state, const PositionInsights & before, const PositionInsights & after);
};