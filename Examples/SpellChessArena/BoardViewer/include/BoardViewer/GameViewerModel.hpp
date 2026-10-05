#pragma once

#include "GameRunnerBridge.hpp"
#include "OpenSessionRequest.hpp"
#include "BoardViewerModel.hpp"

#include <SpellChess/AsynchronousPlayer.h>
#include <SpellChess/GameRunner.h>

#include <QCoreApplication> 
#include <QString>

namespace BoardViewer {

    class GameViewerModel : public Model  {
        using PlayerPointer = typename SpellChessLib::Scheduler::ResourcePointer<SpellChessLib::Player>;
        
        QCoreApplication * application;
        GameRunnerBridge runnerAdapter;
        std::shared_ptr<SpellChessLib::GameRunner> gameRunner;
        std::thread* gameThread;
        int gameRunnerId;

        bool isPlayerHuman[3] = {false, false, false}; // Nobody, White, Black
        SpellChessLib::AsynchronousPlayer* players[3] = {nullptr, nullptr, nullptr}; // Non owning pointers to GUI players

        bool event(QEvent * e) override;
        void handleMakeMove(SpellChessLib::SpellChessMove move) override;

        bool isCurrentStateTail() const;
        void dispatchMoveToGameRunner(const SpellChessLib::SpellChessMove & move);
        void navigateDocument(const SpellChessLib::SpellChessMove & move);
        PlayerPointer playerFromDef(const PlayerDefinition & def, const OpenSessionRequest & request);

        public:

        GameViewerModel(std::shared_ptr<SpellChessLib::TimerService> timeServ, const OpenSessionRequest & request,  QCoreApplication * app);
        ~GameViewerModel();

        SpellChessLib::SpellChessDocument::Handle getCurrentPosition() const override;
        std::shared_ptr<const SpellChessLib::SpellChessDocument> getDocument() const override;
        bool canUserSubmitMove() override;
        QString getWhitePlayerName() const override;
        QString getBlackPlayerName() const override;

    private:
        QString whitePlayerName;
        QString blackPlayerName;
    };

};