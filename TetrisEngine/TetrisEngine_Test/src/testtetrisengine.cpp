// main() provided by Catch in file testmain.cpp.
#include <catch2/catch.hpp>

#include <ai.h>

using namespace tetris;

namespace {

	constexpr int TetrisWidth = 10;
	constexpr int TetrisHeight = 24;

}

bool blockEqual(const Block& block1, const Block& block2) {
	if (block1.getSize() != block2.getSize()) {
		return false;
	}
	for (int i = 0; i < block1.getSize(); ++i) {
		if (block1[i] != block2[i]) {
			return false;
		}
	}
	if (block1.getBlockType() != block2.getBlockType()) {
		return false;
	}
	if (block1.getNumberOfRotations() != block2.getNumberOfRotations()) {
		return false;
	}

	if (block1.getCurrentRotation() != block2.getCurrentRotation()) {
		return false;
	}

	if (block1.getStartColumn() != block2.getStartColumn()) {
		return false;
	}

	if (block1.getRotationSquare() != block2.getRotationSquare()) {
		return false;
	}
	return true;
}

TEST_CASE("Test Square", "[square]") {
	INFO("Testing square");

	const Square sq{1, 5};
	SECTION("square constuctor give the correct result") {
		REQUIRE((sq.column == 1 && sq.row == 5));
	}

	SECTION("square copy give the same result") {
		const Square sqCopy = sq;
		REQUIRE((sqCopy.column == sq.column && sqCopy.row == sq.row));
	}

	SECTION("testing equality") {
		SECTION("changing row") {
			Square sqCopy = sq;
			++sqCopy.row;
			REQUIRE(sqCopy != sq);
			REQUIRE(!(sqCopy == sq));
		}
		SECTION("changing column") {
			Square sqCopy = sq;
			++sqCopy.column;
			REQUIRE(sqCopy != sq);
			REQUIRE(!(sqCopy == sq));
		}
	}
}

void rotateBlock(Block& block, int rotation) {
	if (rotation > 0) {
		for (int i = 0; i < rotation; ++i) {
			block.rotateLeft();
		}
	} else {
		rotation = -1 * rotation;
		for (int i = 0; i < rotation; ++i) {
			block.rotateRight();
		}
	}
}

TEST_CASE("Test Block", "[block][square]") {
	INFO("Default tetrisboard");

	SECTION("Move block") {
		Block block{BlockType::J, 0, 0};

		SECTION("Move down") {
			block.moveDown();
			Block tmpBlock{BlockType::J, 0, -1};
			REQUIRE(blockEqual(tmpBlock, block));

			block.moveDown();
			block.moveDown();
			tmpBlock = Block{BlockType::J, 0, -3};
			REQUIRE(blockEqual(tmpBlock, block));
		}

		SECTION("Move left") {
			block.moveLeft();
			Block tmpBlock{BlockType::J, -1, 0};
			REQUIRE(blockEqual(tmpBlock, block));

			block.moveLeft();
			block.moveLeft();
			tmpBlock = Block{BlockType::J, -3, 0};
			REQUIRE(blockEqual(tmpBlock, block));
		}

		SECTION("Move right") {
			block.moveRight();
			Block tmpBlock{BlockType::J, 1, 0};
			REQUIRE(blockEqual(tmpBlock, block));

			block.moveRight();
			block.moveRight();
			tmpBlock = Block{BlockType::J, 3, 0};
			REQUIRE(blockEqual(tmpBlock, block));
		}
	}

	SECTION("Rotating block") {
		Block block{BlockType::J, 0, 0};
		REQUIRE(block.getNumberOfRotations() == 4);
		
		SECTION("Rotating clockwise in a full circle") {
			Block tmpBlock = block;
			block.rotateLeft();
			REQUIRE(!blockEqual(block, tmpBlock));
			block.rotateLeft();
			REQUIRE(!blockEqual(block, tmpBlock));
			block.rotateLeft();
			REQUIRE(!blockEqual(block, tmpBlock));
			block.rotateLeft();
			REQUIRE(blockEqual(block, tmpBlock));
		}

		SECTION("Rotating counter clockwise in a full circle") {
			Block tmpBlock = block;
			block.rotateRight();
			REQUIRE(!blockEqual(block, tmpBlock));
			block.rotateRight();
			REQUIRE(!blockEqual(block, tmpBlock));
			block.rotateRight();
			REQUIRE(!blockEqual(block, tmpBlock));
			block.rotateRight();
			REQUIRE(blockEqual(block, tmpBlock));
		}

		SECTION("Rotating in both directions") {
			const Block copyBlock = block;
			SECTION("Rotating one/three step") {
				Block rightBlock = block;
				rightBlock.rotateRight();
				rightBlock.rotateRight();
				rightBlock.rotateRight();
				block.rotateLeft(); // One step.
				REQUIRE(!blockEqual(block, copyBlock));
				REQUIRE(!blockEqual(rightBlock, copyBlock));
				REQUIRE(blockEqual(block, rightBlock));
			}

			SECTION("Rotating two/two steps") {
				Block rightBlock = block;
				rightBlock.rotateRight();
				rightBlock.rotateRight();
				block.rotateLeft(); // One step.
				block.rotateLeft(); // Two step.
				REQUIRE(!blockEqual(block, copyBlock));
				REQUIRE(!blockEqual(rightBlock, copyBlock));
				REQUIRE(blockEqual(block, rightBlock));
			}
		}
	}
}

TEST_CASE("Test tetrisboard", "[tetrisboard]") {
	INFO("Default tetrisboard");

	const BlockType firstNextBlockType = BlockType::L;
	const BlockType firstCurrentBlockType = BlockType::S;
	TetrisBoard board{TetrisWidth, TetrisHeight, firstCurrentBlockType, firstNextBlockType};

	int size = sizeof(TetrisBoard);

	SECTION("should not be game over") {
		REQUIRE(!board.isGameOver());
	}

	SECTION("testing the size of the board") {
		REQUIRE(board.getColumns() == TetrisWidth);
		REQUIRE(board.getRows() == TetrisHeight);
	}

	SECTION("update the next block") {
		REQUIRE(board.getNextBlockType() == firstNextBlockType);
		REQUIRE(board.getBlockType() == firstCurrentBlockType);

		const BlockType newBlockType = BlockType::I;
		REQUIRE(newBlockType != board.getNextBlockType());
		board.setNextBlock(newBlockType);
		REQUIRE(newBlockType == board.getNextBlockType());
	}

	SECTION("update the current block") {
		REQUIRE(board.getNextBlockType() == firstNextBlockType);
		REQUIRE(board.getBlockType() == firstCurrentBlockType);
		REQUIRE(firstNextBlockType != firstCurrentBlockType);

		SECTION("current block becomes the nextblock") {
			Block startBlock = board.getBlock();
			REQUIRE(blockEqual(startBlock, board.getBlock()));
			board.update(Move::DownGround);
			REQUIRE(!blockEqual(startBlock, board.getBlock()));
			board.update(Move::DownGravity);
			REQUIRE(board.getBlockType() == firstNextBlockType);
		}
	}


	SECTION("restart the board") {
		const int NEW_WIDTH = 15;
		const int NEW_HEIGHT = 30;
		const BlockType NEW_CURRENT = BlockType::I;
		const BlockType NEW_NEXT = BlockType::Z;
		TetrisBoard restartBoard{NEW_WIDTH, NEW_HEIGHT, NEW_CURRENT, NEW_NEXT};

		// Different size compare to board.
		REQUIRE(board.getBlockType() != restartBoard.getBlockType());
		REQUIRE(board.getNextBlockType() != restartBoard.getNextBlockType());
		REQUIRE(board.getColumns() != restartBoard.getColumns());
		REQUIRE(board.getRows() != restartBoard.getRows());

		SECTION("restart blocktypes") {
			// Restart.
			board.restart(NEW_CURRENT, NEW_NEXT);
			// New types.
			REQUIRE(board.getBlockType() == NEW_CURRENT);
			REQUIRE(board.getNextBlockType() == NEW_NEXT);
			// Same size.
			REQUIRE(board.getColumns() == TetrisWidth);
			REQUIRE(board.getRows() == TetrisHeight);
		}
		
		SECTION("restart and resize") {
			// Restart.
			board.update(Move::Left);
			board.update(Move::DownGround);
			board.restart(NEW_WIDTH, NEW_HEIGHT, NEW_CURRENT, NEW_NEXT);
			// New types.
			REQUIRE(board.getBlockType() == restartBoard.getBlockType());
			REQUIRE(board.getNextBlockType() == restartBoard.getNextBlockType());
			// Same block.
			REQUIRE(blockEqual(board.getBlock(), restartBoard.getBlock()));
			// New size.
			REQUIRE(board.getColumns() == restartBoard.getColumns());
			REQUIRE(board.getRows() == restartBoard.getRows());
		}
	}
}

TEST_CASE("Test ai", "[ai]") {
	INFO("Default tetrisboard");
	
	const BlockType NEW_NEXT = BlockType::Z;

	Block current{BlockType::I, 4, 20};
	
	int intBoard[] = {
		0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
		0, 0, 0, 0, 0, 0, 0, 1, 0, 1,
		0, 0, 1, 1, 0, 0, 1, 1, 1, 1,
		1, 1, 1, 1, 0, 1, 1, 1, 1, 1,
		0, 1, 1, 1, 0, 0, 1, 1, 0, 1,
		1, 1, 1, 1, 0, 1, 1, 1, 1, 1,
		0, 1, 1, 1, 0, 1, 1, 1, 1, 1,
		0, 1, 1, 1, 1, 1, 0, 1, 1, 1,
		1, 1, 0, 1, 1, 0, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1, 1, 0, 1, 0,
		0, 1, 1, 0, 1, 0, 1, 0, 1, 1,
		0, 1, 1, 0, 1, 1, 1, 1, 0, 1,
		1, 1, 1, 1, 0, 1, 1, 1, 0, 0,
		1, 0, 1, 1, 0, 1, 1, 1, 1, 1,
		0, 1, 1, 1, 1, 1, 1, 0, 0, 0};
	std::vector<BlockType> boardBlockTypes;

	// Take the lowest row first, and so forth.
	for (int y = 0; y < 16; ++y) {
		for (int x = 0; x < 10; ++x) {
			int index = x + (15 - y) * 10;
			if (intBoard[index] == 0) {
				boardBlockTypes.push_back(BlockType::Empty);
			} else {
				boardBlockTypes.push_back(BlockType::I);
			}
		}
	}

	TetrisBoard tetrisBoard{boardBlockTypes, TetrisWidth, TetrisHeight, current, NEW_NEXT};
	tetrisBoard.update(Move::DownGround);

	SECTION("Calcualte highest used row") {
		REQUIRE(calculateHighestUsedRow(tetrisBoard) == 15);
		tetrisBoard.update(Move::DownGravity); // Two rows removed.
		REQUIRE(calculateHighestUsedRow(tetrisBoard) == 13);
	}

	SECTION("Calculate Landing height, f1") {
		REQUIRE(calculateLandingHeight(tetrisBoard.getBlock()) == 8);
	}

	SECTION("Calculate eroded pieces, f2") {
		REQUIRE(calculateErodedPieces(tetrisBoard) == 2 * 2);
	}	

	SECTION("Calculate row holes, f3") {
		tetrisBoard.update(Move::DownGravity);
		REQUIRE(calculateRowTransitions(tetrisBoard) == 29);
	}
	
	SECTION("Calculate column holes, f4") {
		tetrisBoard.update(Move::DownGravity);
		REQUIRE(calculateColumnTransitions(tetrisBoard) == 27);
	}

	SECTION("Calculate number of holes, f5") {
		tetrisBoard.update(Move::DownGravity);
		REQUIRE(calculateNumberOfHoles(tetrisBoard) == 17);
	}

	SECTION("Calculate sum of cumulative wells, f6") {
		tetrisBoard.update(Move::DownGravity);
		REQUIRE(calculateCumulativeWells(tetrisBoard) == 8);
	}

	SECTION("Calculate sum of hole depths, f7") {
		tetrisBoard.update(Move::DownGravity);
		REQUIRE(calculateHoleDepth(tetrisBoard) == 62);
	}

	SECTION("Calculate the number of rows containing at least one hole, f8") {
		tetrisBoard.update(Move::DownGravity);
		REQUIRE(calculateRowHoles(tetrisBoard) == 9);
	}
}
