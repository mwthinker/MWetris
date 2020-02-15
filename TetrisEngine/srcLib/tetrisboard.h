#ifndef TETRISBOARD_H
#define TETRISBOARD_H

#include "block.h"
#include "rawtetrisboard.h"

#include "random.h"

#include <mw/signal.h>

#include <vector>

namespace tetris {

	BlockType randomBlockType();

	std::vector<BlockType> generateRow(const RawTetrisBoard& board, double squaresPerLength);

	std::vector<BlockType> generateRow(int width, int holes);

	// Represents a tetris board.
	class TetrisBoard : public RawTetrisBoard {
	public:
		TetrisBoard(int columns, int rows, BlockType current, BlockType next);

		TetrisBoard(const std::vector<BlockType>& board,
			int rows, int columns, Block current, BlockType next,
			int savedRowsRemoved = 0);

		// Copy a tetrisboard except the connected gameEventListener.
		TetrisBoard(const TetrisBoard&);
		TetrisBoard& operator=(const TetrisBoard&);

		TetrisBoard(TetrisBoard&& other) = default;
		TetrisBoard& operator=(TetrisBoard&& other) = default;

		// Restarts the board. Resets all states. Current and next represents the two starting blocks.
		void restart(BlockType current, BlockType next);

		// Add rows to be added at the bottom of the board at the next change of the moving block.
		void addRows(const std::vector<BlockType>& blockTypes);

		int getTurns() const {
			return turns_;
		}

		mw::signals::Connection addGameEventListener(const std::function<void(BoardEvent, const TetrisBoard&)>& callback);

		int getRemovedRows() const {
			return rowsRemoved_;
		}

	private:
		// @RawTetrisBoard
		void triggerEvent(BoardEvent gameEvent) override;

		// @RawTetrisBoard
		std::vector<BlockType> addExternalRows() override;

		std::vector<BlockType> squaresToAdd_;
		mw::Signal<BoardEvent, const TetrisBoard&> listener_;
		Random random_;
		int turns_{};
		int rowsRemoved_{};
	};

} // Namespace tetris.

#endif // TETRISBOARD_H
