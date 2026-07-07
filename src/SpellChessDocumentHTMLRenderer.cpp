#include <SpellChess/SpellChessDocumentHTMLRenderer.h>

namespace SpellChessLib {
    namespace HTML {

        bool Element::isBlock() const {
            return false;
        }

        void Element::onCurrentPosChanged(SpellChessDocument::Handle currentPos) {};

        bool LeafElement::isLeaf() const {
            return true;
        }

        ContainerElement::ContainerElement(char sep): elementSeparator(sep) {
        }

        bool ContainerElement::isLeaf() const {
            return false;
        }

        std::string ContainerElement::toText() const {
            std::string result = openTag();
            result.push_back(elementSeparator);

            for (auto & child : children) {
                result += child->toText();
                result.push_back(elementSeparator);
            }

            result.push_back(elementSeparator);
            result += closeTag();
            return result;
        };

        void ContainerElement::onCurrentPosChanged(SpellChessDocument::Handle currentPos) {
            for (auto & child : children)
                child->onCurrentPosChanged(currentPos);
        };


        std::list<std::unique_ptr<Element>> & ContainerElement::getChildren() {
            return children;
        };

        const std::list<std::unique_ptr<Element>> & ContainerElement::getChildren() const {
            return children;
        };

        void ContainerElement::pushBackInternal(std::unique_ptr<Element> && newElement){
            children.emplace_back(std::move(newElement));
        }

        std::string Root::openTag() const {
            return "<body>";
        }

        std::string Root::closeTag() const {
            return "</body>";
        }

        Root::Root() {
        }

        std::string Variation::openTag() const {
            return "<ul>";
        }

        std::string Variation::closeTag() const {
            return "</ul>";
        }

        Variation::Variation() {}
    
        Block::Block(unsigned int numberIndicator, BlockType type) : 
        ContainerElement(' '), type(type), numberIndicator(numberIndicator){}

        std::string Block::openTag() const {
            std::string result = "<li>";
            result += std::to_string(numberIndicator) + ".";

            if (type == RightColumn)
                result += ".. \t";

            return result;
        };

        std::string Block::closeTag() const {
            std::string result;
            
            if (type == LeftColumn)
                result += "\t..";

            result += "</li>";
            return result;
        }

        bool Block::isBlock() const {
            return true;
        }

        unsigned int Block::getNumberIndicator() const {
            return numberIndicator;
        }

        BlockType Block::getType() const {
            return type;
        }

        CommentToken::CommentToken(const std::string & text) : text(text) {
        }

        std::string CommentToken::toText() const {
            return "<i style=\"color: orange;\">" + text + "</i>";
        }

        MoveToken::MoveToken(const SpellChessDocument::Arc & arc) : arc(arc){
        }

        std::string MoveToken::toText() const {
            std::string result = spellChessMoveToString(arc.move);
            if (bolt)
               result = "<b>" + result + "</b>";
            return result;
        }

        void MoveToken::onCurrentPosChanged(SpellChessDocument::Handle currentPos) {
            bolt = arc.node.lock() == currentPos.lock();
        }

    };

    void insertMoveTokenNode(HTML::ContainerElement & parent, const SpellChessDocument::Arc & arc,  const SpellChessDocument & document) {
        parent.pushBack(std::make_unique<HTML::MoveToken>(arc));

        std::string currentComment = document.getComment(arc.node);
        if(!currentComment.empty())
                parent.pushBack(std::make_unique<HTML::CommentToken>(currentComment));
    };

    void singleBlockDocumentScan(HTML::ContainerElement & scope, const SpellChessDocument & document, SpellChessDocument::Handle currentPos) {
        auto currentState = document.getGameState(currentPos);
        auto & currentBlock = scope.pushBack(std::make_unique<HTML::Block>(
            currentState.getFullMoveCounter(),
            (currentState.getSideToMove() == SpellChessLib::Black) ? HTML::RightColumn : HTML::LeftColumn
        ));
        
        auto arcs = document.getArcs(currentPos);

        std::string currentComment;

        if (!arcs.empty()) {
            insertMoveTokenNode(currentBlock, arcs[0], document);
        }
        
        for (size_t i = 1; i < arcs.size(); ++i) {
            auto & variant = scope.pushBack(std::make_unique<HTML::Variation>());
            auto newState = document.getGameState(arcs[i].node);
            
            auto & variantBlock = variant.pushBack(std::make_unique<HTML::Block>(
            newState.getFullMoveCounter(),
            (newState.getSideToMove() == SpellChessLib::Black) ? HTML::RightColumn : HTML::LeftColumn
            ));

            insertMoveTokenNode(variantBlock, arcs[i], document);
            singleBlockDocumentScan(variant, document, arcs[i].node); 
        }

        if(!arcs.empty() && !document.getArcs(arcs[0].node).empty())
            singleBlockDocumentScan(scope, document, arcs[0].node);
    }

    void eraseEmptyBlocks(HTML::ContainerElement & scope) {
        auto & children = scope.getChildren();

        HTML::Block * currElement;

        for(auto it = children.begin(); it != children.end(); ) {
            
            if (!it->get()->isBlock()) {
                ++it;
                continue;
            }
            currElement = (HTML::Block*)it->get(); 
            auto currIt = it;
            ++it;

            if (currElement->getChildren().empty())
                children.erase(currIt);
        };

    }

    std::unique_ptr<HTML::Block> mergeBlocks(HTML::Block & block1, HTML::Block & block2) {
        auto result = std::make_unique<HTML::Block>(block1.getNumberIndicator(), HTML::Full);

        for (auto & child : block1.getChildren()) 
            result->pushBack(std::move(child));
        
        for (auto & child : block2.getChildren()) 
            result->pushBack(std::move(child)); 

        return result;
    };

    void mergeHtmlBlocks(HTML::ContainerElement & scope) {
        auto & children = scope.getChildren();

        if (children.size() >= 2) {
            auto previous = children.begin(); 
            auto current = children.begin();
            ++current;

            while(current != children.end()) {
                if(current->get()->isBlock() && previous->get()->isBlock()) {
                    HTML::Block* prev = (HTML::Block*)previous->get();
                    HTML::Block* curr = (HTML::Block*)current->get(); 
                    
                    if (prev->getNumberIndicator() == curr->getNumberIndicator()) {
                        auto merged = mergeBlocks(*prev, *curr);
                        children.erase(current);
                        current = children.emplace(previous, (HTML::Element*)merged.release());
                        children.erase(previous);
                    };
                };

                previous = current;
                ++current;
            };
        }
        
        for (auto & child : children)
            if (!child->isLeaf())
                mergeHtmlBlocks(*(HTML::ContainerElement*)child.get());
    };

    std::unique_ptr<HTML::Element> toHtml(const SpellChessDocument & document) {
        auto result = std::make_unique<HTML::Root>();

        auto currentPos = document.getRoot();

        singleBlockDocumentScan(*result, document, currentPos);
        eraseEmptyBlocks(*result);
        mergeHtmlBlocks(*result);

        return result;
    };
};