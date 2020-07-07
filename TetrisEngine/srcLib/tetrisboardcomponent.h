#ifndef TETRISBOARD_H
#define TETRISBOARD_H

#include "block.h"
#include "tetrisboard.h"

#include "random.h"

#include <mw/signal.h>

#include <vector>

namespace tetris {

	BlockType randomBlockType();

	std::vector<BlockType> generateRow(const TetrisBoard& board, double squaresPerLength);

	std::vector<BlockType> generateRow(int width, int holes);

	class TetrisBoardComponent : public TetrisBoard {
	public:
		TetrisBoardComponent(int columns, int rows, BlockType current, BlockType next);

		TetrisBoardComponent(const std::vector<BlockType>& board,
			int rows, int columns, Block current, BlockType next,
			int savedRowsRemoved = 0);

		TetrisBoardComponent(const TetrisBoardComponent&) = delete;
		TetrisBoardComponent& operator=(const TetrisBoardComponent&) = delete;

		TetrisBoardComponent(TetrisBoardComponent&& other) noexcept = default;
		TetrisBoardComponent& operator=(TetrisBoardComponent&& other) noexcept = default;

		void restartBoard(BlockType current, BlockType next);

		// Add rows to be added at the bottom of the board at the next change of the moving block.
		void addRows(const std::vector<BlockType>& blockTypes);

		int getTurns() const {
			return turns_;
		}

		mw::signals::Connection addGameEventListener(const std::function<void(BoardEvent, const TetrisBoardComponent&)>& callback);

		int getRemovedRows() const {
			return rowsRemoved_;
		}

	private:
		void triggerEvent(BoardEvent gameEvent) override;

		std::vector<BlockType> addExternalRows() override;

		std::vector<BlockType> squaresToAdd_;
		mw::Signal<BoardEvent, const TetrisBoardComponent&> listener_;
		Random random_;
		int turns_{};
		int rowsRemoved_{};
	};

}

#endif
