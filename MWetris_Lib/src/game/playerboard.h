#ifndef MWETRIS_GAME_PLAYERBOARD_H
#define MWETRIS_GAME_PLAYERBOARD_H

#include "device.h"
#include "tetrisgameevent.h"

#include <tetrisboard.h>

#include <mw/signal.h>

#include <string>
#include <memory>
#include <variant>

namespace mwetris::game {

	struct DefaultPlayerData {
		int level;
		int points;
	};

	struct SurvivalPlayerData {
		int opponentRows;
	};

	using PlayerData = std::variant<DefaultPlayerData, SurvivalPlayerData>;

	class PlayerBoard;
	using PlayerBoardPtr = std::shared_ptr<PlayerBoard>;

	struct UpdateNextBlock {
		tetris::BlockType next;
	};

	struct UpdateRestart {
		tetris::BlockType current;
		tetris::BlockType next;
	};
	struct UpdatePlayerData {
		PlayerData playerData;
	};
	struct UpdateMove {
		tetris::Move move;
	};
	struct ExternalRows {};

	using PlayerBoardEvent = std::variant<UpdateRestart, UpdatePlayerData, ExternalRows, UpdateMove, UpdateNextBlock>;

	class PlayerBoard : public std::enable_shared_from_this<PlayerBoard> {
	public:
		mw::PublicSignal<PlayerBoard, tetris::BoardEvent, int> gameboardEventUpdate;

		PlayerBoard(const tetris::TetrisBoard& tetrisBoard, const std::string& name);

		virtual ~PlayerBoard() = default;

		const std::string& getName() const {
			return name_;
		}

		virtual int getClearedRows() const = 0;

		bool isGameOver() const;

		virtual const PlayerData& getPlayerData() const = 0;

		int getColumns() const {
			return tetrisBoard_.getColumns();
		}

		int getRows() const {
			return tetrisBoard_.getRows();
		}

		const std::vector<tetris::BlockType>& getBoardVector() const {
			return tetrisBoard_.getBoardVector();
		}

		tetris::BlockType getBlockType(int column, int row) const {
			return tetrisBoard_.getBlockType(column, row);
		}

		tetris::Block getBlockDown() const {
			return tetrisBoard_.getBlock();
		}

		tetris::Block getBlock() const {
			return tetrisBoard_.getBlock();
		}

		// Return the type of the moving block.
		tetris::BlockType getCurrentBlockType() const {
			return tetrisBoard_.getCurrentBlockType();
		}

		// Return the type of the next block.
		tetris::BlockType getNextBlockType() const {
			return tetrisBoard_.getNextBlockType();
		}

		const tetris::TetrisBoard& getTetrisBoard() const {
			return tetrisBoard_;
		}
	protected:
		void updateTetrisBoard(tetris::Move move);

		void restartTetrisBoard(tetris::BlockType current, tetris::BlockType next);

		void setNextTetrisBlock(tetris::BlockType next);

		virtual void handleBoardEvent(tetris::BoardEvent boardEvent, int value);

		tetris::TetrisBoard tetrisBoard_;
		std::string name_;
	};

}

#endif
