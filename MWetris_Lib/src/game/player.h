#ifndef MWETRIS_GAME_PLAYER_H
#define MWETRIS_GAME_PLAYER_H

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

		virtual void updateRestart() = 0;

		virtual PlayerBoardPtr getPlayerBoard() const = 0;

		virtual void addRowWithHoles(int nbr) = 0;

		virtual void updatePlayerData(const PlayerData& playerData) = 0;

		virtual const PlayerData& getPlayerData() const = 0;

		virtual void updateGravity(float speed) = 0;

		[[nodiscard]]
		virtual mw::signals::Connection addEventCallback(std::function<void(tetris::BoardEvent, int)>&& callback) = 0;
	};

}

#endif
