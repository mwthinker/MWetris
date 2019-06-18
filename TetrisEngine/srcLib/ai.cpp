#include "ai.h"

#include <calc/cache.h>
#include <calc/calculatorexception.h>

#include <limits>
#include <cmath>
#include <iostream>

namespace tetris {

	namespace {

		// Calculate and return all possible states for the block provided.
		std::vector<Ai::State> calculateAllPossibleStates(const RawTetrisBoard& board, Block block) {
			std::vector<Ai::State> states;

			// Valid block position?
			if (!board.collision(block)) {
				// Go through all rotations for the block.
				for (int rotationLeft = 0; rotationLeft <= block.getNumberOfRotations(); ++rotationLeft, block.rotateLeft()) {
					// Go left.
					Block horizontal = block;
					horizontal.moveLeft();

					int stepsLeft = 1;
					// Go left until obstacle.
					while (!board.collision(horizontal)) {
						Block vertical = horizontal;
						int stepsDown = 1;
						vertical.moveDown();
						while (!board.collision(vertical)) {
							++stepsDown;
							vertical.moveDown();
						}
						states.emplace_back(stepsLeft, rotationLeft);

						++stepsLeft;
						horizontal.moveLeft();
					}

					stepsLeft = 0;

					horizontal = block;
					// Go right until obstacle.
					while (!board.collision(horizontal)) {
						Block vertical = horizontal;
						int stepsDown = 1;
						vertical.moveDown();
						while (!board.collision(vertical)) {
							++stepsDown;
							vertical.moveDown();
						}
						states.emplace_back(stepsLeft, rotationLeft);

						--stepsLeft;
						horizontal.moveRight();
					}
				}
			}
			return states;
		}

		inline int factorial(int number) {
			int result = number;
			while (number > 1) {
				number -= 1;
				result *= number;
			}
			return result;
		}

	} // Anonymous namespace.

	RowRoughness calculateRowRoughness(const RawTetrisBoard& board, int highestUsedRow) {
		RowRoughness rowRoughness;
		int holes = 0;
		for (int row = 0; row < highestUsedRow; ++row) {
			bool lastHole = board.getBlockType(0, row) == BlockType::EMPTY;
			for (int column = 0; column < board.getColumns(); ++column) {
				bool hole = board.getBlockType(column, row) == BlockType::EMPTY;
				if (lastHole != hole) {
					rowRoughness.holes_ += 1;
					lastHole = hole;
				}
				if (!hole) {
					rowRoughness.rowSum_ += row;
					++holes;
				}
			}
		}
		return rowRoughness;
	}

	ColumnRoughness calculateColumnHoles(const RawTetrisBoard& board, int highestUsedRow) {
		ColumnRoughness roughness;
		int lastColumnNbr;
		for (int column = 0; column < board.getColumns(); ++column) {
			bool lastHole = board.getBlockType(column, 0) == BlockType::EMPTY;
			int columnNbr = lastHole ? 0 : 1;
			for (int row = 1; row < highestUsedRow; ++row) {
				bool hole = board.getBlockType(column, row) == BlockType::EMPTY;
				if (lastHole != hole) {
					roughness.holes_ += 1;
					lastHole = hole;
				}
				if (!hole) {
					++columnNbr;
				}
			}
			if (column != 0) {
				roughness.bumpiness += std::abs(lastColumnNbr - columnNbr);
			}
			lastColumnNbr = columnNbr;
		}
		return roughness;
	}

	// Calculation is performed before the block is part of the board.
	int calculateLandingHeight(const Block& block) { // f1	
		return block.getLowestRow();
	}

	// Calculate the cleared lines times the current blocks square contribution to the cleared lines. 
	// Calculation is perfomed before the block is part of the board.
	int calculateErodedPieces(const RawTetrisBoard& board) { // f2	
		int w = board.getColumns();
		int h = board.getRows();
		const Block& block = board.getBlock();

		int clearedLines = 0;
		int contribution = 0;
		for (int y = 0; y < 4; ++y) {
			int row = block.getLowestRow() + y;
			int filled = board.calculateSquaresFilled(row);
			int rowContribution = 0;
			for (const Square& sq : block) {
				if (sq.row_ == row) {
					++rowContribution;
				}
			}
			if (filled + rowContribution == w) {
				// Line is full.
				clearedLines += 1;
				contribution += rowContribution;
			}
		}
		return clearedLines * contribution;
	}

	// Calculate the number of filled cells adjacent to empty cells summed of all rows.
	int calculateRowTransitions(const RawTetrisBoard& board) { // f3
		const int w = board.getColumns();
		const int h = board.getRows();
		const size_t highestRow = calculateHighestUsedRow(board);

		int holes = 0;
		for (int y = 0; y <= highestRow; ++y) {
			bool lastFilled = true; // Left wall counts as filled.
			for (int x = 0; x < w; ++x) {
				bool filled = board.getBlockType(x, y) != BlockType::EMPTY;
				if (lastFilled != filled && filled) {
					++holes;
				}
				lastFilled = filled;
			}
			if (!lastFilled) {
				++holes;
			}
		}
		return holes;
	}

	// Calculate the number of filled cells adjacent to empty cells summed of all columns.
	int calculateColumnTransitions(const RawTetrisBoard& board) { // f4
		const int w = board.getColumns();
		const int h = board.getRows();
		const size_t highestRow = calculateHighestUsedRow(board) + 1; // One higher, should be a empty square.

		int holes = 0;
		for (int x = 0; x < w; ++x) {
			bool lastFilled = true; // Upper limit counts as filled.
			for (int y = 0; y <= highestRow; ++y) {
				bool filled = board.getBlockType(x, y) != BlockType::EMPTY;
				if (lastFilled != filled && filled) {
					++holes;
				}
				lastFilled = filled;
			}
			if (!lastFilled) {
				++holes;
			}
		}
		return holes;
	}

	// Calculate the number of holes, the number of empty cells with at least one filled cell above.
	int calculateNumberOfHoles(const RawTetrisBoard& board) { // f5
		int w = board.getColumns();
		int h = board.getRows();
		const size_t highestRow = calculateHighestUsedRow(board);

		int holes = 0;
		for (int x = 0; x < w; ++x) {
			bool foundFirstFilled = false; // Is always empty at the top. No hole.
			bool lastEmpty = false;
			for (int y = highestRow; y >= 0; --y) {
				bool empty = board.getBlockType(x, y) == BlockType::EMPTY;
				if (!empty) {
					foundFirstFilled = true;
				}
				if (empty && foundFirstFilled && !lastEmpty) {
					++holes;
				}
				lastEmpty = empty;
			}
		}
		return holes;
	}

	// Calculate the sum of the accumulated depths of the wells.
	int calculateCumulativeWells(const RawTetrisBoard& board) { // f6
		int w = board.getColumns();
		int h = board.getRows();
		const size_t highestRow = calculateHighestUsedRow(board);

		int cumulativeWells = 0;
		for (int x = 0; x < w; ++x) {
			int cumulative = 0;
			for (int y = highestRow; y >= 0; --y) {
				bool empty = board.getBlockType(x, y) == BlockType::EMPTY;
				bool neighborsFilled = board.getBlockType(x - 1, y) != BlockType::EMPTY && board.getBlockType(x + 1, y) != BlockType::EMPTY;
				if (!empty) {
					break;
				}
				if (neighborsFilled) {
					++cumulative;
				}
				if (!neighborsFilled && cumulative > 0) {
					break;
				}
			}
			cumulativeWells += factorial(cumulative);
		}
		return cumulativeWells;
	}

	// Calculate the number of filled cells above holes summed over all columns.
	int calculateHoleDepth(const RawTetrisBoard& board) { // f7
		int w = board.getColumns();
		int h = board.getRows();
		const size_t highestRow = calculateHighestUsedRow(board);

		int filled = 0;
		for (int x = 0; x < w; ++x) {
			bool foundEmpty = false;
			for (int y = 0; y <= highestRow; ++y) {
				bool empty = board.getBlockType(x, y) == BlockType::EMPTY;
				if (empty) {
					foundEmpty = true;
				}
				if (foundEmpty && !empty) {
					++filled;
				}
			}
		}
		return filled;
	}

	namespace {

		bool isAtLeastOneFilledSquareAbowe(int x, int y, const RawTetrisBoard& board, size_t highestRow) {
			for (++y; y <= highestRow; ++y) {
				if (board.getBlockType(x, y) != BlockType::EMPTY) {
					return true;
				}
			}
			return false;
		}

	}

	// Calculate the number of rows containing at least one hole.
	int calculateRowHoles(const RawTetrisBoard& board) { // f8
		int w = board.getColumns();
		int h = board.getRows();
		const size_t highestRow = calculateHighestUsedRow(board);

		int rows = 0;
		for (int y = 0; y <= highestRow; ++y) {
			for (int x = 0; x < w; ++x) {
				bool empty = board.getBlockType(x, y) == BlockType::EMPTY;
				if (empty && isAtLeastOneFilledSquareAbowe(x, y, board, highestRow)) {
					++rows;
					break;
				}
			}
		}
		return rows;
	}

	size_t calculateHighestUsedRow(const RawTetrisBoard& board) {
		auto v = board.getBoardVector();
		size_t index = 0;
		for (auto it = v.rbegin(); it != v.rend(); ++it) {
			if (*it != BlockType::EMPTY) {
				index = it - v.rbegin();
				break;
			}
		}
		return (v.size() - index - 1) / board.getColumns();
	}

	float calculateBlockMeanHeight(const Block& block) {
		int blockMeanHeight = 0;
		for (const Square& sq : block) {
			blockMeanHeight += sq.row_;
		}
		return (float) blockMeanHeight / block.getSize();
	}

	int calculateBlockEdges(const RawTetrisBoard& board, const Block& block) {
		int edges = 0;
		for (const Square& sq : block) {
			board.getBlockType(sq.column_ - 1, sq.row_) != BlockType::EMPTY ? ++edges : 0;
			board.getBlockType(sq.column_, sq.row_ - 1) != BlockType::EMPTY ? ++edges : 0;
			board.getBlockType(sq.column_ + 1, sq.row_) != BlockType::EMPTY ? ++edges : 0;
		}
		return edges;
	}
	Ai::State::State() : left_(0), rotationLeft_(0), value_(std::numeric_limits<float>::lowest()) {
	}

	Ai::State::State(int left, int rotations) : left_(left), rotationLeft_(rotations), value_(std::numeric_limits<float>::lowest()) {
	}

	Ai::Ai() : Ai("Default", Ai::getDefaultValueFunction()) {
	}

	Ai::Ai(std::string name, std::string valueFunction, bool allowException) : name_(name), valueFunction_(valueFunction) {
		initCalculator(allowException);
	}

	Ai::State Ai::calculateBestState(const RawTetrisBoard& board, int depth) {
		calculator_.updateVariable("rows", (float) board.getRows());
		calculator_.updateVariable("columns", (float) board.getColumns());
		if (depth >= 2) {
			return calculateBestStateRecursive(board, 2);
		} else if (depth == 1) {
			return calculateBestStateRecursive(board, 1);
		} else {
			return calculateBestStateRecursive(board, 0);
		}
	}

	// Find the best state for the block to move.
	Ai::State Ai::calculateBestStateRecursive(const RawTetrisBoard& board, int depth) {
		Ai::State bestState;

		if (depth > 0) {
			std::vector<Ai::State> states = calculateAllPossibleStates(board, board.getBlock());

			for (const Ai::State& state : states) {
				RawTetrisBoard childBoard = board;

				if (depth == 2) {
					moveBlockToBeforeImpact(state, childBoard);

					// Impact, the block is now a part of the board.
					childBoard.update(Move::DOWN_GRAVITY);

					State childState = calculateBestStateRecursive(childBoard, 1);

					if (childState.value_ > bestState.value_) {
						bestState = state;
						// Only updating the value from the child.
						bestState.value_ = childState.value_;
					}
				} else {
					float value = moveBlockToGroundCalculateValue(state, childBoard);
					if (value > bestState.value_) {
						bestState = state;
						bestState.value_ = value;
					}
				}
			}
		}
		return bestState;
	}

	void moveBlockToBeforeImpact(const Ai::State& state, RawTetrisBoard& board) {
		// Rotate.
		for (int i = 0; i < state.rotationLeft_; ++i) {
			board.update(Move::ROTATE_LEFT);
		}

		// Move left.
		for (int i = 0; i < state.left_; ++i) {
			board.update(Move::LEFT);
		}
		// Move right.
		for (int i = 0; i < -1 * state.left_; ++i) {
			board.update(Move::RIGHT);
		}

		// Move down the block and stop just before impact.
		board.update(Move::DOWN_GROUND);
	}

	float Ai::moveBlockToGroundCalculateValue(const State& state, RawTetrisBoard& board) {
		moveBlockToBeforeImpact(state, board);
		if (parameters_.landingHeight_) {
			calculator_.updateVariable("landingHeight", (float) calculateLandingHeight(board.getBlock()));
		}
		if (parameters_.erodedPieces_) {
			calculator_.updateVariable("erodedPieces", (float) calculateErodedPieces(board));
		}
		board.update(Move::DOWN_GRAVITY);
		if (parameters_.rowHoles_) {
			calculator_.updateVariable("rowHoles", (float) calculateRowTransitions(board));
		}
		if (parameters_.columnHoles_) {
			calculator_.updateVariable("columnHoles", (float) calculateColumnTransitions(board));
		}
		if (parameters_.holes_) {
			calculator_.updateVariable("holes", (float) calculateNumberOfHoles(board));
		}
		if (parameters_.cumulativeWells_) {
			calculator_.updateVariable("cumulativeWells", (float) calculateCumulativeWells(board));
		}
		if (parameters_.holeDepth_) {
			calculator_.updateVariable("holeDepth", (float) calculateHoleDepth(board));
		}
		return calculator_.excecute(cache_);
	}

	void Ai::initCalculator(bool allowException) {
		calculator_.addVariable("landingHeight", 0);
		calculator_.addVariable("erodedPieces", 0);
		calculator_.addVariable("rowHoles", 0);
		calculator_.addVariable("columnHoles", 0);
		calculator_.addVariable("holes", 0);
		calculator_.addVariable("cumulativeWells", 0);
		calculator_.addVariable("holeDepth", 0);

		calculator_.addVariable("rows", 0);
		calculator_.addVariable("columns", 0);

		if (allowException) {
			cache_ = calculator_.preCalculate(valueFunction_);
		} else {
			try {
				cache_ = calculator_.preCalculate(valueFunction_);
			} catch (calc::CalculatorException exception) {
				cache_ = calculator_.preCalculate("0");
			}
		}
		initAiParameters(calculator_, cache_);
	}

	void Ai::initAiParameters(const calc::Calculator& calculator, const calc::Cache& cache) {
		parameters_.landingHeight_ = calculator_.hasVariable("landingHeight", cache_);
		parameters_.erodedPieces_ = calculator_.hasVariable("erodedPieces", cache_);
		parameters_.rowHoles_ = calculator_.hasVariable("rowHoles", cache_);
		parameters_.columnHoles_ = calculator_.hasVariable("columnHoles", cache_);
		parameters_.holes_ = calculator_.hasVariable("holes", cache_);
		parameters_.cumulativeWells_ = calculator_.hasVariable("cumulativeWells", cache_);
		parameters_.holeDepth_ = calculator_.hasVariable("holeDepth", cache_);
	}

} // Namespace tetris.
