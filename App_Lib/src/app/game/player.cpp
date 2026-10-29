#include "player.h"
#include "tetrisboardcontroller.h"
#include "input.h"
#include "computer.h"
#include "device.h"

#include <tetris/helper.h>

#include <SDL3/SDL_timer.h>
#include <spdlog/spdlog.h>

#include <algorithm>
#include <cstdint>
#include <optional>
#include <queue>

namespace app::game {
	namespace {
		void recordInputToMoveDelay(std::uint64_t eventTimestamp) {
			constexpr std::uint64_t ReportInterval = 5'000'000'000;
			constexpr double NanosecondsPerMillisecond = 1'000'000.0;
			static std::uint64_t reportStart = 0;
			static std::uint64_t total = 0;
			static std::uint64_t maximum = 0;
			static std::uint64_t samples = 0;

			const auto now = SDL_GetTicksNS();
			if (eventTimestamp == 0 || now < eventTimestamp) {
				return;
			}
			const auto delay = now - eventTimestamp;
			total += delay;
			maximum = std::max(maximum, delay);
			++samples;
			if (reportStart == 0) {
				reportStart = now;
				return;
			}
			if (now - reportStart < ReportInterval) {
				return;
			}

			spdlog::info(
				"[InputToMoveDelay] event-to-board-move samples={}, average={:.3f}ms, maximum={:.3f}ms",
				samples,
				static_cast<double>(total) / static_cast<double>(samples) / NanosecondsPerMillisecond,
				static_cast<double>(maximum) / NanosecondsPerMillisecond);
			reportStart = now;
			total = 0;
			maximum = 0;
			samples = 0;
		}

	}

	class TetrisBoardMoveController {
	public:
		virtual ~TetrisBoardMoveController() = default;

		virtual void updateMove(tetris::TetrisBoard& tetrisBoard, double deltaTime, PlayerBoardEventInvoker& invoker) {
		}

		virtual void setGravity(double gravity) {
		}

		virtual DevicePtr getDevice() const {
			return {};
		}

		virtual bool setDevice(DevicePtr device) {
			return false;
		}

		virtual std::optional<ProfileId> getProfileId() const {
			return std::nullopt;
		}

		virtual bool setProfileId(const ProfileId& profileId) {
			return false;
		}

		virtual void reset() {
		}
	};

	class HumanMoveController final : public TetrisBoardMoveController {
	public:
		HumanMoveController(DevicePtr device, std::shared_ptr<ControlProfiles> controlProfiles)
			: device_{std::move(device)}
			, profileId_{device_ ? controlProfiles->getAssignedProfileId(device_->getGuid()) : ControlProfiles::BalancedProfileId}
			, controlProfiles_{std::move(controlProfiles)} {
			connectDevice();
		}

		void updateMove(tetris::TetrisBoard& tetrisBoard, double deltaTime, PlayerBoardEventInvoker& invoker) override {
			auto profile = controlProfiles_->getProfile(profileId_);
			if (!profile) {
				profileId_ = ControlProfiles::BalancedProfileId;
				profile = controlProfiles_->getProfile(profileId_);
			}
			if (!appliedTiming_ || *appliedTiming_ != profile->timing) {
				tetrisBoardController_.setInputTiming(
					profile->timing.dasMs / 1000.0,
					profile->timing.arrMs / 1000.0);
				appliedTiming_ = profile->timing;
			}
			if (!device_) {
				return;
			}
			tetrisBoardController_.update(tetrisBoard, device_->getInput(), deltaTime, [&](const PlayerBoardEvent& event) {
				invoker.invokePlayerBoardEvent(event);
			}, [](const InputCommand& command) {
				if (!command.deferredUntilNextPiece) {
					recordInputToMoveDelay(command.timestamp);
				}
			});
		}

		void setGravity(double gravity) override {
			tetrisBoardController_.updateGravity(gravity);
		}

		DevicePtr getDevice() const override {
			return device_;
		}

		bool setDevice(DevicePtr device) override {
			if (!device) {
				return false;
			}
			if (device_ != device) {
				device_ = std::move(device);
				connectDevice();
			}
			return true;
		}

		std::optional<ProfileId> getProfileId() const override {
			return profileId_;
		}

		bool setProfileId(const ProfileId& profileId) override {
			if (!controlProfiles_->getProfile(profileId)) {
				return false;
			}
			profileId_ = profileId;
			return true;
		}

		void reset() override {
			tetrisBoardController_.reset();
		}

	private:
		void connectDevice() {
			inputConnections_.clear();
			tetrisBoardController_.reset();
			if (!device_) {
				return;
			}
			inputConnections_ += device_->command.connect([this](const InputCommand& command) {
				(void)tetrisBoardController_.enqueue(command);
			});
			inputConnections_ += device_->inputReset.connect([this]() {
				tetrisBoardController_.reset();
			});
			inputConnections_ += device_->connection.connect([this](bool connected) {
				if (!connected) {
					tetrisBoardController_.reset();
				}
			});
		}

		DevicePtr device_;
		ProfileId profileId_;
		std::shared_ptr<ControlProfiles> controlProfiles_;
		std::optional<InputTiming> appliedTiming_;
		TetrisBoardController tetrisBoardController_;
		mw::signals::ScopedConnections inputConnections_;
	};

	class AiMoveController : public TetrisBoardMoveController {
	public:
		AiMoveController(const tetris::Ai& ai)
			: computer_{ai} {
		}

		void updateMove(tetris::TetrisBoard& tetrisBoard, double deltaTime, PlayerBoardEventInvoker& invoker) override {
			const auto input = computer_.getInput();
			enqueuePressTransitions(previousInput_, input);
			previousInput_ = input;
			tetrisBoardController_.update(tetrisBoard, input, deltaTime, [&](const PlayerBoardEvent& playerBoardEvent) {
				invoker.invokePlayerBoardEvent(playerBoardEvent);
				if (auto tetrisBoardEvent = std::get_if<TetrisBoardEvent>(&playerBoardEvent)) {
					computer_.onGameboardEvent(tetrisBoard, tetrisBoardEvent->event, tetrisBoardEvent->value);
				}
			});
		}

		void setGravity(double gravity) override {
			tetrisBoardController_.updateGravity(gravity);
		}

		void reset() override {
			tetrisBoardController_.reset();
			previousInput_ = {};
		}

	private:
		void enqueuePressTransitions(const Input& previous, const Input& current) {
			const auto enqueue = [this](InputAction action, bool before, bool now) {
				if (now && !before) {
					(void)tetrisBoardController_.enqueue(InputCommand{action, 0});
				}
			};
			enqueue(InputAction::Left, previous.left, current.left);
			enqueue(InputAction::Right, previous.right, current.right);
			enqueue(InputAction::SoftDrop, previous.down, current.down);
			enqueue(InputAction::Rotate, previous.rotate, current.rotate);
			enqueue(InputAction::HardDrop, previous.downGround, current.downGround);
		}

		TetrisBoardController tetrisBoardController_;
		Computer computer_;
		Input previousInput_{};
	};

	PlayerPtr createHumanPlayer(DevicePtr device, std::shared_ptr<ControlProfiles> controlProfiles, const PlayerData& playerData, tetris::TetrisBoard&& tetrisBoard) {
		auto moveController = std::make_unique<HumanMoveController>(std::move(device), std::move(controlProfiles));
		auto player = std::make_shared<Player>(
			Player::Type::Human,
			std::move(moveController),
			std::move(tetrisBoard));
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

	Player::Player(
		Type type,
		std::unique_ptr<TetrisBoardMoveController> moveController,
		tetris::TetrisBoard&& tetrisBoard)
		: moveController_{std::move(moveController)}
		, tetrisBoard_{std::move(tetrisBoard)}
		, type_{type} {
	}
	
	void Player::update(double deltaTime) {
		moveController_->updateMove(tetrisBoard_, deltaTime, *this);
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
		resetInput();
		externalRows_.clear();
		clearedRows_ = 0;

		UpdateRestart updateRestart{
			.current = current,
			.next = next
		};
		playerBoardUpdate(updateRestart);
		tetrisBoard_.restart(current, next);
	}

	void Player::resetInput() {
		moveController_->reset();
	}

	void Player::updatePlayerData(const PlayerData& playerData) {
		playerBoardUpdate(UpdatePlayerData{playerData});
		playerData_ = playerData;
	}

	void Player::updateMove(tetris::Move move) {
		playerBoardUpdate(UpdateMove{move});
		tetrisBoard_.update(move, [&](tetris::BoardEvent event, int nbr) {
			invokePlayerBoardEvent(TetrisBoardEvent{
				.event = event,
				.value = nbr
			});
		});
	}

	void Player::updateNextBlock(tetris::BlockType next) {
		tetrisBoard_.setNextBlock(next);
	}

	void Player::addExternalRows(const std::vector<tetris::BlockType>& rows) {
		playerBoardUpdate(ExternalRows{rows});
		externalRows_.insert(externalRows_.end(), rows.begin(), rows.end());
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

	DevicePtr Player::getDevice() const {
		return moveController_->getDevice();
	}

	bool Player::setDevice(DevicePtr device) {
		return moveController_->setDevice(std::move(device));
	}

	std::optional<ProfileId> Player::getProfileId() const {
		return moveController_->getProfileId();
	}

	bool Player::setProfileId(const ProfileId& profileId) {
		return moveController_->setProfileId(profileId);
	}

	void Player::setGravity(double gravity) {
		moveController_->setGravity(gravity);
	}

	void Player::invokePlayerBoardEvent(const PlayerBoardEvent& playerBoardEvent) {
		playerBoardUpdate(playerBoardEvent);
		if (auto value = std::get_if<TetrisBoardEvent>(&playerBoardEvent)) {
			handleBoardEvent(value->event, value->value);
		}
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
			playerBoardUpdate(nextBlock);
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

}
