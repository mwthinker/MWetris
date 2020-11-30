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

		explicit Player(const tetris::TetrisBoard& tetrisBoard)
			: tetrisBoard_{tetrisBoard} {
		}

		virtual ~Player() {
		}

		virtual std::string getName() const = 0;

		virtual int getLevel() const = 0;

		virtual int getPoints() const = 0;

		virtual int getClearedRows() const = 0;

		virtual int getLevelUpCounter() const = 0;

		bool isGameOver() const;

		virtual int getGameOverPosition() const = 0;

		virtual DevicePtr getDevice() const = 0;

		const tetris::TetrisBoard& getTetrisBoard() const {
			return tetrisBoard_;
		}

	protected:
		void updateTetrisBoard(tetris::Move move);

		void restartTetrisBoard(tetris::BlockType current, tetris::BlockType next);

		void setNextTetrisBlock(tetris::BlockType next);

		virtual void handleBoardEvent(tetris::BoardEvent boardEvent, int value);

	private:
		tetris::TetrisBoard tetrisBoard_;
	};

}

#endif
