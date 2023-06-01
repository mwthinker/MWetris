#include <gtest/gtest.h>

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

class TetrisTest : public ::testing::Test {
protected:

	TetrisTest() {

	}

	~TetrisTest() override {}

	void SetUp() override {}

	void TearDown() override {}
};

TEST_F(TetrisTest, squareConstructorGiveCorrectResult) {
	const Square sq{1, 5};

	EXPECT_EQ(1, sq.column);
	EXPECT_EQ(5, sq.row);
}

TEST_F(TetrisTest, squareCopy) {
	const Square sq{1, 5};
	const Square sqCopy = sq;
	EXPECT_EQ(sq.column, sqCopy.column);
	EXPECT_EQ(sq.row, sqCopy.row);
}

TEST_F(TetrisTest, squareEquality) {
	const Square sq{1, 5};
	Square sqCopy = sq;
	++sqCopy.row;

	EXPECT_TRUE(sqCopy != sq);
	EXPECT_TRUE(!(sqCopy == sq));

	sqCopy = sq;
	++sqCopy.column;

	EXPECT_TRUE(sqCopy != sq);
	EXPECT_TRUE(!(sqCopy == sq));
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

TEST_F(TetrisTest, moveBlockDown) {
	Block block{BlockType::J, 0, 0};

	block.moveDown();
	Block tmpBlock{BlockType::J, 0, -1};
	EXPECT_TRUE(blockEqual(tmpBlock, block));

	block.moveDown();
	block.moveDown();
	tmpBlock = Block{BlockType::J, 0, -3};
	EXPECT_TRUE(blockEqual(tmpBlock, block));
}

TEST_F(TetrisTest, moveBlockLeft) {
	Block block{BlockType::J, 0, 0};

	block.moveLeft();
	Block tmpBlock{BlockType::J, -1, 0};
	EXPECT_TRUE(blockEqual(tmpBlock, block));

	block.moveLeft();
	block.moveLeft();
	tmpBlock = Block{BlockType::J, -3, 0};
	EXPECT_TRUE(blockEqual(tmpBlock, block));
}

TEST_F(TetrisTest, moveBlockRight) {
	Block block{BlockType::J, 0, 0};

	block.moveRight();
	Block tmpBlock{BlockType::J, 1, 0};
	EXPECT_TRUE(blockEqual(tmpBlock, block));;

	block.moveRight();
	block.moveRight();
	tmpBlock = Block{BlockType::J, 3, 0};
	EXPECT_TRUE(blockEqual(tmpBlock, block));;
}

TEST_F(TetrisTest, rotateBlockClockwise) {
	Block block{BlockType::J, 0, 0};
	EXPECT_EQ(4, block.getNumberOfRotations());

	Block tmpBlock = block;
	block.rotateLeft();
	EXPECT_TRUE(!blockEqual(block, tmpBlock));
	block.rotateLeft();
	EXPECT_TRUE(!blockEqual(block, tmpBlock));
	block.rotateLeft();
	EXPECT_TRUE(!blockEqual(block, tmpBlock));
	block.rotateLeft();
	EXPECT_TRUE(blockEqual(block, tmpBlock));
}

TEST_F(TetrisTest, rotateBlockCounterClockwise) {
	Block block{BlockType::J, 0, 0};
	EXPECT_EQ(4, block.getNumberOfRotations());

	Block tmpBlock = block;
	block.rotateRight();
	EXPECT_TRUE(!blockEqual(block, tmpBlock));
	block.rotateRight();
	EXPECT_TRUE(!blockEqual(block, tmpBlock));
	block.rotateRight();
	EXPECT_TRUE(!blockEqual(block, tmpBlock));
	block.rotateRight();
	EXPECT_TRUE(blockEqual(block, tmpBlock));
}

TEST_F(TetrisTest, rotateBlockInBothDirection1_3) {
	Block block{BlockType::J, 0, 0};
	EXPECT_EQ(4, block.getNumberOfRotations());

	const Block copyBlock = block;
	Block rightBlock = block;
	rightBlock.rotateRight();
	rightBlock.rotateRight();
	rightBlock.rotateRight();
	block.rotateLeft(); // One step.
	EXPECT_TRUE(!blockEqual(block, copyBlock));
	EXPECT_TRUE(!blockEqual(rightBlock, copyBlock));
	EXPECT_TRUE(blockEqual(block, rightBlock));
}

TEST_F(TetrisTest, rotateBlockInBothDirection2_2) {
	Block block{BlockType::J, 0, 0};
	EXPECT_EQ(4, block.getNumberOfRotations());

	const Block copyBlock = block;
	Block rightBlock = block;
	rightBlock.rotateRight();
	rightBlock.rotateRight();
	block.rotateLeft(); // One step.
	block.rotateLeft(); // Two step.
	EXPECT_TRUE(!blockEqual(block, copyBlock));
	EXPECT_TRUE(!blockEqual(rightBlock, copyBlock));
	EXPECT_TRUE(blockEqual(block, rightBlock));
}

TEST_F(TetrisTest, boardIsGameOver) {
	const BlockType firstNextBlockType = BlockType::L;
	const BlockType firstCurrentBlockType = BlockType::S;
	TetrisBoard board{TetrisWidth, TetrisHeight, firstCurrentBlockType, firstNextBlockType};

	EXPECT_FALSE(board.isGameOver());
	EXPECT_EQ(TetrisWidth, board.getColumns());
	EXPECT_EQ(TetrisHeight, board.getRows());
	//EXPECT_EQ(firstCurrentBlockType, board.getBlockType());
	EXPECT_EQ(firstNextBlockType, board.getNextBlockType());

	const BlockType newBlockType = BlockType::I;
	EXPECT_TRUE(newBlockType != board.getNextBlockType());
	board.setNextBlock(newBlockType);
	EXPECT_EQ(newBlockType, board.getNextBlockType());
}

/*
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
*/