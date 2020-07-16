#ifndef MWETRIS_GAME_TETRISBOARDWRAPPER_H
#define MWETRIS_GAME_TETRISBOARDWRAPPER_H


#include "block.h"
#include "tetrisboard.h"
#include "random.h"
#include "helper.h"

#include <mw/signal.h>

#include <vector>

namespace tetris {

	enum class TetrisEvent {
		BlockCollision,
		CurrentBlockUpdated,
		ExternalRowsAdded,

		PlayerMovesBlockUpdated,
		PlayerMovesBlockLeft,
		PlayerMovesBlockRight,
		PlayerMovesBlockDown,
		PlayerMovesBlockDownGround,
		GravityMovesBlock,

		OneRowRemoved,
		TwoRowRemoved,
		ThreeRowRemoved,
		FourRowRemoved,
		RowToBeRemoved,
		GameOver
	};

	class TetrisBoardWrapper {
	public:
		TetrisBoardWrapper(int columns, int rows, BlockType current, BlockType next);

		TetrisBoardWrapper(const std::vector<BlockType>& board,
			int rows, int columns, Block current, BlockType next,
			int savedRowsRemoved = 0);

		TetrisBoardWrapper(const TetrisBoardWrapper&) = delete;
		TetrisBoardWrapper& operator=(const TetrisBoardWrapper&) = delete;

		TetrisBoardWrapper(TetrisBoardWrapper&& other) noexcept = default;
		TetrisBoardWrapper& operator=(TetrisBoardWrapper&& other) noexcept = default;

		void restart(BlockType current, BlockType next);

		// Add rows to be added at the bottom of the board at the next change of the moving block.
		void addRows(const std::vector<BlockType>& blockTypes);

		mw::signals::Connection addGameEventListener(const std::function<void(BoardEvent, const TetrisBoardWrapper&)>& callback);

		const TetrisBoard& getTetrisBoard() const;

		int getTurns() const {
			return turns_;
		}

		int getRemovedRows() const {
			return nbrOneLines_ + nbrTwoLines_ * 2 + nbrThreeLines_ * 3 + nbrFourLines_ * 4;
		}

		int getRows() const {
			return tetrisBoard_.getRows();
		}

		int getColumns() const {
			return tetrisBoard_.getColumns();
		}

		bool isGameOver() const {
			return tetrisBoard_.isGameOver();
		}

		int getRowToBeRemoved() const {
			return rowToBeRemoved_;
		}

		int getNbrExternalRowsAdded() const {
			return externalRowsAdded_;
		}

		BlockType getBlockType() const {
			return tetrisBoard_.getBlockType();
		}

		BlockType getBlockType(int column, int row) const {
			return tetrisBoard_.getBlockType(column, row);
		}

		Block getBlock() const {
			return tetrisBoard_.getBlock();
		}

		void update(Move move) {
			tetrisBoard_.update(move, [&](BoardEvent boardEvent, int value) {
				triggerEvent(boardEvent, value);
			});
		}

		void setNextBlock(BlockType next) {
			tetrisBoard_.setNextBlock(next);
		}

		BlockType getNextBlockType() const {
			return tetrisBoard_.getNextBlockType();
		}

		const std::vector<BlockType>& getBoardVector() const {
			return tetrisBoard_.getBoardVector();
		}

		int getNbrOneRowsRemoved() const {
			return nbrOneLines_;
		}

		int getNbrTwoRowsRemoved() const {
			return nbrTwoLines_;
		}

		int getNbrThreeRowsRemoved() const {
			return nbrThreeLines_;
		}

		int getNbrFourRowsRemoved() const {
			return nbrFourLines_;
		}

	private:
		void triggerEvent(BoardEvent gameEvent, int value);

		TetrisBoard tetrisBoard_;
		mw::Signal<BoardEvent, const TetrisBoardWrapper&> boardEventCallbacks_;
		std::vector<BlockType> squaresToAdd_;
		Random random_;
		int turns_{};
		int nbrOneLines_{};
		int nbrTwoLines_{};
		int nbrThreeLines_{};
		int nbrFourLines_{};
		int rowToBeRemoved_{};
		int externalRowsAdded_{};
	};

}

#endif
