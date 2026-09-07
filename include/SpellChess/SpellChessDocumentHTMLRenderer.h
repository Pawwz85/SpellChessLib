#pragma once

#include "SpellChessDocument.h"

#include <list>
#include <unordered_map>
#include <optional>
#include <memory>

namespace SpellChessLib {

    namespace HTML {

        // Abstract base of all html elements.
        class Element {
            public:
            virtual ~Element() = default;
            virtual bool isLeaf() const = 0;

            /*
                Returns true if and only if block is instance of HTML::Block.
                Scallable? No. Do I care? No.
            */
            virtual bool isBlock() const; 

            virtual void onCurrentPosChanged(SpellChessDocument::Handle currentPos);

            virtual std::string toText() const = 0; 
        };

        // Abstract base class for all html leaf elements
        class LeafElement : public Element {
            protected:
            
            public:
            virtual ~LeafElement() = default;
            bool isLeaf() const override;
        };

        // Abstract class for all container elements
        class ContainerElement : public Element {
            std::list<std::unique_ptr<Element>> children;
            virtual std::string openTag() const = 0;
            virtual std::string closeTag() const = 0;

            const char elementSeparator;

            void pushBackInternal(std::unique_ptr<Element> && newElement);
            
            public:
            ContainerElement(char elementSeperator = '\n');
        
            bool isLeaf() const override;
            std::string toText() const override;
            void onCurrentPosChanged(SpellChessDocument::Handle currentPos) override;

            std::list<std::unique_ptr<Element>> & getChildren();
            const std::list<std::unique_ptr<Element>> & getChildren() const;
            
            template <class T>
            T & pushBack(std::unique_ptr<T> && newChild) {
                T * raw = newChild.release();
                pushBackInternal(std::unique_ptr<Element>(raw));
                return *raw;
            };
        };

        // Root element of html document.
        class Root : public ContainerElement {
            std::string openTag() const override;
            std::string closeTag() const override;
            public:
            Root();
        };

        class Variation : public ContainerElement {
            std::string openTag() const override;
            std::string closeTag() const override;
            public:
            Variation();
        };

        class MoveToken : public LeafElement {
            SpellChessDocument::Arc arc;
            bool bolt = false;

            std::string openAnchor(SpellChessDocument::Handle pos) const;
            std::string closeAnchor() const;

            std::string tokenValue;

            public:
            MoveToken(const SpellChessDocument::Arc & arc);
            std::string toText() const override;
            void onCurrentPosChanged(SpellChessDocument::Handle currentPos) override;
            
        };

        class CommentToken : public LeafElement {
            std::string text;
            SpellChessDocument::Handle pos;

            bool isCurrentPos = false;
            public:
            CommentToken(const std::string & text, SpellChessDocument::Handle position);
            std::string toText() const override;
            void onCurrentPosChanged(SpellChessDocument::Handle currentPos) override;
        };

        enum BlockType {
            LeftColumn,
            RightColumn,
            Full
        };

        class Block : public ContainerElement {
            std::string openTag() const override;
            std::string closeTag() const override;

            unsigned int numberIndicator;
            BlockType type;

            public:
            Block(unsigned int numberIndicator, BlockType type);
            bool isBlock() const override;
            unsigned int getNumberIndicator() const;
            BlockType getType() const;
            void setType(BlockType type);

        };
    };

    class HTMLDocumentColorPalete {
        std::unordered_map< Annotation::MoveClassification, std::string> movePallete;

        HTMLDocumentColorPalete();
        public:
        HTMLDocumentColorPalete(const HTMLDocumentColorPalete &) = delete;
        HTMLDocumentColorPalete(HTMLDocumentColorPalete &&) = delete;
        
        std::optional<std::string> getMoveTokenColor(const Annotation::MoveClassification & moveType);
        static HTMLDocumentColorPalete & getInstance();
    };

    extern std::unique_ptr<HTML::Element> toHtml(const SpellChessDocument & document);
};