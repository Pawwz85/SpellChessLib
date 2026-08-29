#pragma once

#include "BoardViewerModel.hpp"

namespace BoardViewer {
    class DocumentViewerModel : public Model {
        void handleMakeMove(SpellChessLib::SpellChessMove move) override;
        public:
        DocumentViewerModel(std::shared_ptr<SpellChessLib::SpellChessDocument> document, QObject * parent = nullptr);
        SpellChessLib::SpellChessDocument::Handle getCurrentPosition() const override;
        std::shared_ptr<const SpellChessLib::SpellChessDocument> getDocument() const override;
        bool canUserSubmitMove() override;
    };
}

