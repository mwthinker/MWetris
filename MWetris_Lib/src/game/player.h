#ifndef MWETRIS_GAME_PLAYER_H
#define MWETRIS_GAME_PLAYER_H

#include "device.h"
#include "tetrisgameevent.h"

#include <tetrisboard.h>

#include <string>
#include <memory>

#include <mw/signal.h>

namespace mwetris::game {

	class PlayerBoard;
	using PlayerBoardPtr = std::shared_ptr<PlayerBoard>;

	class PlayerBoard : public std::enable_shared_from_this<PlayerBoard> {
	public:
		mw::PublicSignal<PlayerBoard, tetris::BoardEvent, int> gameboardEventUpdate;

		PlayerBoard(const tetris::TetrisBoard& tetrisBoard, const std::string& name);

		virtual ~PlayerBoard() = default;

		const std::string& getName() const {
			return name_;
		}

		virtual int getLevel() const = 0;

		virtual int getPoints() const = 0;

		virtual int getClearedRows() const = 0;

		bool isGameOver() const;

		virtual int getGameOverPosition() const = 0;

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
		std::string name_;
		std::string uniqueId_;
	};

}

#endif
