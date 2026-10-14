#pragma once

#include <cstdint>
#include <SpellChess/Board.h>


 namespace UCILoader { template <class Move> class EngineInstance; }

namespace Grimoire {

    struct Configuration;

    struct GameNode {
        SpellChessLib::GameState state;
        SpellChessLib::SpellChessMove move; 
        int32_t  evalution;             // evalution of a given position (in centipawns)
        uint32_t childrenCount;         // number of children
        uint32_t lvl;                   // distance froom root
        GameNode * children;            // pointer to array of children associated with this node
        bool evaluated;                 // was node evaluated by the engine
        bool expanded;                  // were children of this node generated
        bool pruned;                    // node was considered not worth including in the opening book
    };

    extern void deallocate_game_tree(GameNode * root);
    extern GameNode * createRoot();
    extern bool expand_node(GameNode * node, Configuration * config, UCILoader::EngineInstance<SpellChessLib::SpellChessMove> * engine);
    extern bool evaluate_node(GameNode * node, Configuration * config, UCILoader::EngineInstance<SpellChessLib::SpellChessMove> * engine);
    extern void build_game_tree(GameNode * root, Configuration * config,  UCILoader::EngineInstance<SpellChessLib::SpellChessMove> * engine);
};