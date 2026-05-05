#include "GameNode.h"
#include <SpellChess/MoveGen.h>
#include <SpellChess/ZobristHash.h>

#include <UCILoader/Parser.h>
#include <sstream>
#include <queue>
#include <set>
#include <iostream>

void deallocate_game_tree_recursive(Grimoire::GameNode* node ){
    if (node->children == nullptr)
        return;

    for (size_t i = 0; i < node->childrenCount; ++i)
        deallocate_game_tree_recursive(node->children + i);
    
    delete[] node->children;
};

void Grimoire::deallocate_game_tree(GameNode *root)
{
    deallocate_game_tree_recursive(root);
    delete root;
}

Grimoire::GameNode *Grimoire::createRoot(){
    GameNode * result = new GameNode;
    SpellChessLib::FenParser parser;
    result->state = parser.fromSpellChessFen(SpellChessLib::SPELL_START_POS);
    result->children = nullptr;
    result->childrenCount = 0;
    result->lvl = 0;
    result->expanded = result->evaluated = result->pruned = false;
    result->evalution = -10000;
    return result;
}



class GrimoireInfoReceiver : public UCILoader::EventReceiver { 

    std::vector<SpellChessLib::SpellChessMove> moves;
    
    UCILoader::UciScore score = UCILoader::UciScore::fromMateDistance(-1);

    std::vector<std::string> tokenize(const std::string& line) const
	{
		std::vector<std::string> tokens;
		std::stringstream sstream(line);
		std::string token;

		while (sstream) {
			token = "";
			sstream >> token;
			if (token.size()) tokens.push_back(token);
		}

		return tokens;
	}

    public:

	uint32_t eventFilter() override {
        return UCILoader::NamedEngineEvents::InfoReceived;
    }

    void receiveEvent(const UCILoader::EngineEvent* event) override {
        UCILoader::Info<SpellChessLib::SpellChessMove>* info = (UCILoader::Info<SpellChessLib::SpellChessMove>* )event->getPayload();
        
        // std::cout << info->getStringValue() << std::endl;
        
        if (info->getType() == UCILoader::InfoString) {

            auto tokens = tokenize(info->getStringValue());

            if (tokens[0] == "grimoire") {
                moves.clear();
                for(size_t i = 1; i < tokens.size(); ++i) 
                        try {
                            moves.push_back(SpellChessLib::parseMove(tokens[i].c_str()));
                        } catch (std::exception ignored) {};
            };
            
        };

        if (info->getType() == UCILoader::Score) {
            score = info->getAsScore();
        }

    }; 

    const std::vector<SpellChessLib::SpellChessMove> & getMoves() const {
        return moves;
    }

    const UCILoader::UciScore & getScore() {
        return score;
    }

};

int32_t uciScoreToCentipawns(const UCILoader::UciScore & score) {
    if (score.getUnit() == UCILoader::UciScore::Centipawn)
        return score.getValue();

    int mateDistance = score.getValue();
    if (mateDistance > 0)
        return 10000 - mateDistance;
    else
        return -10000 - mateDistance;
}

template <bool expand, bool eval>
bool visit_node (Grimoire::GameNode *node, Grimoire::Configuration * config ,UCILoader::EngineInstance<SpellChessLib::SpellChessMove> * engine) {

    if constexpr (!expand && !eval)
        return false;


    SpellChessLib::MoveGenerator gen(node->state);

    if (gen.generateMoves().empty()) {
       return false; 
    }

    auto grimoire_extension_listener = std::make_shared<GrimoireInfoReceiver>();
    
    engine->connect(grimoire_extension_listener);
    auto searchConnection = engine->search(config->go_params, SpellChessLib::StateFormatter(node->state));
    searchConnection->waitFor(config->engine_timeout);
    
    grimoire_extension_listener->unlinkAll();
    if ( searchConnection->getStatus() != UCILoader::SearchStatusCode::ResultReady) {
        return false;
    };

    auto grimoire_moves = grimoire_extension_listener->getMoves();
    auto node_score = grimoire_extension_listener->getScore();

    if constexpr (expand) {
        node->expanded = true;

        if (grimoire_moves.size() == 0)
            goto EVAL;

        node->children = new Grimoire::GameNode[grimoire_moves.size()];
        node->childrenCount = grimoire_moves.size();
        for(size_t i = 0; i < grimoire_moves.size(); ++i) {
            node->children[i].children = nullptr;
            node->children[i].childrenCount = 0;
            node->children[i].lvl = node->lvl + 1;
            node->children[i].expanded = node->children[i].evaluated = node->children[i].pruned = false;
            node->children[i].evalution = 0;
            node->children[i].move = grimoire_moves[i];
            node->children[i].state = node->state;
            node->children[i].state.makeMove(grimoire_moves[i]);
        };
    }
    
    EVAL:

    if constexpr (eval) {
        node->evalution = uciScoreToCentipawns(node_score);
        node->evaluated = true;
        node->pruned = node->evalution > config->evalBounds;
        std::cout << "Eval: " << node->evalution << "\n";
        
    }

    return true;
}

bool Grimoire::expand_node(GameNode *node, Configuration * config ,UCILoader::EngineInstance<SpellChessLib::SpellChessMove> * engine){
    return visit_node<true, true>(node, config, engine);    
}

bool Grimoire::evaluate_node(GameNode *node, Configuration *config, UCILoader::EngineInstance<SpellChessLib::SpellChessMove> *engine){
    return visit_node<false, true>(node, config, engine);   
}

void Grimoire::build_game_tree(GameNode *root, Configuration *config, UCILoader::EngineInstance<SpellChessLib::SpellChessMove> *engine){
    std::queue<GameNode*> nodes;
    uint32_t expandedNodes = 0;
    nodes.emplace(root);

    GameNode * currentNode;

    std::set<uint64_t> seen;
    int32_t bound = config->evalBounds;

    while(!nodes.empty()) {
        currentNode = nodes.front();
        nodes.pop();

        uint64_t hash = SpellChessLib::calculateHash(currentNode->state);
        if(currentNode->lvl != 0)
        std::cout << "Move " << SpellChessLib::spellChessMoveToString(currentNode->move) << std::endl;

        if (seen.find(hash) != seen.end()) {
            std::cout << "Dropped move " << SpellChessLib::spellChessMoveToString(currentNode->move) << std::endl;
            continue;
        }
        seen.insert(hash);

        bool shouldExpand = expandedNodes < config->extendedNodeLimit &&
         currentNode->lvl < config->maximumOpeningDepth;  
        
        if (shouldExpand) {
            expand_node(currentNode, config, engine);
            bool withinBounds = !currentNode->pruned;

            if(withinBounds)
                ++expandedNodes;

            if (currentNode->children && withinBounds)
                for(size_t i = 0; i < currentNode->childrenCount; ++i)
                    nodes.push(currentNode->children + i);            
        } else 
            evaluate_node(currentNode, config, engine);
        std::cout << "Progress: expanded " << expandedNodes <<"/" << config->extendedNodeLimit << std::endl;
        std::cout << "Queue size: " << nodes.size() << std::endl;
    }
    std::cout << "Work finished "<< std::endl;
};
