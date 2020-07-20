#ifndef MWETRIS_GAME_TETRISBOARDWRAPPER_H
#define MWETRIS_GAME_TETRISBOARDWRAPPER_H

#include "eventmanager.h"

#include "block.h"
#include "tetrisboard.h"
#include "random.h"
#include "helper.h"

#include <vector>

namespace tetris::game {

	class BlockCollisionEvent : public Event {
	public:
	};

	class CurrentBlockUpdatedEvent : public Event {
	public:
	};

	class ExternalRowsAddedEvent : public Event {
	public:
	};

	class RowToBeRemovedEvent : public Event {
	public:
		RowToBeRemovedEvent(int row)
			: row{row} {
		}

		int row;
	};

	class RowsRemovedEvent : public Event {
	public:
		RowsRemovedEvent(int nbr)
			: nbr{nbr} {
			
		}
		int nbr;
	};

	class TetrisBoardGameOver : public Event {
	public:
	};

	class Row {
	public:
		Row(int row)
			: row_{row} {

		}

	private:
		int row_;
		std::vector<BlockType> elements_;
	};

	class TetrisBoardWrapper {
	public:
		enum class Event {
			BlockCollision,
			CurrentBlockUpdated,

			PlayerMovesBlock,
			GravityMovesBlock,

			RowsRemoved,
			RowToBeRemoved,
			GameOver
		};

		TetrisBoardWrapper(const TetrisBoard& tetrisBoard, int savedRowsRemoved = 0);

		TetrisBoardWrapper(const TetrisBoardWrapper&) = delete;
		TetrisBoardWrapper& operator=(const TetrisBoardWrapper&) = delete;

		TetrisBoardWrapper(TetrisBoardWrapper&& other) noexcept = default;
		TetrisBoardWrapper& operator=(TetrisBoardWrapper&& other) noexcept = default;

		void restart(BlockType current, BlockType next);

		// Add rows to be added at the bottom of the board at the next change of the moving block.
		void addRows(const std::vector<BlockType>& blockTypes);

		int getSenderId() const {
			return 0;
		}

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

		template <class EventCallback>
		void update(Move move, EventCallback&& eventCallback) {
			static_assert(std::is_invocable_v<EventCallback, Event, int>, "EventCallback must be in the form: void(BoardEvent, int) ");

			tetrisBoard_.update(move, [&](BoardEvent boardEvent, int value) {
				switch (boardEvent) {
					case BoardEvent::BlockCollision:
						eventCallback(TetrisBoardWrapper::Event::BlockCollision, value);
						break;
					case BoardEvent::RowToBeRemoved:
						rowToBeRemoved_ = value;
						break;
					case BoardEvent::CurrentBlockUpdated:
						externalRowsAdded_ = tetrisBoard_.addExternalRows(squaresToAdd_);
						squaresToAdd_.clear();
						++turns_;
						break;
					case BoardEvent::RowsRemoved:
						nbrOneLines_ += value;
						break;
				}
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
		TetrisBoard tetrisBoard_;
		std::vector<BlockType> squaresToAdd_;

		std::vector<std::shared_ptr<Row>> rows_;
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
