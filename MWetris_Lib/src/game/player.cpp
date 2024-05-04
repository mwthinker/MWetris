#include "player.h"
#include "tetrisboardcontroller.h"
#include "input.h"
#include "computer.h"
#include "device.h"

#include "helper.h"

#include <queue>

namespace mwetris::game {

	class TetrisBoardMoveController {
	public:
		virtual ~TetrisBoardMoveController() = default;

		virtual void updateMove(tetris::TetrisBoard& tetrisBoard, double deltaTime, std::function<void(const PlayerBoardEvent&)> callback);

		virtual void setGravity(double gravity) {}
	};

	class HumanMoveController : public TetrisBoardMoveController {
	public:
		HumanMoveController(DevicePtr device);

		void updateMove(tetris::TetrisBoard& tetrisBoard, double deltaTime, std::function<void(const PlayerBoardEvent&)> callback) override;

		void setGravity(double gravity) override {
			tetrisBoardController_.updateGravity(gravity);
		}

	private:
		DevicePtr device_;
		TetrisBoardController tetrisBoardController_;
	};

	class AiMoveController : public TetrisBoardMoveController {
	public:
		AiMoveController(const tetris::Ai& ai);

		void updateMove(tetris::TetrisBoard& tetrisBoard, double deltaTime, std::function<void(const PlayerBoardEvent&)> callback) override;

		void setGravity(double gravity) override {
			tetrisBoardController_.updateGravity(gravity);
		}

	private:
		TetrisBoardController tetrisBoardController_;
		Computer computer_;
	};

	void TetrisBoardMoveController::updateMove(tetris::TetrisBoard& tetrisBoard, double deltaTime, std::function<void(const PlayerBoardEvent&)> callback) {}

	HumanMoveController::HumanMoveController(DevicePtr device)
		: device_{device} {}

	void HumanMoveController::updateMove(tetris::TetrisBoard& tetrisBoard, double deltaTime, std::function<void(const PlayerBoardEvent&)> callback) {
		tetrisBoardController_.update(tetrisBoard, device_->getInput(), deltaTime, [&](const PlayerBoardEvent& event) {
			callback(event);
		});
	}

	AiMoveController::AiMoveController(const tetris::Ai& ai)
		: computer_{tetris::Ai{}} {}

	void AiMoveController::updateMove(tetris::TetrisBoard& tetrisBoard, double deltaTime, std::function<void(const PlayerBoardEvent&)> callback) {
		tetrisBoardController_.update(tetrisBoard, computer_.getInput(), deltaTime, [&](const PlayerBoardEvent& playerBoardEvent) {
			callback(playerBoardEvent);
			if (auto tetrisBoardEvent = std::get_if<TetrisBoardEvent>(&playerBoardEvent)) {
				computer_.onGameboardEvent(tetrisBoard, tetrisBoardEvent->event, tetrisBoardEvent->value);
			}
		});
	}

	PlayerPtr createHumanPlayer(DevicePtr device, const PlayerData& playerData, tetris::TetrisBoard&& tetrisBoard) {
		auto moveController = std::make_unique<HumanMoveController>(device);
		auto player = std::make_shared<Player>(Player::Type::Human, std::move(moveController), std::move(tetrisBoard));
		player->updatePlayerData(playerData);
		return player;
	}

	PlayerPtr createAiPlayer(const tetris::Ai& ai, const PlayerData& playerData, tetris::TetrisBoard&& tetrisBoard) {
		auto moveController = std::make_unique<AiMoveController>(ai);
		auto player = std::make_shared<Player>(Player::Type::Ai, std::move(moveController), std::move(tetrisBoard));
		player->updatePlayerData(playerData);
		return player;
	}

	PlayerPtr createRemotePlayer(const PlayerData& playerData, tetris::TetrisBoard&& tetrisBoard) {
		auto moveController = std::make_unique<TetrisBoardMoveController>();
		auto player = std::make_shared<Player>(Player::Type::Remote, std::move(moveController), std::move(tetrisBoard));
		player->updatePlayerData(playerData);
		return player;
	}

	Player::Player(Type type, std::unique_ptr<TetrisBoardMoveController> moveController, tetris::TetrisBoard&& tetrisBoard)		
		: tetrisBoard_{tetrisBoard}
		, moveController_{std::move(moveController)}
		, type_{type} {
	}
	
	void Player::update(double deltaTime) {
		moveController_->updateMove(tetrisBoard_, deltaTime, [&](const PlayerBoardEvent& playerBoardEvent) {
			invokePlayerBoardUpdate(playerBoardEvent);
			if (auto value = std::get_if<TetrisBoardEvent>(&playerBoardEvent)) {
				handleBoardEvent(value->event, value->value);
			}
		});
	}

	int Player::getRows() const {
		return tetrisBoard_.getRows();
	}

	int Player::getColumns() const {
		return tetrisBoard_.getColumns();
	}

	tetris::BlockType Player::getBlockType(int x, int y) const {
		return tetrisBoard_.getBlockType(x, y);
	}

	tetris::BlockType Player::getNextBlockType() const {
		return tetrisBoard_.getNextBlockType();
	}

	int Player::getClearedRows() const {
		return clearedRows_;
	}

	bool Player::isGameOver() const {
		return tetrisBoard_.isGameOver();
	}

	void Player::updateRestart(tetris::BlockType current, tetris::BlockType next) {
		externalRows_.clear();
		clearedRows_ = 0;

		UpdateRestart updateRestart{
			.current = current,
			.next = next
		};
		invokePlayerBoardUpdate(updateRestart);
		tetrisBoard_.restart(current, next);
	}

	void Player::updatePlayerData(const PlayerData& playerData) {
		invokePlayerBoardUpdate(UpdatePlayerData{playerData});
		playerData_ = playerData;
	}

	void Player::updateMove(tetris::Move move) {
		invokePlayerBoardUpdate(UpdateMove{move});
		tetrisBoard_.update(move, [&](tetris::BoardEvent event, int nbr) {
			handleBoardEvent(event, nbr);
		});
	}

	void Player::updateNextBlock(tetris::BlockType next) {
		tetrisBoard_.setNextBlock(next);
	}

	tetris::Block Player::getBlockDown() const {
		return tetrisBoard_.getBlockDown();
	}

	tetris::Block Player::getBlock() const {
		return tetrisBoard_.getBlock();
	}

	const PlayerData& Player::getPlayerData() const {
		return playerData_;
	}

	bool Player::isHuman() const {
		return type_ == Type::Human;
	}

	bool Player::isAi() const {
		return type_ == Type::Ai;
	}

	bool Player::isRemote() const {
		return type_ == Type::Remote;
	}

	bool Player::isLocal() const {
		return !isRemote();
	}

	void Player::setGravity(double gravity) {
		moveController_->setGravity(gravity);
	}

	void Player::setClearedRows(int clearedRows) {
		clearedRows_ = clearedRows;
	}

	const std::vector<tetris::BlockType>& Player::getBoardVector() const {
		return tetrisBoard_.getBoardVector();
	}

	void Player::handleBoardEvent(tetris::BoardEvent boardEvent, int value) {
		if (boardEvent == tetris::BoardEvent::CurrentBlockUpdated) {
			UpdateNextBlock nextBlock{
				.next = tetris::randomBlockType()
			};
			invokePlayerBoardUpdate(nextBlock);
			tetrisBoard_.setNextBlock(nextBlock.next);
		}
		if (boardEvent == tetris::BoardEvent::RowsRemoved) {
			clearedRows_ += value;
		}
		if (boardEvent == tetris::BoardEvent::BlockCollision) {
			tetrisBoard_.addExternalRows(externalRows_);
			externalRows_.clear();
		}
	}

	void Player::invokePlayerBoardUpdate(PlayerBoardEvent playerBoardEvent) {
		playerBoardUpdate(playerBoardEvent);
	}

}
