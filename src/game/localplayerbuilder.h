#ifndef MWETRIS_GAME_LOCALPLAYERBUILDER_H
#define MWETRIS_GAME_LOCALPLAYERBUILDER_H

#include "localplayer.h"
#include "tetrisboard.h"

namespace mwetris::game {

	class LocalPlayerBuilder {
	public:
		LocalPlayerPtr build();

		LocalPlayerBuilder withWidth(int width) {
			width_ = width;
			return *this;
		}

		LocalPlayerBuilder& withHeight(int height) {
			height_ = height;
			return *this;
		}

		LocalPlayerBuilder& withMovingBlock(const tetris::Block& block) {
			movingBlockType_ = tetris::BlockType::Empty;
			movingBlock_ = block;
			return *this;
		}

		LocalPlayerBuilder& withMovingBlockType(const tetris::BlockType& blockType) {
			movingBlockType_ = blockType;
			return *this;
		}

		LocalPlayerBuilder& withNextBlockType(tetris::BlockType next) {
			next_ = next;
			return *this;
		}

		LocalPlayerBuilder& withDevice(DevicePtr device) {
			device_ = device;
			return *this;
		}

		LocalPlayerBuilder& withPoints(int points) {
			points_ = points;
			return *this;
		}

		LocalPlayerBuilder& withLevel(int level) {
			level_ = level;
			return *this;
		}

		LocalPlayerBuilder& withLevelUpCounter(int counter) {
			levelUpCounter_ = counter;
			return *this;
		}

		LocalPlayerBuilder& withClearedRows(int rows) {
			clearedRows_ = rows;
			return *this;
		}

		LocalPlayerBuilder& withBoard(const std::vector<tetris::BlockType>& board) {
			board_ = board;
			return *this;
		}

		LocalPlayerBuilder& withGameOverPosition(int gameOverPosition) {
			gameOverPosition_ = gameOverPosition;
			return *this;
		}

		LocalPlayerBuilder& withName(const std::string& name) {
			name_ = name;
			return *this;
		}

	private:
		DevicePtr device_;
		std::string name_{"PLAYER"};
		int points_ = 0;
		int level_ = 1;
		int levelUpCounter_ = 0;
		int clearedRows_ = 0;
		int width_ = 10;
		int height_ = 24;
		tetris::Block movingBlock_;
		tetris::BlockType movingBlockType_ = tetris::BlockType::Empty;
		tetris::BlockType next_ = tetris::BlockType::Empty;
		std::vector<tetris::BlockType> board_;
		int gameOverPosition_ = 0;
	};

}

#endif
