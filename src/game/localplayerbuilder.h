#ifndef MWETRIS_GAME_LOCALPLAYERBUILDER_H
#define MWETRIS_GAME_LOCALPLAYERBUILDER_H

#include "localplayer.h"
#include "tetrisboard.h"
#include "eventmanager.h"

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

		LocalPlayerBuilder& widthMovingBlock(const tetris::Block& block) {
			movingBlockType_ = tetris::BlockType::Empty;
			movingBlock_ = block;
			return *this;
		}

		LocalPlayerBuilder& widthMovingBlockType(const tetris::BlockType& blockType) {
			movingBlockType_ = blockType;
			return *this;
		}

		LocalPlayerBuilder& widthNextBlockType(tetris::BlockType next) {
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

		LocalPlayerBuilder& widthBoard(const std::vector<tetris::BlockType>& board) {
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
		int points_{0};
		int level_{1};
		int levelUpCounter_{0};
		int clearedRows_{0};
		int width_{10};
		int height_{24};
		tetris::Block movingBlock_;
		tetris::BlockType movingBlockType_{tetris::BlockType::Empty};
		tetris::BlockType next_{tetris::BlockType::Empty};
		std::vector<tetris::BlockType> board_;
		int gameOverPosition_{};
	};

}

#endif
