#include "tetriscontroller.h"
#include "game/tetrisgame.h"
#include "game/device.h"
#include "game/gamerules.h"
#include "graphic/gamecomponent.h"
#include "network/network.h"

#include <tetris/helper.h>

#include <spdlog/spdlog.h>

namespace mwetris {

	TetrisController::TetrisController(std::shared_ptr<network::Network> network,
		std::shared_ptr<game::TetrisGame> tetrisGame,
		std::shared_ptr<graphic::GameComponent> gameComponent)
		: network_{network}
		, tetrisGame_{tetrisGame}
		, gameComponent_{gameComponent} {

		connections_ += network_->networkEvent.connect([this](const network::NetworkEvent& networkEvent) {
			std::visit([&](auto&& event) {
				onNetworkEvent(event);
			}, networkEvent);
		});

		connections_ += tetrisGame_->initGameEvent.connect([this](const game::InitGameEvent& initGameEvent) {
			gameComponent_->initGame(initGameEvent);
		});
		connections_ += tetrisGame_->gamePauseEvent.connect([this](const game::GamePause& gamePause) {
			tetrisEvent(gamePause);
			gameComponent_->gamePause(gamePause);
		});
	}

	void TetrisController::onNetworkEvent(const network::PlayerSlotEvent& playerSlotEvent) {
		tetrisEvent(PlayerSlotEvent{
			.playerSlot = playerSlotEvent.playerSlot,
			.slot = playerSlotEvent.index
		});
	}

	void TetrisController::onNetworkEvent(const network::RestartEvent& restartEvent) {
		tetrisGame_->restartGame(restartEvent.current, restartEvent.next);
	}

	void TetrisController::onNetworkEvent(const network::JoinGameRoomEvent& joinGameRoomEvent) {
		setGameRoomType(GameRoomType::NetworkInsideGameRoom);
	}

	void TetrisController::onNetworkEvent(const network::CreateGameRoomEvent& createGameRoomEvent) {
		setGameRoomType(GameRoomType::NetworkInsideGameRoom);
	}

	void TetrisController::onNetworkEvent(const network::PauseEvent& pauseEvent) {
		tetrisGame_->setPause(pauseEvent.pause);
	}

	void TetrisController::onNetworkEvent(const network::CreateGameEvent& createGameEvent) {
		tetrisGame_->createGame(std::make_unique<game::SurvivalGameRules>(), createGameEvent.players);
		tetrisEvent(CreateGameEvent{});
	}

	void TetrisController::onNetworkEvent(const network::LeaveGameRoomEvent& leaveGameRoomEvent) {
		setGameRoomType(GameRoomType::OutsideGameRoom);
	}

	void TetrisController::onNetworkEvent(const network::ClientDisconnectedEvent& clientDisconnectedEvent) {
		// TODO! Handle client disconnect from server.
	}

	void TetrisController::onNetworkEvent(const network::GameRoomListEvent& gameRoomListEvent) {
		tetrisEvent(gameRoomListEvent);
	}

	// Updates everything. Should be called each frame.
	void TetrisController::update(double deltaTime) {
		tetrisGame_->update(deltaTime);
	}

	void TetrisController::draw(int width, int height, double deltaTime) {
		gameComponent_->draw(width, height, deltaTime);
	}

	void TetrisController::createDefaultGame(game::DevicePtr device) {
		if (network_->isInsideGameRoom()) {
			network_->leaveGameRoom();
		}
		tetrisGame_->createDefaultGame(device);
		tetrisGame_->saveDefaultGame();
	}

	void TetrisController::startNetworkGame(int w, int h) {
		network_->startGame(w, h);
	}

	void TetrisController::startLocalGame(std::unique_ptr<game::GameRules> gameRules, const std::vector<game::PlayerPtr>& players) {
		if (network_->isInsideGameRoom()) {
			spdlog::debug("[TetrisController] Leaving game room before starting local game.");
			network_->leaveGameRoom();
		}
		tetrisGame_->createGame(std::move(gameRules), players);
		tetrisGame_->saveDefaultGame();
		tetrisEvent(CreateGameEvent{});
	}

	bool TetrisController::isPaused() const {
		return tetrisGame_->isPaused();
	}

	void TetrisController::pause() {
		if (network_->isInsideGameRoom()) {
			network_->sendPause(!tetrisGame_->isPaused()); // TODO! May need to handle delays, to avoid multiple pause events.
		} else {
			tetrisGame_->pause();
		}
	}

	void TetrisController::restartGame() {
		if (network_->isInsideGameRoom()) {
			network_->sendRestart();
		} else {
			tetrisGame_->restartGame(tetris::randomBlockType(), tetris::randomBlockType());
		}
	}

	const char* TetrisController::getGameRoomId() const {
		return network_->getGameRoomId().c_str();
	}

	bool TetrisController::isInsideGameRoom() const {
		switch (gameRoomType_) {
			case GameRoomType::LocalInsideGameRoom: return true;
			case GameRoomType::OutsideGameRoom: return false;
			case GameRoomType::NetworkWaitingCreateGameRoom: return false;
			case GameRoomType::NetworkInsideGameRoom: return true;
		}
		return false;
	}

	void TetrisController::createLocalGameRoom() {
		setGameRoomType(GameRoomType::LocalInsideGameRoom);
	}

	void TetrisController::createNetworkGameRoom(const std::string& name) {
		setGameRoomType(GameRoomType::NetworkWaitingCreateGameRoom);
		network_->createGameRoom(name);
	}

	void TetrisController::joinGameRoom(const std::string& gameRoomId) {
		network_->joinGameRoom(network::GameRoomId{gameRoomId});
	}

	void TetrisController::leaveGameRoom() {
		if (network_->isInsideGameRoom()) {
			network_->leaveGameRoom();
		} else {
			setGameRoomType(GameRoomType::OutsideGameRoom);
		}
	}

	void TetrisController::setPlayerSlot(const game::PlayerSlot& playerSlot, int slot) {
		if (network_->isInsideGameRoom()) {
			network_->setPlayerSlot(playerSlot, slot);
		} else {
			tetrisEvent(PlayerSlotEvent{playerSlot, slot});
		}
	}

	int TetrisController::getNbrOfPlayers() const {
		return tetrisGame_->getNbrOfPlayers();
	}

	void TetrisController::saveDefaultGame() {
		if (network_->isInsideGameRoom()) {
			spdlog::debug("[TetrisController] Can't save default game when inside a room.");
			return;
		}

		tetrisGame_->saveDefaultGame();
	}

	void TetrisController::setFixTimestep(double delta) {
		tetrisGame_->setFixTimestep(delta);
	}

	bool TetrisController::isDefaultGame() const {
		return !network_->isInsideGameRoom() && tetrisGame_->isDefaultGame();
	}

	void TetrisController::refreshGameRoomList() {
		network_->requestGameRoomList();
	}

	void TetrisController::setGameRoomType(GameRoomType gameRoomType) {
		gameRoomType_ = gameRoomType;
		tetrisEvent(GameRoomEvent{
			.type = gameRoomType_
		});
	}

}
