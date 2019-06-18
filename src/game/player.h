#ifndef PLAYER_H
#define PLAYER_H

#include "tetrisboard.h"
#include "playerdata.h"

#include <mw/signal.h>

#include <string>
#include <memory>

namespace tetris {

	class IPlayer;
	using IPlayerPtr = std::shared_ptr<IPlayer>;

	class IPlayer {
	public:
		virtual ~IPlayer() = default;

		virtual std::string getName() const = 0;

		virtual int getLevel() const = 0;

		virtual int getPoints() const = 0;

		virtual int getClearedRows() const = 0;

		virtual int getLevelUpCounter() const = 0;

		virtual bool isGameOver() const = 0;

		virtual int getGameOverPosition() const = 0;

		virtual IDevicePtr getDevice() const = 0;

		virtual const TetrisBoard& getTetrisBoard() const = 0;

		virtual mw::signals::Connection addGameEventListener(
			const std::function<void(BoardEvent, const TetrisBoard&)>& callback) = 0;
	};

}

#endif // PLAYER_H
