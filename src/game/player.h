#ifndef MWETRIS_GAME_PLAYER_H
#define MWETRIS_GAME_PLAYER_H

#include "tetrisboardcomponent.h"
#include "playerdata.h"

#include <mw/signal.h>

#include <string>
#include <memory>

namespace tetris::game {

	class Player;
	using PlayerPtr = std::shared_ptr<Player>;

	class Player {
	public:
		virtual ~Player() = default;

		virtual std::string getName() const = 0;

		virtual int getLevel() const = 0;

		virtual int getPoints() const = 0;

		virtual int getClearedRows() const = 0;

		virtual int getLevelUpCounter() const = 0;

		virtual bool isGameOver() const = 0;

		virtual int getGameOverPosition() const = 0;

		virtual DevicePtr getDevice() const = 0;

		virtual const TetrisBoardComponent& getTetrisBoard() const = 0;

		virtual mw::signals::Connection addGameEventListener(
			const std::function<void(BoardEvent, const TetrisBoardComponent&)>& callback) = 0;
	};

}

#endif
