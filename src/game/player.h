#ifndef MWETRIS_GAME_PLAYER_H
#define MWETRIS_GAME_PLAYER_H

#include "device.h"
#include "tetrisgameevent.h"

#include <tetrisboard.h>

#include <string>
#include <memory>

#include <mw/signal.h>

namespace mwetris::game {

	class Player;
	using PlayerPtr = std::shared_ptr<Player>;

	class Player : public std::enable_shared_from_this<Player> {
	public:
		mw::PublicSignal<Player, tetris::BoardEvent, int> gameboardEventUpdate;

		explicit Player(const tetris::TetrisBoard& tetrisBoard);

		virtual ~Player() = default;

		virtual const std::string& getName() const = 0;

		virtual int getLevel() const = 0;

		virtual int getPoints() const = 0;

		virtual int getClearedRows() const = 0;

		bool isGameOver() const;

		virtual int getGameOverPosition() const = 0;

		virtual const DevicePtr& getDevice() const = 0;

		const tetris::TetrisBoard& getTetrisBoard() const {
			return tetrisBoard_;
		}

		const std::string& getUniqueId() const;

	protected:
		void updateTetrisBoard(tetris::Move move);

		void restartTetrisBoard(tetris::BlockType current, tetris::BlockType next);

		void setNextTetrisBlock(tetris::BlockType next);

		virtual void handleBoardEvent(tetris::BoardEvent boardEvent, int value);

		tetris::TetrisBoard tetrisBoard_;
		std::string uniqueId_;
	};

}

#endif
