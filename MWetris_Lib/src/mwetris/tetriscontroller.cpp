#include "tetriscontroller.h"
#include "game/serialize.h"
#include "game/tetrisgame.h"
#include "game/device.h"
#include "game/gamerules.h"
#include "graphic/gamecomponent.h"
#include "network/network.h"

#include <tetris/helper.h>

#include <spdlog/spdlog.h>

namespace mwetris {

	namespace {

		std::unique_ptr<game::GameRules> createGameRulesInstance(const game::DefaultGameRules::Config& config) {
			return std::make_unique<game::DefaultGameRules>();
		}

		std::unique_ptr<game::GameRules> createGameRulesInstance(const game::SurvivalGameRules::Config& config) {
			return std::make_unique<game::SurvivalGameRules>();
		}

		std::unique_ptr<game::GameRules> createGameRules(const game::GameRulesConfig& gameRulesConfig) {
			std::unique_ptr<game::GameRules> rules;
			
			std::visit([&](auto&& config) {
				rules = createGameRulesInstance(config);
			}, gameRulesConfig);

			return rules;
		}

	}
		

	TetrisController::TetrisController(std::shared_ptr<game::DeviceManager> deviceManager, std::shared_ptr<network::Network> network, std::shared_ptr<graphic::GameComponent> gameComponent)
		: deviceManager_{deviceManager}
		, network_{network}
		, gameComponent_{gameComponent} {

		connections_ += network_->networkEvent.connect([this](const network::NetworkEvent& networkEvent) {
			std::visit([&](auto&& event) {
				onNetworkEvent(event);
			}, networkEvent);
		});
		connections_ += tetrisGame_.gamePauseEvent.connect([this](const game::GamePause& gamePause) {
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

	void TetrisController::onNetworkEvent(const network::GameRoomEvent& gameRoomEvent) {
		gameRoomClients_ = gameRoomEvent.gameRoomClients;
	}

	void TetrisController::onNetworkEvent(const network::RestartEvent& restartEvent) {
		rules_->restart();
	}

	void TetrisController::onNetworkEvent(const network::JoinGameRoomEvent& joinGameRoomEvent) {
		setGameRoomType(GameRoomType::NetworkGameRoomLooby);
	}

	void TetrisController::onNetworkEvent(const network::PauseEvent& pauseEvent) {
		tetrisGame_.setPause(pauseEvent.pause);
	}

	void TetrisController::onNetworkEvent(const network::CreateGameEvent& createGameEvent) {
		gameRoomType_ = GameRoomType::GameSession;
		createGame(createGameEvent.players, createGameEvent.gameRulesConfig);
	}

	void TetrisController::onNetworkEvent(const network::LeaveGameRoomEvent& leaveGameRoomEvent) {
		gameRoomType_ = GameRoomType::OutsideGameRoom;
		//createDefaultGame(deviceManager_->getDefaultDevice1());
	}

	void TetrisController::onNetworkEvent(const network::ClientDisconnectedEvent& clientDisconnectedEvent) {
		// TODO! Handle client disconnect from server.
	}

	void TetrisController::onNetworkEvent(const network::GameRoomListEvent& gameRoomListEvent) {
		tetrisEvent(gameRoomListEvent);
	}

	void TetrisController::onNetworkEvent(const game::GameRoomConfigEvent& gameRoomConfigEvent) {
		gameRulesConfig_ = gameRoomConfigEvent.gameRulesConfig;
		tetrisEvent(gameRoomConfigEvent);
	}

	// Updates everything. Should be called each frame.
	void TetrisController::update(double deltaTime) {
		tetrisGame_.update(deltaTime);
	}

	void TetrisController::draw(int width, int height, double deltaTime) {
		gameComponent_->draw(width, height, deltaTime);
	}

	void TetrisController::createDefaultGame(game::DevicePtr device) {
		if (network_->isInsideGameRoom()) {
			network_->leaveGameRoom();
		}
		game::PlayerPtr player = loadGame(device);
		if (!player) {
			game::DefaultPlayerData playerData{
				.level = 1,
				.points = 0
			};
			tetris::TetrisBoard tetrisBoard{game::TetrisWidth, game::TetrisHeight, tetris::randomBlockType(), tetris::randomBlockType()};
			player = createHumanPlayer(device, playerData, std::move(tetrisBoard));
		}
		connections_ += player->playerBoardUpdate.connect([](const game::PlayerBoardEvent& playerBoardEvent) {
			game::clearSavedGame();
		});
		gameRoomType_ = GameRoomType::GameSession;
		createGame({player}, game::DefaultGameRules::Config{});
		saveDefaultGame();
	}

	void TetrisController::startNetworkGame(const game::GameRulesConfig& gameRulesConfig, int w, int h) {
		network_->startGame(gameRulesConfig);
	}

	void TetrisController::startLocalGame(const game::GameRulesConfig& gameRulesConfig, const std::vector<game::PlayerPtr>& players) {
		if (network_->isInsideGameRoom()) {
			spdlog::debug("[TetrisController] Leaving game room before starting local game.");
			network_->leaveGameRoom();
		}
		gameRoomType_ = GameRoomType::GameSession;
		createGame(players, gameRulesConfig);
		saveDefaultGame();
	}

	bool TetrisController::isPaused() const {
		return tetrisGame_.isPaused();
	}

	void TetrisController::pause() {
		if (network_->isInsideGameRoom()) {
			network_->sendPause(!tetrisGame_.isPaused()); // TODO! May need to handle delays, to avoid multiple pause events.
		} else {
			tetrisGame_.pause();
		}
	}

	void TetrisController::restartGame() {
		if (network_->isInsideGameRoom()) {
			network_->sendRestart();
		} else {
			auto current = tetris::randomBlockType();
			auto next = tetris::randomBlockType();

			for (auto& player : tetrisGame_.getPlayers()) {
				player->updateRestart(current, next);
			}
			rules_->restart();
		}
	}

	void TetrisController::updateGameRulesConfig(const game::GameRulesConfig& gameRulesConfig) {
		if (network_->isInsideGameRoom()) {
			// TODO! Send game rules config to server.
		} else {
			// TODO! Update game rules config.
		}
	}

	const char* TetrisController::getGameRoomId() const {
		return network_->getGameRoomId().c_str();
	}

	bool TetrisController::isGameRoomSession() const {
		return network_->isInsideGameRoom() || gameRoomType_ == GameRoomType::LocalGameRoomLooby;
	}

	void TetrisController::createLocalGameRoom() {
		setGameRoomType(GameRoomType::LocalGameRoomLooby);
	}

	void TetrisController::createNetworkGameRoom(const std::string& name, bool isPublic) {
		setGameRoomType(GameRoomType::NetworkWaitingCreateGameRoom);
		network_->createGameRoom(name, isPublic);
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
		return tetrisGame_.getNbrOfPlayers();
	}

	void TetrisController::saveDefaultGame() {
		if (network_->isInsideGameRoom()) {
			spdlog::debug("[TetrisController] Can't save default game when inside a room.");
			return;
		}

		if (!isDefaultGame()) {
			return;
		}

		const auto& player = *(tetrisGame_.getPlayers().front());
		if (!player.isGameOver()) {
			saveGame(player);
		}
	}

	void TetrisController::setFixTimestep(double delta) {
		tetrisGame_.setFixTimestep(delta);
	}

	void TetrisController::refreshGameRoomList() {
		network_->requestGameRoomList();
	}

	void TetrisController::setGameRoomType(GameRoomType gameRoomType) {
		if (gameRoomType != GameRoomType::NetworkGameRoomLooby) {
			gameRoomClients_.clear();
		}

		gameRoomType_ = gameRoomType;
		tetrisEvent(GameRoomEvent{
			.type = gameRoomType_
		});
	}

	void TetrisController::createGame(const std::vector<game::PlayerPtr>& players, const game::GameRulesConfig& gameRulesConfig) {
		gameRulesConfig_ = gameRulesConfig;
		rules_ = createGameRules(gameRulesConfig);
		tetrisGame_.createGame(players);
		gameComponent_->initGame(players);
		rules_->createGame(players);
		setGameRoomType(gameRoomType_);
	}

	bool TetrisController::isDefaultGame() const {
		if (network_->isInsideGameRoom()) {
			return false;
		}

		const auto& players = tetrisGame_.getPlayers();

		if (players.size() != 1) {
			return false;
		}

		const auto& player = *players.front();

		if (player.isLocal()) {
			return player.isLocal();
		}

		if (player.getRows() != game::TetrisHeight) {
			return false;
		}

		if (player.getColumns() != game::TetrisWidth) {
			return false;
		}

		if (!dynamic_cast<game::DefaultGameRules*>(rules_.get())) {
			return false;
		}

		return player.isHuman();
	}

	const std::vector<network::GameRoomClient>& TetrisController::getGameRoomClients() const {
		return gameRoomClients_;
	}

}
