#include "tetriscontroller.h"
#include "game/aiplayer.h"
#include "game/tetrisgame.h"
#include "game/device.h"
#include "game/gamerules.h"
#include "graphic/gamecomponent.h"
#include "network/network.h"

#include <helper.h>

#include <spdlog/spdlog.h>

namespace mwetris {

	TetrisController::TetrisController(std::shared_ptr<network::Network> network,
		std::shared_ptr<game::TetrisGame> tetrisGame,
		std::shared_ptr<graphic::GameComponent> gameComponent)
		: network_{network}
		, tetrisGame_{tetrisGame}
		, gameComponent_{gameComponent} {
		
		connections_ += network_->pauseEvent.connect([this](const network::PauseEvent& pauseEvent) {
			tetrisGame_->setPause(pauseEvent.pause);
		});
		connections_ += network_->restartEvent.connect([this](const network::RestartEvent& restartEvent) {
			tetrisGame_->restartGame(restartEvent.current, restartEvent.next);
		});
		connections_ += network_->createGameEvent.connect([this](const network::CreateGameEvent& createGameEvent) {
			tetrisGame_->createGame(std::make_unique<game::DefaultGameRules>(), createGameEvent.players);
		});
		connections_ += network_->joinGameRoomEvent.connect([this](const network::JoinGameRoomEvent& joinGameRoomEvent) {
		});
		connections_ += network_->playerSlotEvent.connect([this](const network::PlayerSlotEvent& playerSlotEvent) {
		});
		
		connections_ += tetrisGame_->initGameEvent.connect([this](const game::InitGameEvent& initGameEvent) {
			gameComponent_->initGame(initGameEvent);
		});
		connections_ += tetrisGame_->gamePauseEvent.connect([this](const game::GamePause& gamePause) {
			tetrisEvent(gamePause);
			gameComponent_->gamePause(gamePause);
		});
	}

	// Updates everything. Should be called each frame.
	void TetrisController::update(double deltaTime) {
		network_->update();
		tetrisGame_->update(deltaTime);
	}

	void TetrisController::draw(int width, int height, double deltaTime) {
		gameComponent_->draw(width, height, deltaTime);
	}

	void TetrisController::createDefaultGame(game::DevicePtr device) {
		tetrisGame_->createDefaultGame(device);
	}

	void TetrisController::startNetworkGame(int w, int h) {
		network_->startGame(w, h);
	}

	void TetrisController::createGame(std::unique_ptr<game::GameRules> gameRules, const std::vector<game::PlayerPtr>& players) {
		if (network_->isInsideRoom()) {
			spdlog::error("Can't create a local game when inside a room.");
			return;
		}
		tetrisGame_->createGame(std::move(gameRules), players);
	}

	bool TetrisController::isPaused() const {
		return tetrisGame_->isPaused();
	}

	void TetrisController::pause() {
		if (network_->isActive()) {
			tetrisGame_->pause();
		} else {
			network_->sendPause(!tetrisGame_->isPaused()); // TODO! May need to handle delays, to avoid multiple pause events.
		}
	}

	void TetrisController::restartGame() {
		if (network_->isActive()) {
			network_->sendRestart();
		} else {
			tetrisGame_->restartGame(tetris::randomBlockType(), tetris::randomBlockType());
		}
	}

	const std::string& TetrisController::getGameRoomUuid() const {
		return network_->getGameRoomUuid();
	}

	void TetrisController::createGameRoom(const std::string& name) {
		network_->createGameRoom(name);
	}

	void TetrisController::joinGameRoom(const std::string& gameRoomUuid) {
		network_->joinGameRoom(gameRoomUuid);
	}

	void TetrisController::setPlayerSlot(const game::PlayerSlot& playerSlot, int slot) {
		if (network_->isActive()) {
			network_->setPlayerSlot(playerSlot, slot);
		} else {
			tetrisEvent(PlayerSlotEvent{playerSlot, slot}); // TODO! Does this work?
		}
	}

	int TetrisController::getNbrOfPlayers() const {
		return tetrisGame_->getNbrOfPlayers();
	}

	void TetrisController::saveDefaultGame() {
		tetrisGame_->saveDefaultGame();
	}

	void TetrisController::setFixTimestep(double delta) {
		tetrisGame_->setFixTimestep(delta);
	}

	bool TetrisController::isDefaultGame() const {
		return !network_->isInsideRoom() && tetrisGame_->isDefaultGame();
	}

}
