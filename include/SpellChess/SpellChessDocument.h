#pragma once

#include <memory>
#include <vector>
#include <cstdint>
#include <mutex>
#include <atomic>
#include <unordered_map>
#include <optional>
#include "GameAnnotations.h"

namespace SpellChessLib {

    class SpellChessDocument : public std::enable_shared_from_this<SpellChessDocument> {
        public:
        class Node;

        private:
        // Helper struct containing information about transition from parent node to child node
        struct NodeTransition {
            const std::shared_ptr<Node> child;
            SpellChessMove move;
        };

        public:
        
        
        using Handle = typename std::weak_ptr<const Node>;
        
        class BadHandle : public std::exception {
            public:
            const char * what() const noexcept override;
        };

        class IllegalMoveError : public std::exception {
            std::string fen;
            SpellChessMove move;
            public:
            IllegalMoveError(const GameState & state, const SpellChessMove & move);
            const char * what() const noexcept override;
        };

        class Node : public std::enable_shared_from_this<Node>{
            friend SpellChessDocument;
            const Handle parent;
            std::string comment;
            const GameState gameState;
            std::vector<NodeTransition> transitions;
            mutable std::mutex mut;
            int nodeId;

            void registerId(SpellChessDocument & document);
            public:
            Node(const GameState & state, Handle parent, SpellChessDocument & doc);
            Node(const Node & other) = delete;
            const GameState & getGameState() const;
            std::string getComment() const;
            int getNodeId() const;
            bool isLeaf() const;

            Handle getChildByMove(const SpellChessMove & move) const;
            Handle getFirstChildNode() const;
            Handle getParentNode() const;
        };

        private:
        std::atomic_bool dirty;
        std::shared_ptr<Node> root;
        std::weak_ptr<Node> tail;
        std::map<int, Node*> idMap;
        mutable std::mutex mut;
        mutable std::mutex tagMut;

        Annotation::TagSection tags;
        Annotation::GameTermination gameResult;

        void initNodeFromAnnotations(std::shared_ptr<Node> node, std::shared_ptr<Node> parent, Annotation::AnnotatedSequence::const_iterator it, Annotation::AnnotatedSequence::const_iterator end);
        void writeNodeToAnnotatedSequence(const Node & node, Annotation::AnnotatedSequence & seq) const;
        std::shared_ptr<Node> makeChildNode(std::shared_ptr<Node> node, const SpellChessMove & move);
        Node & referenceNodeByHandle(Handle h);

        public:

        // empty document constructor
        SpellChessDocument(const GameState & root,
             const Annotation::SevenTagRoster & roster,
             Annotation::GameTermination result = Annotation::GameTermination::Unknown);
        
        // construct document from annotated sequence
        SpellChessDocument(const Annotation::GameAnnotations & annotatedGame); 

        bool isDirty() const;
        bool clearDirtyFlag(); // sets dirty flag to zero and returns its old value

        // get root node of the document
        Handle getRoot() const;
            
        // get last position in main variation
        Handle getTail() const;

        // get parent node of given handle
        Handle getPreviousNode(Handle node) const;

        // get first child (main variation) of given handle
        Handle getNextNode(Handle node) const;

        // Map handle to id space. -1 means the node is invalid
        int getNodeId(Handle node) const;

        // Maps id value back to Handle type
        Handle getNodeById(int id) const;

        // get children of a node
        std::vector<Handle> getChildren(Handle node) const;

        // Set comment on selected node 
        void setComment(Handle where, const std::string & comment);

        // Make move and return the handle to resulting position.
        // If the move leads to one of existing children, the handle to this will child will be returned.
        // Otherwise handle to newly created node is returned.
        Handle makeMoveAndAdvance(Handle where, const SpellChessMove & move);

        // Return copy of position at given position
        GameState getGameState(Handle where) const;

        // Return copy of string at given position
        std::string getComment(Handle where) const;

        // Compatibility with serialization APIs
        Annotation::GameAnnotations toAnnotations() const;

        std::optional<std::string> getTag(const std::string & tagName) const;
        
        void setTag(const std::string & tagName, const std::string & tagValue);
        void setTag(const Annotation::TagPair & tag);
        void setGameResult(Annotation::GameTermination gameResult);
    };   

    std::shared_ptr<SpellChessDocument> createEmptySpellChessDocument(const GameState & root);
    std::shared_ptr<SpellChessDocument> createEmptySpellChessDocument(const std::string & rootFen = SpellChessLib::SPELL_START_POS);
    

};