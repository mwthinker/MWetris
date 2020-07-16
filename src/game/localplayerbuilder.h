#ifndef MWETRIS_GAME_LOCALPLAYERBUILDER_H
#define MWETRIS_GAME_LOCALPLAYERBUILDER_H

#include "localplayer.h"
#include "tetrisboardwrapper.h"
#include "eventmanager.h"

namespace tetris::game {

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

		LocalPlayerBuilder& widthMovingBlock(const Block& block) {
			movingBlockType_ = BlockType::Empty;
			movingBlock_ = block;
			return *this;
		}

		LocalPlayerBuilder& widthMovingBlockType(const BlockType& blockType) {
			movingBlockType_ = blockType;
			return *this;
		}

		LocalPlayerBuilder& widthNextBlockType(BlockType next) {
			next_ = next;
			return *this;
		}

		LocalPlayerBuilder& widthDevice(DevicePtr device) {
			device_ = device;
			return *this;
		}

		LocalPlayerBuilder& widthPoints(int points) {
			points_ = points;
			return *this;
		}

		LocalPlayerBuilder& widthLevel(int level) {
			level_ = level;
			return *this;
		}

		LocalPlayerBuilder& widthLevelUpCounter(int counter) {
			levelUpCounter_ = counter;
			return *this;
		}

		LocalPlayerBuilder& widthClearedRows(int rows) {
			clearedRows_ = rows;
			return *this;
		}

		LocalPlayerBuilder& widthBoard(const std::vector<BlockType>& board) {
			board_ = board;
			return *this;
		}

		LocalPlayerBuilder& widthGameOverPosition(int gameOverPosition) {
			gameOverPosition_ = gameOverPosition;
			return *this;
		}

		LocalPlayerBuilder& widthName(const std::string& name) {
			name_ = name;
			return *this;
		}
		
		
		LocalPlayerBuilder& widthEventManager(std::shared_ptr<EventManager> eventManager) {
			eventManager_ = std::move(eventManager);
			return *this;
		}

	private:
		std::shared_ptr<EventManager> eventManager_;
		DevicePtr device_;
		std::string name_{"PLAYER"};
		int points_{0}, level_{1}, levelUpCounter_{0}, clearedRows_{0};
		int width_{10}, height_{24};
		Block movingBlock_;
		BlockType movingBlockType_{BlockType::Empty};
		BlockType next_{BlockType::Empty};
		std::vector<BlockType> board_;
		int gameOverPosition_{};
	};

}

#endif
