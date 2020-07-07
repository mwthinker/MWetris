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

	class TetrisBoard : public RawTetrisBoard {
	public:
		TetrisBoard(int columns, int rows, BlockType current, BlockType next);

		TetrisBoard(const std::vector<BlockType>& board,
			int rows, int columns, Block current, BlockType next,
			int savedRowsRemoved = 0);

		TetrisBoard(const TetrisBoard&) = delete;
		TetrisBoard& operator=(const TetrisBoard&) = delete;

		TetrisBoard(TetrisBoard&& other) noexcept = default;
		TetrisBoard& operator=(TetrisBoard&& other) noexcept = default;

		void restartBoard(BlockType current, BlockType next);

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
		void triggerEvent(BoardEvent gameEvent) override;

		std::vector<BlockType> addExternalRows() override;

		std::vector<BlockType> squaresToAdd_;
		mw::Signal<BoardEvent, const TetrisBoard&> listener_;
		Random random_;
		int turns_{};
		int rowsRemoved_{};
	};

}

#endif
