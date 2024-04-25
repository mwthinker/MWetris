#ifndef MWETRIS_GAME_PLAYER_H
#define MWETRIS_GAME_PLAYER_H

#include "playerboard.h"

#include <tetrisboard.h>

#include <mw/signal.h>

#include <memory>
#include <variant>

namespace mwetris::game {

	class Player;
	using PlayerPtr = std::shared_ptr<Player>;

	class Player {
	public:
		virtual ~Player() = default;

		virtual void update(double timeStep) = 0;

		virtual void updateRestart(tetris::BlockType current, tetris::BlockType next) = 0;

		virtual PlayerBoardPtr getPlayerBoard() const = 0;

		virtual void addRowWithHoles(int nbr) = 0;

		virtual void updatePlayerData(const PlayerData& playerData) = 0;

		virtual const PlayerData& getPlayerData() const = 0;

		virtual void updateGravity(float speed) = 0;

		[[nodiscard]]
		virtual mw::signals::Connection addPlayerBoardUpdateCallback(std::function<void(game::PlayerBoardEvent)>&& callback) = 0;

		[[nodiscard]]
		virtual mw::signals::Connection addEventCallback(std::function<void(tetris::BoardEvent, int)>&& callback) = 0;

		virtual const std::string& getName() const = 0;

		virtual bool isAi() const = 0;

		virtual bool isLocal() const = 0;
	};

}

#endif
