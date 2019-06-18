#ifndef LOCALPLAYER_H
#define LOCALPLAYER_H

#include "player.h"
#include "tetrisboard.h"
#include "actionhandler.h"
#include "device.h"
#include "playerdata.h"

#include "protocol.h"

namespace tetris {

//#include <net/connection.h>
//#include <net/packet.h>

	class ILocalPlayer;
	using ILocalPlayerPtr = std::shared_ptr<ILocalPlayer>;

	class LocalPlayerBuilder {
	public:
		LocalPlayerBuilder();

		ILocalPlayerPtr build();

		LocalPlayerBuilder withWidth(int width) {
			width_ = width;
			return *this;
		}

		LocalPlayerBuilder& withHeight(int height) {
			height_ = height;
			return *this;
		}

		LocalPlayerBuilder& widthMovingBlock(const Block& block) {
			movingBlockType_ = BlockType::EMPTY;
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

		LocalPlayerBuilder& widthDevice(IDevicePtr device) {
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

	private:
		void init() {
			name_ = "PLAYER";
			points_ = 0;
			level_ = 1;
			levelUpCounter_ = 0;
			clearedRows_ = 0;
			points_ = 0;
			width_ = 10;
			height_ = 24;
			movingBlockType_ = BlockType::EMPTY;
		}

		IDevicePtr device_;
		std::string name_;
		int points_, level_, levelUpCounter_, clearedRows_;
		int width_, height_;
		Block movingBlock_;
		BlockType movingBlockType_;
		BlockType next_;
		std::vector<BlockType> board_;
		int gameOverPosition_;
	};

	class ILocalPlayer : public IPlayer {
	public:
		ILocalPlayer(const RawTetrisBoard& board, const IDevicePtr& device);

		virtual ~ILocalPlayer();

		ILocalPlayer(const ILocalPlayer&) = delete;
		ILocalPlayer& operator=(const ILocalPlayer&) = delete;

		virtual void update(double deltaTime);

		virtual void addRow(int holes);

		virtual void updatePoints(int points);

		virtual void updateName(const std::string& name);

		virtual void updateLevelUpCounter(int counter);

		virtual void updateLevel(int level);

		virtual void updateGameOverPosition(int gameOverPosition);

		virtual void updateRestart();

		virtual void updateGameOver();

		mw::signals::Connection addGameEventListener(
			const std::function<void(BoardEvent, const TetrisBoard&)>& callback) override {
			return tetrisBoard_.addGameEventListener(callback);
		}

		std::string getName() const override {
			return name_;
		}

		int getLevel() const override {
			return level_;
		}

		int getPoints() const override {
			return points_;
		}

		inline int getClearedRows() const override {
			return tetrisBoard_.getRemovedRows();
		}

		inline int getLevelUpCounter() const override {
			return levelUpCounter_;
		}

		inline bool isGameOver() const override {
			return tetrisBoard_.isGameOver();
		}

		inline int getGameOverPosition() const override {
			return gameOverPosition_;
		}

		inline IDevicePtr getDevice() const override {
			return device_;
		}

		inline const TetrisBoard& getTetrisBoard() const override {
			return tetrisBoard_;
		}

		inline virtual float getGravityDownSpeed() const {
			return 1 + level_ * 0.5f;
		}

	private:
		void boardListener(BoardEvent gameEvent);

		IDevicePtr device_;
		std::string name_;
		int points_, level_, levelUpCounter_;
		TetrisBoard tetrisBoard_;
		int gameOverPosition_;
		mw::signals::Connection connection_;

		ActionHandler gravityMove_, downHandler_, leftHandler_, rightHandler_, rotateHandler_, downGroundHandler_;
	};

}

#endif // LOCALPLAYER_H
