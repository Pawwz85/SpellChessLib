#pragma once

#include <SpellChess/GameAnnotations.h>
#include <optional>
#include <memory>

#include <QObject>
#include <QWidget>
#include <QTextDocument>
#include <QTextBrowser>
#include <QLayout>

namespace BoardViewer {
    class MoveListModel : public QObject {
        Q_OBJECT
        public:
        using Sequence = typename SpellChessLib::Annotation::AnnotatedSequence;
        using RootPos  = typename SpellChessLib::GameState;
        protected:
            Sequence annotations;
            RootPos initialPosition;
        public:
        MoveListModel();
        MoveListModel(const RootPos & root, const Sequence & seq) : initialPosition(root), annotations(seq) {};

        virtual ~MoveListModel() = default;

        inline const Sequence & getSequence() const {
            return annotations;
        };

        inline const RootPos  & getInitialState() const {
            return initialPosition;
        };

        inline void set(const RootPos & pos, const Sequence & seq) {
            annotations = seq;
            initialPosition = pos;
            changed();
        };

        signals:
        void changed();
    };

    class MoveListView : public QWidget {
        Q_OBJECT
        std::shared_ptr<MoveListModel> model;
        QLayout * layout;

        protected:
        QTextDocument * doc;
        QTextBrowser * browser;

        void writeAnnotationGlyph(QTextCursor & cursor, const SpellChessLib::Annotation::NumericAnnotationGlyph & glyph);
        void writeMoveToken(QTextCursor & cursor, const SpellChessLib::SpellChessMove * move);
        void writeVariation(QTextCursor & cursor, const SpellChessLib::Annotation::AnnotatedSequence & variation);
        void writeComment(QTextCursor & cursor, const std::string & comment);
        void writeTab(QTextCursor & cursor);
        
        void writeNumberIndicator(QTextCursor & cursor, int indicator);

        void writeExtras(QTextCursor & cursor, MoveListModel::Sequence::const_iterator begin, MoveListModel::Sequence::const_iterator end);

        protected slots:
        void handleModelChanged();

        public:
        MoveListView(std::shared_ptr<MoveListModel> model, QWidget * parent = nullptr);
    };
};