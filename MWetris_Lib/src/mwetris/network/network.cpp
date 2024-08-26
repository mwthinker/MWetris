#include "network.h"

#include "game/playerslot.h"
#include "util/auxiliary.h"

#include <network/client.h>
#include <network/protobufmessagequeue.h>

#include <protocol/shared.pb.h>
#include <protocol/client_to_server.pb.h>
#include <protocol/server_to_client.pb.h>

#include <tetris/random.h>
#include <tetris/helper.h>

#include <fmt/core.h>
#include <spdlog/spdlog.h>

#include <asio.hpp>

namespace mwetris::network {

	namespace {

		game::GameRulesConfig createGameRulesConfig(const tp_s2c::CreateGame& createGame) {
			game::GameRulesConfig gameRoomConfig;
			if (createGame.has_game_rules()) {
				if (createGame.game_rules().has_default_game_rules()) {
					gameRoomConfig = game::DefaultGameRules::Config{};
				} else if (createGame.game_rules().has_survival_game_rules()) {
					gameRoomConfig = game::SurvivalGameRules::Config{};
				}
			}
			return gameRoomConfig;
		}

		void fromCppToProto(const game::GameRulesConfig& gameRoomConfig, tp::GameRules& tpGameRules) {
			std::visit([&](auto&& config) {
				using T = std::decay_t<decltype(config)>;
				if constexpr (std::is_same_v<T, game::DefaultGameRules::Config>) {
					tpGameRules.mutable_default_game_rules()->set_lines_per_level(10);
					tpGameRules.mutable_default_game_rules()->set_start_level(1);
					tpGameRules.mutable_default_game_rules()->set_max_level(10);
				} else if constexpr (std::is_same_v<T, game::SurvivalGameRules::Config>) {
					tpGameRules.mutable_survival_game_rules()->set_lines_per_level(10);
					tpGameRules.mutable_survival_game_rules()->set_start_level(1);
					tpGameRules.mutable_survival_game_rules()->set_max_level(10);
				} else {
					static_assert(util::always_false_v<T>, "non-exhaustive visitor!");
				}
			}, gameRoomConfig);
		}

		std::vector<GameRoomClient> extractGameRoomClients(const tp_s2c::GameRoomClients& tpGameRoomClients) {
			std::vector<GameRoomClient> gameRoomClients;
			for (const auto& client : tpGameRoomClients.clients()) {
				gameRoomClients.push_back(GameRoomClient{
					.clientId = client.client_id(),
					.connectionId = client.connection_id()
				});
			}
			return gameRoomClients;
		}

	}
		

	Network::Network(std::shared_ptr<Client> client)
		: client_{client}
		, timer_{client->getIoContext()} {

		start();
	}

	bool Network::isInsideGameRoom() const {
		return !gameRoomId_.isEmpty();
	}

	void Network::start() {
		running_ = true;

		asio::co_spawn(client_->getIoContext(), [this]() mutable -> asio::awaitable<void> {
			auto network = shared_from_this();
			co_await run(network);
		}, asio::detached);
	}

	void Network::stop() {
		client_->stop();
		// To avoid getting stuck
		gameRoomId_ = GameRoomId{};

		// To be able to stop the coroutine
		running_ = false;

		wrapperToServer_.Clear();
	}

	Network::~Network() {
		spdlog::info("[Network] Destructor");
		stop();

		// To be able to stop the coroutine
		running_ = false;
	}

	const GameRoomId& Network::getGameRoomId() const {
		return gameRoomId_;
	}

	void Network::startGame(const game::GameRulesConfig& gameRulesConfig) {
		if (!isInsideGameRoom()) {
			spdlog::warn("[Network] Can't start game, not inside a room");
			return;
		}

		wrapperToServer_.Clear();
		auto startGame = wrapperToServer_.mutable_start_game();
		startGame->set_ready(true);
		fromCppToProto(gameRulesConfig, *startGame->mutable_game_rules());
		send(wrapperToServer_);
	}

	void Network::sendPause(bool pause) {
		if (!isInsideGameRoom()) {
			spdlog::warn("[Network] Can't pause game, not inside a room");
			return;
		}

		wrapperToServer_.Clear();
		wrapperToServer_.mutable_game_command()->set_pause(pause);
		send(wrapperToServer_);
	}

	void Network::sendRestart() {
		if (!isInsideGameRoom()) {
			spdlog::warn("[Network] Can't restart game, not inside a room");
			return;
		}
		wrapperToServer_.Clear();
		auto requestGameRestart = wrapperToServer_.mutable_request_game_restart();
		requestGameRestart->set_restart(true);
		send(wrapperToServer_);
	}

	void Network::createGameRoom(const std::string& gameRoom, bool isPublic) {
		if (isInsideRoom()) {
			spdlog::warn("[Network] Can't create room, already inside a room");
			return;
		}
		wrapperToServer_.Clear();
		auto createGameRoom = wrapperToServer_.mutable_create_game_room();
		createGameRoom->set_name(gameRoom);
		createGameRoom->set_is_public(isPublic);
		send(wrapperToServer_);
	}

	void Network::leaveGameRoom() {
		if (!isInsideRoom()) {
			spdlog::warn("[Network] Can't leave room, not inside a room");
			return;
		}
		wrapperToServer_.Clear();
		auto leaveGameRoom = wrapperToServer_.mutable_leave_game_room();
		fromCppToProto(gameRoomId_, *leaveGameRoom->mutable_game_room_id());
		send(wrapperToServer_);
	}

	void Network::setPlayerSlot(const game::PlayerSlot& playerSlot, int index) {
		if (!isInsideRoom()) {
			spdlog::warn("[Network] Can't set player slot, not inside a room");
			return;
		}
		if (index >= networkSlots_.size()) {
			spdlog::error("[Network] Invalid slot index: {}", index);
			return;
		}

		wrapperToServer_.Clear();
		auto tpPlayerSlot = wrapperToServer_.mutable_player_slot();
		tpPlayerSlot->set_index(index);
		if (auto human = std::get_if<game::Human>(&playerSlot); human) {
			tpPlayerSlot->set_slot_type(tp_c2s::PlayerSlot_SlotType_HUMAN);
			tpPlayerSlot->set_name(human->name);
			deviceBySlotIndex_[index] = human->device;
			send(wrapperToServer_);
		} else if (auto ai = std::get_if<game::Ai>(&playerSlot); ai) {
			tpPlayerSlot->set_slot_type(tp_c2s::PlayerSlot_SlotType_AI);
			tpPlayerSlot->set_name(ai->name);
			aiBySlotIndex_[index] = ai->ai;
			send(wrapperToServer_);
		} else if (std::get_if<game::OpenSlot>(&playerSlot)) {
			tpPlayerSlot->set_slot_type(tp_c2s::PlayerSlot_SlotType_OPEN_SLOT);
			send(wrapperToServer_);
		} else if (std::get_if<game::ClosedSlot>(&playerSlot)) {
			tpPlayerSlot->set_slot_type(tp_c2s::PlayerSlot_SlotType_CLOSED_SLOT);
			send(wrapperToServer_);
		}
	}

	void Network::joinGameRoom(const GameRoomId& gameRoomId) {
		if (isInsideRoom()) {
			spdlog::warn("[Network] Can't join room, already inside a room");
			return;
		}

		connections_.clear();

		wrapperToServer_.Clear();
		auto joinGameRoom = wrapperToServer_.mutable_join_game_room();
		fromCppToProto(gameRoomId, *joinGameRoom->mutable_game_room_id());
		send(wrapperToServer_);
	}

	bool Network::isInsideRoom() const {
		return !gameRoomId_.isEmpty();
	}

	void Network::removeClient(const ClientId& clientId) {
		wrapperToServer_.Clear();
		auto removeClient = wrapperToServer_.mutable_remove_client();
		fromCppToProto(clientId, *removeClient->mutable_client_id());
		send(wrapperToServer_);
	}

	void Network::requestGameRoomList() {
		wrapperToServer_.Clear();
		auto requestGameRoomList = wrapperToServer_.mutable_request_game_room_list();
		requestGameRoomList->set_ranked(true);
		send(wrapperToServer_);
	}

	bool Network::isConnected() const {
		return client_->isConnected();
	}

	asio::awaitable<void> Network::run(std::shared_ptr<Network> network) try {
		int value = network.use_count();
		while (network->running_) {
			// Connect to server
			co_await nextMessage(network);
			if (network->wrapperFromServer_.has_failed_to_connect()) {
				continue;
			} else if (network->wrapperFromServer_.has_game_room_joined()) {
				network->handleGameRoomJoined(network->wrapperFromServer_.game_room_joined());
			} else if (network->wrapperFromServer_.has_game_room_list()) {
				network->handleGameRoomList(network->wrapperFromServer_.game_room_list());
				continue;
			} else {
				continue;
			}

			// Wait for game to start
			do {
				co_await nextMessage(network);
				if (network->wrapperFromServer_.has_game_looby()) {
					network->handleGameLooby(network->wrapperFromServer_.game_looby());
				}
				if (network->wrapperFromServer_.has_game_room_joined()) {
					network->handleGameRoomJoined(network->wrapperFromServer_.game_room_joined());
				}
				if (network->wrapperFromServer_.has_connections()) {
					network->handleConnections(network->wrapperFromServer_.connections());
				}
				if (network->wrapperFromServer_.has_leave_game_room()) {
					network->handleLeaveGameRoom(network->wrapperFromServer_.leave_game_room());
				}
				if (network->wrapperFromServer_.has_create_game()) {
					network->handleCreateGame(network->wrapperFromServer_.create_game());
					break;
				}
			} while (network->gameRoomId_);

			spdlog::debug("[Network] Game started GameRoomId {}", network->gameRoomId_);

			// Game loop
			while (network->gameRoomId_) {
				co_await nextMessage(network);
				if (network->wrapperFromServer_.has_game_command()) {
					network->handleGameCommand(network->wrapperFromServer_.game_command());
				}
				if (network->wrapperFromServer_.has_request_game_restart()) {
					network->handleRequestGameRestart(network->wrapperFromServer_.request_game_restart());
				}
				if (network->wrapperFromServer_.has_game_restart()) {
					network->handleGameRestart(network->wrapperFromServer_.game_restart());
				}
				if (network->wrapperFromServer_.has_board_move()) {
					network->handleBoardMove(network->wrapperFromServer_.board_move());
				}
				if (network->wrapperFromServer_.has_next_block()) {
					network->handleBoardNextBlock(network->wrapperFromServer_.next_block());
				}
				if (network->wrapperFromServer_.has_board_external_squares()) {
					network->handleBoardExternalSquares(network->wrapperFromServer_.board_external_squares());
				}
				if (network->wrapperFromServer_.has_client_disconnected()) {
					network->handleClientDisconnected(network->wrapperFromServer_.client_disconnected());
				}
				if (network->wrapperFromServer_.has_remove_client()) {
					network->handleRemoveClient(network->wrapperFromServer_.remove_client());
				}
				if (network->wrapperFromServer_.has_leave_game_room()) {
					network->handleLeaveGameRoom(network->wrapperFromServer_.leave_game_room());
				}
			}
		}
		co_return;
	} catch (const std::system_error& e) {
		spdlog::warn("[Network] Lost connection: {}", e.what());
		
		if (network->running_) {
			network->reconnect();
		}
		network->networkEvent(NetworkErrorEvent{
			.insideGameRoom = network->isInsideGameRoom()
		});
		network->gameRoomId_ = GameRoomId{}; // Room destroyed.
		network->networkSlots_.clear();
		network->aiBySlotIndex_.clear();
		network->connections_.clear();
		network->players_.clear();
	}

	void Network::reconnect() {
		spdlog::info("[Network] Reconnect");
		client_->reconnect();
		start();
	}

	asio::awaitable<void> Network::nextMessage(std::shared_ptr<Network> network) {
		bool valid = false;
		do {
			ProtobufMessage message = co_await network->client_->receive();
			valid = message.getSize() > 0;
			if (valid) {
				network->wrapperFromServer_.Clear();
				valid = message.parseBodyInto(network->wrapperFromServer_);
				if (message.getSize() == 0) {
					spdlog::warn("[Network] Invalid data");
				}
				network->client_->release(std::move(message));
			}
			if (!network->running_) {
				// Want to shut down without handling any more messages.
				network->wrapperFromServer_.Clear();
				break;
			}
		} while (!valid);
		co_return;
	}

	void Network::handleRequestGameRestart(const tp_s2c::RequestGameRestart& requestGameRestart) {
		spdlog::info("[Network] RequestGameRestart");
		if (std::none_of(players_.begin(), players_.end(), [this](const NetworkPlayer& networkPlayer) {
			return networkPlayer.player->isLocal();
		})) {
			spdlog::debug("[Network] Ignore RequestGameRestart, no local players for client {}", clientId_);
			return;
		}

		wrapperToServer_.Clear();
		auto gameRestartToServer = wrapperToServer_.mutable_game_restart();
		gameRestartToServer->set_current(requestGameRestart.current());
		gameRestartToServer->set_next(requestGameRestart.next());
		send(wrapperToServer_);

		for (auto& networkPlayer : players_) {
			if (networkPlayer.player->isLocal()) {
				networkPlayer.player->updateRestart(static_cast<tetris::BlockType>(requestGameRestart.current()), static_cast<tetris::BlockType>(requestGameRestart.next()));
			}
		}
		networkEvent(RestartEvent{});
	}

	void Network::handleGameRestart(const tp_s2c::GameRestart& gameRestart) {
		auto current = static_cast<tetris::BlockType>(gameRestart.current());
		auto next = static_cast<tetris::BlockType>(gameRestart.next());

		const auto& clientId = gameRestart.client_id();
		if (clientId == clientId_) {
			spdlog::warn("[Network] Client {} received own GameRestart message. Ignoring!", clientId_);
			return;
		}

		for (auto& networkPlayer : players_) {
			if (networkPlayer.clientId == clientId) {
				networkPlayer.player->updateRestart(current, next);
			}
		}
		networkEvent(RestartEvent{});
	}

	void Network::handleGameCommand(const tp_s2c::GameCommand& gameCommand) {
		spdlog::info("[Network] Paused: {}", gameCommand.pause() ? "true" : "false");
		networkEvent(PauseEvent{
			.pause = gameCommand.pause()
		});
	}

	void Network::handleGameRoomJoined(const tp_s2c::GameRoomJoined& gameRoomJoined) {
		spdlog::info("[Network] GameRoomJoined: {}, client id: {}", gameRoomJoined.game_room_id(), gameRoomJoined.client_id());
		gameRoomId_ = gameRoomJoined.game_room_id();
		auto clientId = gameRoomJoined.client_id();
		if (!clientId_) {
			clientId_ = gameRoomJoined.client_id();
		}
		if (clientId_ == clientId) {
			networkEvent(JoinGameRoomEvent{
				.clientId = clientId_,
			});
		}
		networkEvent(GameRoomEvent{
			.gameRoomClients = extractGameRoomClients(gameRoomJoined.game_room_clients())
		});

		handleGameLooby(gameRoomJoined.game_looby());
	}

	void Network::handleGameLooby(const tp_s2c::GameLooby& gameLooby) {
		networkSlots_.clear();
		int index = 0;
		for (const auto& tpSlot : gameLooby.slots()) {
			switch (tpSlot.slot_type()) {
				case tp_s2c::GameLooby_SlotType_REMOTE:
					if (tpSlot.client_id() == clientId_) {
						if (tpSlot.ai()) {
							networkSlots_.emplace_back(game::Ai{.name = tpSlot.name()}, clientId_);
						} else {
							networkSlots_.emplace_back(game::Human{.name = tpSlot.name()}, clientId_);
						}
					} else {
						networkSlots_.emplace_back(game::Remote{
							.name = tpSlot.name(),
							.ai = tpSlot.ai()
						}, clientId_);
					}
					break;
				case tp_s2c::GameLooby_SlotType_OPEN_SLOT:
					networkSlots_.emplace_back(game::OpenSlot{}, ClientId{});
					break;
				case tp_s2c::GameLooby_SlotType_CLOSED_SLOT:
					networkSlots_.emplace_back(game::ClosedSlot{}, ClientId{});
					break;
				default:
					continue;
			}
			networkEvent(PlayerSlotEvent{
				.playerSlot = networkSlots_[index].playerSlot,
				.index = index
			});
			++index;
		}
		index = 0;
	}

	void Network::handleConnections(const tp_s2c::Connections& connections) {
		for (const auto& clientId : connections.client_ids()) {
			spdlog::info("[Network] Connected ClientId: {}", clientId);
		}
	}

	void Network::handleCreateGame(int width, int height, const tp_s2c::CreateGame_Player& tpPlayer, const NetworkSlot& networkSlot) {
		auto current = static_cast<tetris::BlockType>(tpPlayer.current());
		auto next = static_cast<tetris::BlockType>(tpPlayer.next());

		tetris::TetrisBoard tetrisBoard{width, height, current, next};
		game::DefaultPlayerData playerData{
			.level = tpPlayer.level(),
			.points = tpPlayer.points()
		};
		NetworkPlayer networkPlayer;
		if (auto human = std::get_if<game::Human>(&networkSlot.playerSlot); human) {
			networkPlayer = players_.emplace_back(
				NetworkPlayer{
					.player = game::createHumanPlayer(human->device, playerData, std::move(tetrisBoard)),
					.playerId = tpPlayer.player_id(),
					.clientId = clientId_
				}
			);
		} else if (auto ai = std::get_if<game::Ai>(&networkSlot.playerSlot); ai) {
			networkPlayer = players_.emplace_back(
				NetworkPlayer{
					.player = game::createAiPlayer(ai->ai, playerData, std::move(tetrisBoard)),
					.playerId = tpPlayer.player_id(),
					.clientId = clientId_
				}
			);
		} else if (auto remote = std::get_if<game::Remote>(&networkSlot.playerSlot); remote) {
			networkPlayer = players_.emplace_back(
				NetworkPlayer{
					.player = game::createRemotePlayer(playerData, std::move(tetrisBoard)),
					.playerId = tpPlayer.player_id(),
					.clientId = tpPlayer.client_id()
				}
			);
		}
		if (networkPlayer.player && networkPlayer.player->isLocal()) {
			connections_ += networkPlayer.player->playerBoardUpdate.connect([this, index = players_.size() - 1](game::PlayerBoardEvent playerBoardEvent) {
				if (index < 0 || index >= players_.size()) {
					spdlog::error("[Network] Invalid index: {}", index);
					return;
				}
				auto& networkPlayerRef = players_[index];
				std::visit([&](auto&& event) {
					handlePlayerBoardUpdate(networkPlayerRef, event);
				}, playerBoardEvent);
			});
		}
	}

	void Network::handleCreateGame(const tp_s2c::CreateGame& createGame) {
		connections_.clear();
		players_.clear();
		fillSlotsWithDevicesAndAis();

		const auto& tpPlayers = createGame.players();

		int index = 0;
		for (const auto& tpPlayer : createGame.players()) {
			if (index >= networkSlots_.size()) {
				// TODO! Handle error!
				break;
			}
			handleCreateGame(createGame.width(), createGame.height(), tpPlayer, networkSlots_[index]);
			++index;
		}
		game::GameRoomConfigEvent gameRoomConfigEvent;
		if (createGame.has_game_rules()) {
			if (createGame.game_rules().has_default_game_rules()) {
				gameRoomConfigEvent.gameRulesConfig = game::DefaultGameRules::Config{};
			} else if (createGame.game_rules().has_survival_game_rules()) {
				gameRoomConfigEvent.gameRulesConfig = game::SurvivalGameRules::Config{};
			}
		}
		std::vector<game::PlayerPtr> players;
		for (auto& player : players_) {
			players.push_back(player.player);
		}
		networkEvent(CreateGameEvent{
			.players = std::move(players),
			.gameRulesConfig = createGameRulesConfig(createGame)
		});
	}

	void Network::handleBoardMove(const tp_s2c::BoardMove& boardMove) {
		auto move = static_cast<tetris::Move>(boardMove.move());
		for (auto& networkPlayer : players_) {
			if (networkPlayer.playerId == boardMove.player_id()) {
				if (networkPlayer.player->isRemote()) {
					networkPlayer.player->updateMove(move);
				} else {
					spdlog::error("[Network] Invalid player type for BoardMove");
				}
			}
		}
	}

	void Network::handleBoardNextBlock(const tp_s2c::BoardNextBlock& boardNextBlock) {
		for (auto& networkPlayer : players_) {
			if (networkPlayer.playerId == boardNextBlock.player_id()) {
				if (networkPlayer.player->isRemote()) {
					auto next = static_cast<tetris::BlockType>(boardNextBlock.next());
					networkPlayer.player->updateNextBlock(next);
				} else {
					spdlog::error("[Network] Invalid player type for BoardNextBlock");
				}
			}
		}
	}

	void Network::handleBoardExternalSquares(const tp_s2c::BoardExternalSquares& boardExternalSquares) {
		for (auto& networkPlayer : players_) {
			if (networkPlayer.playerId == boardExternalSquares.player_id()) {
				if (networkPlayer.player->isRemote()) {
					std::vector<tetris::BlockType> blockTypes;
					for (const auto& blockType : boardExternalSquares.block_types()) {
						blockTypes.push_back(static_cast<tetris::BlockType>(blockType));
					}
					networkPlayer.player->addExternalRows(blockTypes);
				} else {
					spdlog::error("[Network] Invalid player type for BoardExternalSquares");
				}
			}
		}
	}

	void Network::handleClientDisconnected(const tp_s2c::ClientDisconnected& clientDisconnected) {
		spdlog::info("[Network] ClientDisconnected: {}", clientDisconnected.client_id());
		int nbr = 0;
		networkEvent(ClientDisconnectedEvent{
			.clientId = clientDisconnected.client_id()
		});
	}

	void Network::handleRemoveClient(const tp_s2c::RemoveClient& removeClient) {
		spdlog::info("[Network] RemoveClient: {}", removeClient.client_id());

		players_.erase(std::remove_if(players_.begin(), players_.end(), [removeClient](const NetworkPlayer& networkPlayer) {
			networkPlayer.player->updateMove(tetris::Move::GameOver);
			return networkPlayer.clientId == removeClient.client_id();
		}), players_.end());
	}

	void Network::fillSlotsWithDevicesAndAis() {
		for (int i = 0; i < networkSlots_.size(); ++i) {
			if (auto human = std::get_if<game::Human>(&networkSlots_[i].playerSlot); human) {
				human->device = deviceBySlotIndex_[i];
			} else if (auto ai = std::get_if<game::Ai>(&networkSlots_[i].playerSlot); ai) {
				ai->ai = aiBySlotIndex_[i];
			}
		}
	}


	void Network::handlePlayerBoardUpdate(const NetworkPlayer& player, const game::UpdateRestart& updateRestart) {
		spdlog::info("[Network] handle UpdateRestart: current={}, next={}", static_cast<char>(updateRestart.current), static_cast<char>(updateRestart.next));
	}

	void Network::handlePlayerBoardUpdate(const NetworkPlayer& player, const game::UpdatePlayerData& updatePlayerData) {
		spdlog::info("[Network] handle UpdatePlayerData");
	}

	void Network::handlePlayerBoardUpdate(const NetworkPlayer& player, const game::ExternalRows& externalRows) {
		if (player.player->isLocal()) {
			spdlog::info("[Network] handle ExternalRows");
			wrapperToServer_.Clear();
			auto boardExternalSquares = wrapperToServer_.mutable_board_external_squares();
			for (const auto& blockType : externalRows.blockTypes) {
				boardExternalSquares->add_block_types(static_cast<tp::BlockType>(blockType));
			}
			fromCppToProto(player.playerId, *boardExternalSquares->mutable_player_id());
			send(wrapperToServer_);
		}
	}

	void Network::handlePlayerBoardUpdate(const NetworkPlayer& player, const game::UpdateMove& updateMove) {
		wrapperToServer_.Clear();
		auto boardMove = wrapperToServer_.mutable_board_move();
		boardMove->set_move(static_cast<tp::Move>(updateMove.move));
		fromCppToProto(player.playerId, *boardMove->mutable_player_id());
		send(wrapperToServer_);
	}

	void Network::handlePlayerBoardUpdate(const NetworkPlayer& player, const game::UpdateNextBlock& updateNextBlock) {
		wrapperToServer_.Clear();
		auto nextBlock = wrapperToServer_.mutable_next_block();
		fromCppToProto(player.playerId, *nextBlock->mutable_player_id());
		nextBlock->set_next(static_cast<tp::BlockType>(updateNextBlock.next));

		send(wrapperToServer_);
	}

	void Network::handlePlayerBoardUpdate(const NetworkPlayer& player, const game::TetrisBoardEvent& tetrisBoardEvent) {
		//spdlog::info("[Network] handle TetrisBoardEvent");
	}

	void Network::handleLeaveGameRoom(const tp_s2c::LeaveGameRoom& leaveGameRoom) {
		spdlog::info("[Network] LeaveGameRoom: {}", leaveGameRoom.game_room_id());
		if (leaveGameRoom.client_id() == clientId_) {
			gameRoomId_ = GameRoomId{};
			networkEvent(LeaveGameRoomEvent{
				.clientId = leaveGameRoom.client_id(),
			});
		}
		networkEvent(GameRoomEvent{
			.gameRoomClients = extractGameRoomClients(leaveGameRoom.game_room_clients())
		});
	}

	void Network::handleGameRoomList(const tp_s2c::GameRoomList& gameRoomList) {
		spdlog::info("[Network] GameRoomList");
		std::vector<GameRoomListEvent::GameRoom> gameRooms;
		for (const auto& tpGameRoom : gameRoomList.game_rooms()) {
			gameRooms.push_back(GameRoomListEvent::GameRoom{
				.id = tpGameRoom.game_room_id(),
				.name = tpGameRoom.name(),
				.playerCount = tpGameRoom.player_count(),
				.maxPlayerCount = tpGameRoom.max_player_count()
			});
		}
		networkEvent(GameRoomListEvent{
			.gameRooms = std::move(gameRooms)
		});
	}

	void Network::send(const tp_c2s::Wrapper& wrapper) {
		ProtobufMessage message;
		client_->acquire(message);
		message.setBuffer(wrapper);
		client_->send(std::move(message));
	}

}
