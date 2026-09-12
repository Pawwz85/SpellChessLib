#include <SpellChess/DocumentAnnotator.h>
#include <SpellChess/MoveGen.h>
#include <SpellChess/Arbiter.h>
#include <SpellChess/ZobristHash.h>
#include <stdexcept>
#include <cmath>

std::optional<SpellChessLib::PositionInsights>
SpellChessLib::DocumentAnnotator::lookupCache(SpellChessDocument::Handle pos) {
    auto locked = pos.lock();
    if (!locked)
        return std::nullopt;
    auto it = analizerCache.find(locked.get());
    if (it == analizerCache.end())
        return std::nullopt;
    return it->second;
}

void
SpellChessLib::DocumentAnnotator::updateCache(SpellChessDocument::Handle pos, const PositionInsights & insights) {
    if (auto node = pos.lock()) 
        analizerCache.insert_or_assign(node.get(), insights);
}

void
SpellChessLib::DocumentAnnotator::analizeInternal(const SpellChessDocument & document, SpellChessDocument::Handle pos, const GameState & root, std::vector<SpellChessMove> & movesSinceRoot){
    PositionInsights insights = analizer(root, movesSinceRoot);
    onPositionAnalized();
    updateCache(pos, insights);

    auto arcs = document.getArcs(pos);
    for (auto & arc : arcs) {
        movesSinceRoot.push_back(arc.move);
        analizeInternal(document, arc.node, root, movesSinceRoot);
        movesSinceRoot.pop_back();
    }
}

void
SpellChessLib::DocumentAnnotator::analize(const SpellChessDocument & document) {
    SpellChessDocument::Handle root = document.getRoot();
    GameState rootPosition = document.getGameState(root);
    std::vector<SpellChessMove> moveStack;
    analizerCache.clear();
    try {
        analizeInternal(document, root, rootPosition, moveStack);
    } catch (const std::exception & e) {
        throw std::runtime_error(std::string("Error during document analisis: ") + e.what());
    }
}

void 
SpellChessLib::DocumentAnnotator::annotateInternal(SpellChessDocument & document, SpellChessDocument::Handle pos) {
    auto arcs = document.getArcs(pos);
    
    std::optional<PositionInsights> insights = lookupCache(pos);
    std::optional<PositionInsights> childInsights = std::nullopt;
    auto gameState = document.getGameState(pos);

    if (!insights.has_value())
        goto POS_ANNOTATED;

    if (!insights->suggestedComment.empty())
        document.setComment(pos, insights->suggestedComment);

    for (auto & arc: arcs) {
        childInsights = lookupCache(arc.node);
        if (!childInsights.has_value())
            continue;
        auto classification = moveClassifier(gameState, insights.value(), childInsights.value());

        if (classification == Annotation::MoveClassification::Blunder  ||
            classification == Annotation::MoveClassification::Mistake  ||
            classification == Annotation::MoveClassification::Inaccuraccy )
            tryWritingAlternative(document, pos);

        document.setMoveClassification(pos, arc.move, classification);
    }

    POS_ANNOTATED:
    for (auto & arc : arcs)
        annotateInternal(document, arc.node);
}

void
SpellChessLib::DocumentAnnotator::annotate(SpellChessDocument & document) {
    annotateInternal(document, document.getRoot());
    document.markAsDirty(); // mark document as dirty after annotations are applied
}

void
SpellChessLib::DocumentAnnotator::tryWritingAlternative(SpellChessDocument & document, SpellChessDocument::Handle pos){
    auto insights = lookupCache(pos);

    if (insights.has_value()) {
        auto variationEnd = writeVariation(document, pos, insights.value().bestContinuation);
    };

}


SpellChessLib::SpellChessDocument::Handle 
SpellChessLib::DocumentAnnotator::writeVariation(SpellChessDocument & document, SpellChessDocument::Handle pos, const std::vector<SpellChessMove> & moves){
    auto currentPos = pos;
    for (auto & move : moves) {
        currentPos = document.makeMove(currentPos, move);
    }
    return currentPos;
}


SpellChessLib::DocumentAnnotator::DocumentAnnotator(PositionAnalizer analizer, MoveClassifier moveClassifier) :
    analizer(analizer), moveClassifier(moveClassifier) {

    }

void
SpellChessLib::DocumentAnnotator::setPositionAnalizedCallback(std::function<void()> callback){
    onPositionAnalized = callback;
}

void
SpellChessLib::DocumentAnnotator::setFinishCallback(std::function<void()> callback){
    onFinish = callback;
}


void    
SpellChessLib::DocumentAnnotator::operator()(std::shared_ptr<SpellChessDocument> document){
    analize(*document);
    annotate(*document);
    onFinish();
}

class AnalisisInfoReceiver : public UCILoader::EventReceiver {
    UCILoader::UciScore eval = UCILoader::UciScore::fromCentipawns(0);
    std::vector<SpellChessLib::SpellChessMove> pv;
    public:

    UCILoader::UciScore getEval() const {
        return eval;
    }

    const std::vector<SpellChessLib::SpellChessMove> & getPv() const {
        return pv;
    }

    uint32_t eventFilter() override {
        return UCILoader::NamedEngineEvents::InfoReceived;
    };

    void receiveEvent(const UCILoader::EngineEvent* event) override {
        auto info = (const UCILoader::Info<SpellChessLib::SpellChessMove> *)event->getPayload();
        
        if (info == nullptr)
            return;

        if (info->getType() == UCILoader::Score) {
            eval = info->getAsScore();
        } else if (info->getType() == UCILoader::Pv) {
            pv = info->getMoveArray();
        }
    };
};

SpellChessLib::PlayerColor
getPlayerColorFromSearchRequest( const SpellChessLib::GameState & root, 
        const std::vector<SpellChessLib::SpellChessMove> & movesSinceRoot) {
    bool flip = movesSinceRoot.size() % 2 != 0;
    
    if (!flip)
        return root.getSideToMove();
    
    return SpellChessLib::flipColor(root.getSideToMove());
}

SpellChessLib::PositionInsights
makeInsightsFromArbiterVerdict(SpellChessLib::PlayerColor pov, SpellChessLib::ArbiterVerdict verdict) {

    UCILoader::UciScore score = UCILoader::UciScore::fromCentipawns(0);

    if (verdict == SpellChessLib::ArbiterVerdict::Checkmate || verdict == SpellChessLib::ArbiterVerdict::KingCapture)
        score = UCILoader::UciScore::fromMateDistance(0);
    
        return {
            "", // comment
            {pov, score},
            {}, // best response
            {}, // best continuation
            {} // nag
        };

};

SpellChessLib::PositionInsights
analizePositionByEngine(
        std::shared_ptr<UCILoader::EngineInstance<SpellChessLib::SpellChessMove>> engine, 
        UCILoader::GoParams<SpellChessLib::SpellChessMove> parameters,
        const SpellChessLib::GameState & root, 
        const std::vector<SpellChessLib::SpellChessMove> & movesSinceRoot
    ) {
    
    SpellChessLib::PlayerColor pov = getPlayerColorFromSearchRequest(root, movesSinceRoot);
    auto arbiterVerdict = arbite(root, movesSinceRoot);
    
    if (arbiterVerdict != SpellChessLib::ArbiterVerdict::None) 
        return makeInsightsFromArbiterVerdict(pov, arbiterVerdict);

    auto receiver = std::make_shared<AnalisisInfoReceiver>();
    engine->connect(receiver);
    auto connection = engine->search(parameters, SpellChessLib::StateFormatter(root), movesSinceRoot);
    
    connection->waitFor(std::chrono::seconds(10));
    
    if (connection->getStatus() != UCILoader::ResultReady)
        throw std::runtime_error("Analisis engine timed out or crashed");
    
    auto searchResult = connection->getResult();

    receiver->unlinkAll(); // disconnect info receiver
    return {
        "",                             // suggested comment
        {pov, receiver->getEval()},     // evaluation
        *searchResult.bestMove,         // best move
        receiver->getPv(),              // best continuation
        {}                              // NAGs
    };
}

SpellChessLib::PositionAnalizer 
SpellChessLib::makeFixedDetphAnalizer(std::shared_ptr<UCILoader::EngineInstance<SpellChessMove>> engine, uint32_t depth){
    UCILoader::GoParamsBuilder<SpellChessMove> builder;
    auto params = builder.withDepth(depth).build();

    return [engine, params](const GameState & root, const std::vector<SpellChessMove> & moves) {
        engine->sync();
        return analizePositionByEngine(engine, params, root, moves);
    };
}

SpellChessLib::PositionAnalizer 
SpellChessLib::makeFixedTimeAnalizer(std::shared_ptr<UCILoader::EngineInstance<SpellChessMove>> engine, std::chrono::milliseconds timePerPos) {
    UCILoader::GoParamsBuilder<SpellChessMove> builder;
    auto params = builder.withMoveTime(timePerPos.count()).build();

    return [engine, params](const GameState & root, const std::vector<SpellChessMove> & moves) {
        engine->sync();
        return analizePositionByEngine(engine, params, root, moves);
    };
}

double estimateWinProb(const UCILoader::UciScore & score) {

    if (score.getUnit() == UCILoader::UciScore::Mate) {
        return score.getValue() >= 0 ? 1 : 0;
    }

    auto centipawns = score.getValue();
    const double alpha  = -std::log(10)/ 400.; // scaling factor for centipawn to win probability conversion
    return 1. / (1. + exp(centipawns * alpha));
};

SpellChessLib::Annotation::MoveClassification
SpellChessLib::SimplePositionClassifier (const GameState & state, const PositionInsights & before, const PositionInsights & after) {
    
    if (state.getSideToMove() == Nobody)
        return Annotation::MoveClassification::NoClassification;

    auto pov = state.getSideToMove();
    ScorePOV eval0 = before.positionScore.toPov(pov);
    ScorePOV eval1 = after.positionScore.toPov(pov);
    double winProbLoss = estimateWinProb(eval0.getScore()) - estimateWinProb(eval1.getScore());

    if (winProbLoss >= 0.20)
        return Annotation::MoveClassification::Blunder;
    else if (winProbLoss >= 0.10)
        return Annotation::MoveClassification::Mistake;
    else if (winProbLoss >= 0.05)
        return Annotation::MoveClassification::Inaccuraccy;
    else 
        return Annotation::MoveClassification::GoodMove;
}
