#pragma once
#include <memory>
#include <cstdint>
#include <QString>
#include <QCoreApplication>
#include <SpellChess/GameRunnerConfiguration.h>
#include <SpellChess/PlayerEntry.h>
#include <SpellChess/Player.h>

namespace GameSession {

    class PlayerDefinition {
        public:
        virtual ~PlayerDefinition() = default;
        virtual bool isHuman() const = 0;
        virtual bool isEngine() const = 0;
        virtual const QString  & getName() const = 0;
        virtual uint32_t  getPlayerId() const = 0; 
    };

    class HumanPlayerDefinition : public PlayerDefinition{
        public:
        bool isHuman() const;
        bool isEngine() const;
        const QString & getName() const;
        uint32_t getPlayerId() const; 

    };

    class CPUPlayerDefinition : public PlayerDefinition {
        SpellChessLib::PlayerEntry entry;
        QString name;
        public:
        CPUPlayerDefinition(const SpellChessLib::PlayerEntry & engineEntry); 
        bool isHuman() const override;
        bool isEngine() const override;
        const QString & getName() const override;
        uint32_t getPlayerId() const; 

    };

    class OpenSessionRequest {
        std::shared_ptr<PlayerDefinition> whitePlayer;
        std::shared_ptr<PlayerDefinition> blackPlayer;      
        SpellChessLib::GameRunnerConfiguration gameRunnerConfig;

        public:
        OpenSessionRequest(SpellChessLib::GameRunnerConfiguration gameRunnerConfig, std::shared_ptr<PlayerDefinition> whitePlayer, std::shared_ptr<PlayerDefinition> blackPlayer);
        const PlayerDefinition & getWhitePlayer() const;
        const PlayerDefinition & getBlackPlayer() const;
        const SpellChessLib::GameRunnerConfiguration & getGameRunnerConfig() const;
    };

    extern std::shared_ptr<PlayerDefinition> defaultHumanPlayerDef;

    class OpenSessionRequestBuilder {
        std::shared_ptr<PlayerDefinition> whitePlayer;
        std::shared_ptr<PlayerDefinition> blackPlayer;
        std::shared_ptr<SpellChessLib::GameRunnerConfigurationBuilder> gameRunnerConfigBuilder;

        public:
        OpenSessionRequestBuilder();
        OpenSessionRequestBuilder & withWhitePlayer(std::shared_ptr<PlayerDefinition> player);
        OpenSessionRequestBuilder & withBlackPlayer(std::shared_ptr<PlayerDefinition> player);
        std::shared_ptr<SpellChessLib::GameRunnerConfigurationBuilder> & getGameRunnerConfigBuilder();
        OpenSessionRequest build() const;
    };
};