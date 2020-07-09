#include "ai.h"

#include <calc/cache.h>
#include <calc/calculatorexception.h>

#include <limits>
#include <cmath>
#include <iostream>

namespace tetris {

	namespace {

		// Calculate and return all possible states for the block provided.
		std::vector<Ai::State> calculateAllPossibleStates(const TetrisBoard& board, Block block) {
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

		constexpr const char* getDefaultValueFunction() {
			return "-0.2*cumulativeWells - 1*holeDepth - 1*holes - 1*landingHeight";
		}

	}

	RowRoughness calculateRowRoughness(const TetrisBoard& board, int highestUsedRow) {
		RowRoughness rowRoughness{};
		int holes = 0;
		for (int row = 0; row < highestUsedRow; ++row) {
			bool lastHole = board.getBlockType(0, row) == BlockType::Empty;
			for (int column = 0; column < board.getColumns(); ++column) {
				bool hole = board.getBlockType(column, row) == BlockType::Empty;
				if (lastHole != hole) {
					rowRoughness.holes += 1;
					lastHole = hole;
				}
				if (!hole) {
					rowRoughness.rowSum += row;
					++holes;
				}
			}
		}
		return rowRoughness;
	}

	ColumnRoughness calculateColumnHoles(const TetrisBoard& board, int highestUsedRow) {
		ColumnRoughness roughness{};
		int lastColumnNbr;
		for (int column = 0; column < board.getColumns(); ++column) {
			bool lastHole = board.getBlockType(column, 0) == BlockType::Empty;
			int columnNbr = lastHole ? 0 : 1;
			for (int row = 1; row < highestUsedRow; ++row) {
				bool hole = board.getBlockType(column, row) == BlockType::Empty;
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
	int calculateErodedPieces(const TetrisBoard& board) { // f2	
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
				if (sq.row == row) {
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
	int calculateRowTransitions(const TetrisBoard& board) { // f3
		const int w = board.getColumns();
		const int h = board.getRows();
		const auto highestRow = calculateHighestUsedRow(board);

		int holes = 0;
		for (int y = 0; y <= highestRow; ++y) {
			bool lastFilled = true; // Left wall counts as filled.
			for (int x = 0; x < w; ++x) {
				bool filled = board.getBlockType(x, y) != BlockType::Empty;
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
	int calculateColumnTransitions(const TetrisBoard& board) { // f4
		const int w = board.getColumns();
		const int h = board.getRows();
		const auto highestRow = calculateHighestUsedRow(board) + 1; // One higher, should be a empty square.

		int holes = 0;
		for (int x = 0; x < w; ++x) {
			bool lastFilled = true; // Upper limit counts as filled.
			for (int y = 0; y <= highestRow; ++y) {
				bool filled = board.getBlockType(x, y) != BlockType::Empty;
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
	int calculateNumberOfHoles(const TetrisBoard& board) { // f5
		const auto w = board.getColumns();
		const auto h = board.getRows();
		const auto highestRow = calculateHighestUsedRow(board);

		int holes = 0;
		for (int x = 0; x < w; ++x) {
			bool foundFirstFilled = false; // Is always empty at the top. No hole.
			bool lastEmpty = false;
			for (int y = highestRow; y >= 0; --y) {
				bool empty = board.getBlockType(x, y) == BlockType::Empty;
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
	int calculateCumulativeWells(const TetrisBoard& board) { // f6
		auto w = board.getColumns();
		auto h = board.getRows();
		const auto highestRow = calculateHighestUsedRow(board);

		int cumulativeWells = 0;
		for (int x = 0; x < w; ++x) {
			int cumulative = 0;
			for (int y = highestRow; y >= 0; --y) {
				bool empty = board.getBlockType(x, y) == BlockType::Empty;
				bool neighborsFilled = board.getBlockType(x - 1, y) != BlockType::Empty && board.getBlockType(x + 1, y) != BlockType::Empty;
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
	int calculateHoleDepth(const TetrisBoard& board) { // f7
		int w = board.getColumns();
		int h = board.getRows();
		const auto highestRow = calculateHighestUsedRow(board);

		int filled = 0;
		for (int x = 0; x < w; ++x) {
			bool foundEmpty = false;
			for (int y = 0; y <= highestRow; ++y) {
				bool empty = board.getBlockType(x, y) == BlockType::Empty;
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

		bool isAtLeastOneFilledSquareAbowe(int x, int y, const TetrisBoard& board, size_t highestRow) {
			for (++y; y <= highestRow; ++y) {
				if (board.getBlockType(x, y) != BlockType::Empty) {
					return true;
				}
			}
			return false;
		}

	}

	// Calculate the number of rows containing at least one hole.
	int calculateRowHoles(const TetrisBoard& board) { // f8
		int w = board.getColumns();
		int h = board.getRows();
		const auto highestRow = calculateHighestUsedRow(board);

		int rows = 0;
		for (int y = 0; y <= highestRow; ++y) {
			for (int x = 0; x < w; ++x) {
				bool empty = board.getBlockType(x, y) == BlockType::Empty;
				if (empty && isAtLeastOneFilledSquareAbowe(x, y, board, highestRow)) {
					++rows;
					break;
				}
			}
		}
		return rows;
	}

	int calculateHighestUsedRow(const TetrisBoard& board) {
		const auto& v = board.getBoardVector();
		auto it = std::find_if(v.rbegin(), v.rend(), [](BlockType blockType) {
			return BlockType::Empty != blockType;
		});
		auto index = it - v.rbegin();
		return static_cast<int>((v.size() - index - 1) / board.getColumns());
	}

	float calculateBlockMeanHeight(const Block& block) {
		int blockMeanHeight = 0;
		for (const Square& sq : block) {
			blockMeanHeight += sq.row;
		}
		return (float) blockMeanHeight / block.getSize();
	}

	int calculateBlockEdges(const TetrisBoard& board, const Block& block) {
		int edges = 0;
		for (const Square& sq : block) {
			board.getBlockType(sq.column - 1, sq.row) != BlockType::Empty ? ++edges : 0;
			board.getBlockType(sq.column, sq.row - 1) != BlockType::Empty ? ++edges : 0;
			board.getBlockType(sq.column + 1, sq.row) != BlockType::Empty ? ++edges : 0;
		}
		return edges;
	}
	
	Ai::State::State(int left, int rotations)
		: left{left}
		, rotationLeft{rotations} {
	}

	Ai::Ai()
		: Ai{"Default", getDefaultValueFunction()} {
	}

	Ai::Ai(std::string name, std::string valueFunction, bool allowException)
		: name_{name}
		, valueFunction_{valueFunction} {
		
		initCalculator(allowException);
	}

	Ai::State Ai::calculateBestState(const TetrisBoard& board, int depth) {
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

	Ai::State Ai::calculateBestStateRecursive(const TetrisBoard& board, int depth) {
		Ai::State bestState;

		if (depth > 0) {
			std::vector<Ai::State> states = calculateAllPossibleStates(board, board.getBlock());

			for (const Ai::State& state : states) {
				TetrisBoard childBoard = board;

				if (depth == 2) {
					moveBlockToBeforeImpact(state, childBoard);

					// Impact, the block is now a part of the board.
					childBoard.update(Move::DownGravity);

					State childState = calculateBestStateRecursive(childBoard, 1);

					if (childState.value > bestState.value) {
						bestState = state;
						// Only updating the value from the child.
						bestState.value = childState.value;
					}
				} else {
					float value = moveBlockToGroundCalculateValue(state, childBoard);
					if (value > bestState.value) {
						bestState = state;
						bestState.value = value;
					}
				}
			}
		}
		return bestState;
	}

	float Ai::moveBlockToGroundCalculateValue(const State& state, TetrisBoard& board) {
		moveBlockToBeforeImpact(state, board);
		if (parameters_.landingHeight) {
			calculator_.updateVariable("landingHeight", (float) calculateLandingHeight(board.getBlock()));
		}
		if (parameters_.erodedPieces) {
			calculator_.updateVariable("erodedPieces", (float) calculateErodedPieces(board));
		}
		board.update(Move::DownGround);
		if (parameters_.rowHoles) {
			calculator_.updateVariable("rowHoles", (float) calculateRowTransitions(board));
		}
		if (parameters_.columnHoles) {
			calculator_.updateVariable("columnHoles", (float) calculateColumnTransitions(board));
		}
		if (parameters_.holes) {
			calculator_.updateVariable("holes", (float) calculateNumberOfHoles(board));
		}
		if (parameters_.cumulativeWells) {
			calculator_.updateVariable("cumulativeWells", (float) calculateCumulativeWells(board));
		}
		if (parameters_.holeDepth) {
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
		parameters_.landingHeight = calculator_.hasVariable("landingHeight", cache_);
		parameters_.erodedPieces = calculator_.hasVariable("erodedPieces", cache_);
		parameters_.rowHoles = calculator_.hasVariable("rowHoles", cache_);
		parameters_.columnHoles = calculator_.hasVariable("columnHoles", cache_);
		parameters_.holes = calculator_.hasVariable("holes", cache_);
		parameters_.cumulativeWells = calculator_.hasVariable("cumulativeWells", cache_);
		parameters_.holeDepth = calculator_.hasVariable("holeDepth", cache_);
	}

}
