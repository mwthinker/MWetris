#ifndef MWETRIS_GAME_LOCALPLAYERBOARDBUILDER_H
#define MWETRIS_GAME_LOCALPLAYERBOARDBUILDER_H

#include "localplayerboard.h"
#include "block.h"

#include <vector>
#include <string>

namespace mwetris::game {

	class LocalPlayerBoardBuilder {
	public:
		LocalPlayerBoardPtr build();

		LocalPlayerBoardBuilder& withWidth(int width) {
			width_ = width;
			return *this;
		}

		LocalPlayerBoardBuilder& withHeight(int height) {
			height_ = height;
			return *this;
		}

		LocalPlayerBoardBuilder& withMovingBlock(const tetris::Block& block) {
			movingBlockType_ = tetris::BlockType::Empty;
			movingBlock_ = block;
			return *this;
		}

		LocalPlayerBoardBuilder& withMovingBlockType(const tetris::BlockType& blockType) {
			movingBlockType_ = blockType;
			return *this;
		}

		LocalPlayerBoardBuilder& withNextBlockType(tetris::BlockType next) {
			next_ = next;
			return *this;
		}

		LocalPlayerBoardBuilder& withPlayerData(const PlayerData& playerData) {
			playerData_ = playerData;
			return *this;
		}

		LocalPlayerBoardBuilder& withClearedRows(int rows) {
			clearedRows_ = rows;
			return *this;
		}

		LocalPlayerBoardBuilder& withBoard(const std::vector<tetris::BlockType>& board) {
			board_ = board;
			return *this;
		}

		LocalPlayerBoardBuilder& withGameOverPosition(int gameOverPosition) {
			gameOverPosition_ = gameOverPosition;
			return *this;
		}

		LocalPlayerBoardBuilder& withName(const std::string& name) {
			name_ = name;
			return *this;
		}

	private:
		std::string name_{"PLAYER"};
		int clearedRows_ = 0;
		int width_ = 10;
		int height_ = 24;
		tetris::Block movingBlock_;
		tetris::BlockType movingBlockType_ = tetris::BlockType::Empty;
		tetris::BlockType next_ = tetris::BlockType::Empty;
		std::vector<tetris::BlockType> board_;
		int gameOverPosition_ = 0;
		PlayerData playerData_{};
	};

}

#endif
