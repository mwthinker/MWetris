#include "flags.h"
#include "flagsexception.h"

#include <tetrisboard.h>
#include <helper.h>
#include <ai.h>
#include <calc/calculatorexception.h>

#include <fstream>
#include <sstream>
#include <thread>
#include <limits>
#include <queue>
#include <iostream>
#include <thread>
#include <chrono>
#include <iomanip>

using namespace std::chrono_literals;
using namespace tetris;

BlockType badRandomBlockType() {
	static auto blockType = BlockType::Z;
	if (blockType == BlockType::Z) {
		blockType = BlockType::S;
	} else {
		blockType = BlockType::Z;
	}
	return blockType;
}

BlockType readBlockType(std::ifstream& infile) {
	int data = -1;
	if (infile.is_open()) {
		if (!(infile >> data)) {
			// Restart, read from the start again.
			infile.clear();
			infile.seekg(0, std::ios::beg);
			infile >> data;
		}
	}
	if (data >= 0 && data <= 6) {
		return static_cast<BlockType>(data);
	}
	// In order for the ai to fail.
	return badRandomBlockType();
}

struct Tetris {
	Tetris(TetrisBoard& tetrisBoard, const Flags& flags, std::ifstream& infile)
		: tetrisBoard{tetrisBoard}
		, flags{flags}
		, infile{infile} {
	}
	
	void operator()(BoardEvent gameEvent, int value) {
		switch (gameEvent) {
			case tetris::BoardEvent::BlockCollision:
				if (flags.useRandomFile_) {
					try {
						BlockType blockType = readBlockType(infile);
						tetrisBoard.setNextBlock(blockType);
					} catch (const std::ifstream::failure& e) {
						std::cerr << "Failed to read " << flags.randomFilePath_ << "\n";
						if (flags.verbose_) {
							std::cerr << e.what() << "\n";
						}
						std::exit(1);
					}
				} else {
					tetrisBoard.setNextBlock(randomBlockType());
				}
				break;
			case tetris::BoardEvent::CurrentBlockUpdated:
				++turns;
				break;
			case tetris::BoardEvent::PlayerMovesBlock:
				break;
			case tetris::BoardEvent::GravityMovesBlock:
				break;
			case tetris::BoardEvent::RowsRemoved:
				switch (value) {
					case 1:
						++clearedOneRows;
						break;
					case 2:
						++clearedTwoRows;
						break;
					case 3:
						++clearedThreeRows;
						break;
					case 4:
						++clearedFourRows;
						break;
				}
				break;
			case tetris::BoardEvent::RowToBeRemoved:
				break;
			case tetris::BoardEvent::GameOver:
				break;
			default:
				break;
		}
	}

	TetrisBoard& tetrisBoard;
	Flags flags;
	std::ifstream& infile;
	int turns{};
	int clearedOneRows{};
	int clearedTwoRows{};
	int clearedThreeRows{};
	int clearedFourRows{};
};

void printBoard(const TetrisBoard& board) {
	const std::vector<BlockType>& squares = board.getBoardVector();
	int rows = board.getRows();
	int columns = board.getColumns();
	Block block = board.getBlock();
	std::cout << "\n|";
	for (int row = rows - 3; row > -1; --row) {
		for (int column = 0; column < columns; ++column) {
			if (column == 0 && row < rows - 3) {
				// Add new row.
				std::cout << "  " << row + 2 << "\n|";
			}

			bool newSquare = false;
			for (const auto& sq : block) {
				if (sq.column == column && sq.row == row) {
					std::cout << "X|";
					newSquare = true;
					break;
				}
			}
			if (newSquare) {
				continue;
			}

			if (BlockType::Empty == squares[row * columns + column]) {
				std::cout << " |";
			} else {
				std::cout << "X|";
			}
		}
	}

	std::cout << "  1\n";
	for (int i = 0; i < columns; ++i) {
		std::cout << "--";
	}
	std::cout << "-\n\n";
}

void printGameResult(const Tetris& tetris, const Flags& flags, const std::chrono::duration<double>& gameTime) {
	std::queue<std::string> outputOrder = flags.outputOrder_;

	std::cout << std::setprecision(2) << std::fixed;
	if (flags.play_) {
		printBoard(tetris.tetrisBoard);
		std::cout << "AI: " << flags.ai_.getValueFunction() << "\n";
		std::cout << "Turns: " << tetris.turns << "\n";
		std::cout << "Time in seconds: " << gameTime.count() << "\n";
	}

	while (!outputOrder.empty()) {
		const auto& flag = outputOrder.front();
		if (flag == "-T") {
			if (flags.verbose_) {
				std::cout << "time = ";
			}
			std::cout << gameTime.count();
			if (flags.verbose_) {
				std::cout << "s";
			}
			std::cout << "\t";
		} else if (flag == "-t") {
			if (flags.verbose_) {
				std::cout << "turns = ";
			}
			std::cout << tetris.turns << "\t";
		} else if (flag == "-c") {
			if (flags.verbose_) {
				std::cout << "cleared-rows = ";
			}
			std::cout << tetris.clearedOneRows + tetris.clearedTwoRows * 2 + tetris.clearedThreeRows * 3 + tetris.clearedFourRows * 4 << "\t";
		} else if (flag == "-c1") {
			if (flags.verbose_) {
				std::cout << "cleared-rows-1 = ";
			}
			std::cout << tetris.clearedOneRows << "\t";
		} else if (flag == "-c2") {
			if (flags.verbose_) {
				std::cout << "cleared-rows-2 = ";
			}
			std::cout << tetris.clearedTwoRows << "\t";
		} else if (flag == "-c3") {
			if (flags.verbose_) {
				std::cout << "cleared-rows-3 = ";
			}
			std::cout << tetris.clearedThreeRows << "\t";
		} else if (flag == "-c4") {
			if (flags.verbose_) {
				std::cout << "cleared-rows-4 = ";
			}
			std::cout << tetris.clearedFourRows << "\t";
		}
		outputOrder.pop();
	}

	std::cout << "\n";
}

void runGame(Tetris& tetris) {
	auto time = std::chrono::high_resolution_clock::now();

	auto& flags = tetris.flags;
	auto& tetrisBoard = tetris.tetrisBoard;

	Ai ai = flags.ai_;
	TetrisBoard playBoard{tetrisBoard};
	while (!tetrisBoard.isGameOver() && tetris.turns < flags.maxNbrBlocks_) {
		auto state = ai.calculateBestState(tetrisBoard, flags.depth_);

		if (flags.play_) {
			std::cout << "AI: " << ai.getValueFunction() << "\n";
			std::cout << "Turns: " << tetris.turns << "\n";
			playBoard = tetrisBoard;
		}

		auto tmp = tetrisBoard;
		auto value = ai.moveBlockToGroundCalculateValue(state, tmp);
		moveBlockToBeforeImpact(state, tetrisBoard);

		if (flags.play_) {
			std::cout << "Value: " << value << "\n";
			const auto& variables = ai.getCalculator().getVariables();
			for (const auto& name : variables) {
				std::cout << name << " = " << ai.getCalculator().extractVariableValue(name) << "\t";
			}
			printBoard(playBoard);
		}
		
		tetrisBoard.update(Move::DownGravity, tetris);
		if (flags.delay_ > 1ms) {
			std::this_thread::sleep_for(flags.delay_);
		}
	}

	printGameResult(tetris, flags, std::chrono::high_resolution_clock::now() - time);
}

int main(const int argc, const char* const argv[]) {
	Flags flags;
	try {
		flags = Flags{argc, argv};
	} catch (const FlagsException& e) {
		std::cerr << e.what();
		return 1;
	}

	if (flags.printHelp_) {
		flags.printHelpFunction();
		return 0;
	}

	auto start = randomBlockType();
	auto next = randomBlockType();

	std::ifstream infile;
	infile.exceptions(std::ifstream::badbit| std::ifstream::failbit);

	if (flags.useRandomFile_) {
		try {
			infile.open(flags.randomFilePath_);
		} catch (const std::ifstream::failure& e) {
			std::cerr << "Failed to open file: " + flags.randomFilePath_;
			if (flags.verbose_) {
				std::cerr << "\n" << e.what() << "\n";
			}
			std::exit(1);
		}
		start = readBlockType(infile);
		next = readBlockType(infile);
	}

	TetrisBoard tetrisBoard{flags.width_, flags.height_, start, next};

	Tetris tetris{tetrisBoard, flags, infile};

	runGame(tetris);
	
	return 0;
}
