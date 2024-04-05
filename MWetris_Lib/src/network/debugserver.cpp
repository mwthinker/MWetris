#include "debugserver.h"
#include "debugclient.h"
#include "protobufmessagequeue.h"
#include "util.h"
#include "game/player.h"
#include "game/remoteplayer.h"
#include "protocol.h"

#include <helper.h>

#include <server_to_client.pb.h>
#include <client_to_server.pb.h>

#include <spdlog/spdlog.h>

#include <queue>
#include <random>

namespace mwetris::network {

	namespace {

		constexpr std::string_view Characters = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

		std::string generateUuid() {
			static std::mt19937 generator{std::random_device{}()};
			static std::uniform_int_distribution<> distribution{0, static_cast<int>(Characters.size() - 1)};

			constexpr int UniqueIdSize = 16;
			std::string unique(UniqueIdSize, 'X');

			for (auto& key : unique) {
				key = Characters[distribution(generator)];
			}

			return unique;
		}

	}

	class DebugServer::Impl {
	public:
		mw::PublicSignal<DebugServer::Impl, const std::vector<game::PlayerSlot>&> playerSlotsUpdated;
		mw::PublicSignal<DebugServer::Impl, const std::vector<game::RemotePlayerPtr>&> gameCreated;
		mw::PublicSignal<DebugServer::Impl, const game::InitGameEvent&> initGameEvent;
		mw::PublicSignal<DebugServer::Impl, const ConnectedClient&> connectedClientListener;

		Impl() {
			playerSlots_.resize(4, game::OpenSlot{});
		}

		~Impl() {}

		void update(const sdl::DeltaTime& deltaTime) {
			for (auto& remote : remotes_) {
				receivedFromClient(*remote.client);
			}
		}

		void receivedFromClient(DebugClient& client) {
			ProtobufMessage message;
			while (client.pollSentMessage(message)) {
				wrapperFromClient_.Clear();
				bool valid = wrapperFromClient_.ParseFromArray(message.getBodyData(), message.getBodySize());
				if (valid) {
					receivedFromClient(client, wrapperFromClient_);
				} else {
					spdlog::error("Protocol error");
				}

				messageQueue_.release(std::move(message));
			}
		}

		void receivedFromClient(DebugClient& client, const tp_c2s::Wrapper& wrapper) {
			for (auto& remote : remotes_) {
				if (remote.client.get() == &client) {
					if (!remote.allowToConnect) {
						sendFailedToConnect(*remote.client);
						return;
					}
				}
			}

			if (wrapperFromClient_.has_connect()) {
				handleConnect(client, wrapperFromClient_.connect());
			}
			if (wrapperFromClient_.has_game_looby()) {
				handleGameLooby(client, wrapperFromClient_.game_looby());
			}
			if (wrapperFromClient_.has_player_slot()) {
				handlePlayerSlot(client, wrapperFromClient_.player_slot());
			}
			if (wrapperFromClient_.has_game_command()) {
				handleGameCommand(wrapperFromClient_.game_command());
			}
			if (wrapperFromClient_.has_start_game()) {
				handleStartGame(wrapperFromClient_.start_game());
			}
			if (wrapperFromClient_.has_board_move()) {
				handleBoardMove(wrapperFromClient_.board_move());
			}
			if (wrapperFromClient_.has_next_block()) {
				handleBoardNextBlock(wrapperFromClient_.next_block());
			}
			if (wrapperFromClient_.has_board_external_squares()) {
				handleBoardExternalSquares(wrapperFromClient_.board_external_squares());
			}
			if (wrapperFromClient_.has_game_restart()) {
				handleGameRestart(wrapperFromClient_.game_restart());
			}
			playerSlotsUpdated(playerSlots_);
		}

		void handleConnect(DebugClient& client, const tp_c2s::Connect& connect) {
			wrapperToClient_.Clear();

			if (remotePlayers_.contains(client.getUuid())) {
				spdlog::warn("[DebugServer] Client with uuid {} already connect", client.getUuid());
				return;
			}

			auto uuid = generateUuid();
			client.setUuid(uuid);
			wrapperToClient_.mutable_connected()->set_uuid(uuid);
			sendToClient(client, wrapperToClient_);
		}

		void handleGameRestart(const tp_c2s::GameRestart& gameRestart) {
			auto current = static_cast<tetris::BlockType>(gameRestart.current());
			auto next = static_cast<tetris::BlockType>(gameRestart.next());
			for (auto& [_, player] : remotePlayers_) {
				player->updateRestart(current, next);
			}
		}

		void handleBoardMove(const tp_c2s::BoardMove& boardMove) {
			auto move = static_cast<tetris::Move>(boardMove.move());
			remotePlayers_[boardMove.uuid()]->updateMove(move);
		}

		void handleBoardNextBlock(const tp_c2s::BoardNextBlock& boardNextBlock) {
			auto blockType = static_cast<tetris::BlockType>(boardNextBlock.next());
			remotePlayers_[boardNextBlock.uuid()]->updateNextBlock(blockType);
		}

		void handleBoardExternalSquares(const tp_c2s::BoardExternalSquares& boardExternalSquares) {
			auto& remotePlayer = remotePlayers_[boardExternalSquares.uuid()];
			static std::vector<tetris::BlockType> blockTypes;
			blockTypes.clear();
			for (const auto& tpBlockType : boardExternalSquares.block_types()) {
				blockTypes.push_back(static_cast<tetris::BlockType>(tpBlockType));
			}
			remotePlayer->updateAddExternalRows(blockTypes);
		}

		void handleStartGame(const tp_c2s::StartGame& createServerGame) {
			remotePlayers_.clear();
			wrapperToClient_.Clear();
			auto createGame = wrapperToClient_.mutable_create_game();
			createGame->set_width(10);
			createGame->set_height(24);

			auto current = tetris::randomBlockType();
			auto next = tetris::randomBlockType();

			remotePlayers_.clear();
			std::vector<game::PlayerBoardPtr> playerBoards;
			for (const auto& slot : playerSlots_) {
				if (auto remote = std::get_if<game::Remote>(&slot)) {
					auto remotePlayer = std::make_shared<game::RemotePlayer>(current, next, remote->uuid);
					remotePlayers_[remote->uuid] = remotePlayer;
					playerBoards.push_back(remotePlayer->getPlayerBoard());
					auto tpRemotePlayer = createGame->add_players();
					tpRemotePlayer->set_uuid(remote->uuid);
					tpRemotePlayer->set_name(remote->name);
					tpRemotePlayer->set_level(0);
					tpRemotePlayer->set_points(0);
					tpRemotePlayer->set_ai(remote->ai);
					tpRemotePlayer->set_current(static_cast<tp::BlockType>(current));
					tpRemotePlayer->set_next(static_cast<tp::BlockType>(next));
				}
			}
			initGameEvent(game::InitGameEvent{playerBoards.begin(), playerBoards.end()});
			sendToClients(wrapperToClient_);
		}

		void handleGameCommand(const tp_c2s::GameCommand& gameCommand) {
			paused_ = gameCommand.pause();
		}

		bool slotBelongsToClient(const Client& client, int slotIndex) {
			const auto& slot = playerSlots_[slotIndex];
			if (auto remote = std::get_if<game::Remote>(&slot); remote && remote->uuid == client.getUuid()) {
				return true;
			}
			return std::holds_alternative<game::OpenSlot>(slot);
		}

		void handleGameLooby(Client& client, const tp_c2s::GameLooby& gameLooby) {
			//remotePlayers_[client.getUuid()]->
		}

		void handlePlayerSlot(Client& client, const tp_c2s::PlayerSlot& gameLooby) {
			int index = wrapperFromClient_.player_slot().index();
			const auto& slot = wrapperFromClient_.player_slot();
			
			if (index < 0 || index >= playerSlots_.size()) {
				spdlog::error("Invalid index {}", index);
				return;
			}

			if (slotBelongsToClient(client, index)) {
				if (slot.slot_type() == tp_c2s::PlayerSlot_SlotType_OPEN_SLOT) {
					playerSlots_[index] = game::OpenSlot{};
				} else {
					playerSlots_[index] = game::Remote{
						.name = slot.name(),
						.ai = slot.slot_type() == tp_c2s::PlayerSlot_SlotType_AI,
						.uuid = client.getUuid()
					};
				}
			}
			
			wrapperToClient_.Clear();
			auto tpGameLooby = wrapperToClient_.mutable_game_looby();

			for (const auto& playerSlot : playerSlots_) {
				auto& tpSlot = *tpGameLooby->add_slots();
				tpSlot.set_slot_type(tp_s2c::GameLooby_SlotType_UNSPECIFIED_SLOT_TYPE);

				std::visit([&](auto&& slot) mutable {
					using T = std::decay_t<decltype(slot)>;
					if constexpr (std::is_same_v<T, game::Remote>) {
						tpSlot.set_slot_type(tp_s2c::GameLooby_SlotType_REMOTE);
						tpSlot.set_ai(slot.ai);
						tpSlot.set_name(slot.name);
						tpSlot.set_uuid(slot.uuid);
					} else if constexpr (std::is_same_v<T, game::OpenSlot>) {
						tpSlot.set_slot_type(tp_s2c::GameLooby_SlotType_OPEN_SLOT);
					} else if constexpr (std::is_same_v<T, game::ClosedSlot>) {
						tpSlot.set_slot_type(tp_s2c::GameLooby_SlotType_CLOSED_SLOT);
					} else {
						spdlog::error("[DebugServer.cpp] Invalid slot type");
					}
				}, playerSlot);
			}
			sendToClients(wrapperToClient_);
		}

		void release(ProtobufMessage&& message) {
			messageQueue_.release(std::move(message));
		}

		void connect(const std::string& uuid) {
			wrapperToClient_.Clear();
			connectedUuids_.push_back(uuid);
			for (const auto& connected : connectedUuids_) {
				auto uuidTp = wrapperToClient_.mutable_connections()->add_uuids();
				uuidTp->assign(uuid);
			}
			sendToClients(wrapperToClient_);
		}

		void disconnect(const std::string& uuid) {
			wrapperToClient_.Clear();
			connectedUuids_.push_back(uuid);

			if (std::erase_if(connectedUuids_, [&uuid](const std::string& value) {
				return value == uuid;
			})) {
				for (const auto& connected : connectedUuids_) {
					auto uuidTp = wrapperToClient_.mutable_connections()->add_uuids();
					uuidTp->assign(uuid);
				}
				sendToClients(wrapperToClient_);
			} else {
				spdlog::warn("[DebugServer] Failed to disconnect uuid {} (does not match existing)", uuid);
			}
		}

		void handle() {}

		void acquire(ProtobufMessage& message) {
			messageQueue_.acquire(message);
		}

		void sendFailedToConnect(DebugClient& client) {
			wrapperToClient_.Clear();
			wrapperToClient_.mutable_failed_to_connect();
			sendToClient(client, wrapperToClient_);
		}

		void sendPause(bool pause) {
			paused_ = pause;
			wrapperToClient_.Clear();
			wrapperToClient_.mutable_game_command()->set_pause(paused_);
			sendToClients(wrapperToClient_);
		}

		void restartGame() {
			wrapperToClient_.Clear();
			auto gameRestart = wrapperToClient_.mutable_game_restart();
			gameRestart->set_current(static_cast<tp::BlockType>(tetris::randomBlockType()));
			gameRestart->set_next(static_cast<tp::BlockType>(tetris::randomBlockType()));
			sendToClients(wrapperToClient_);
		}

		bool isPaused() const {
			return paused_;
		}

		std::shared_ptr<Client> createClient(std::shared_ptr<DebugServer> debugServer) {
			auto client = std::make_shared<DebugClient>(debugServer);
			const auto& remote = remotes_.emplace_back(Remote{
				.allowToConnect = true,
				.connected = false,
				.type = RemoteType::Connected,
				.client = client
			});
			triggerConnectedClient(remote);
			return client;
		}

		std::vector<ConnectedClient> getConnectedClients() const {
			std::vector<ConnectedClient> connectedClients;
			for (const Remote& remote : remotes_) {
				connectedClients.push_back(convertToConnectedClient(remote));
			}
			return connectedClients;
		}

		void allowClientToConnect(const std::string& uuid, bool allowed) {
			for (auto& remote : remotes_) {
				if (remote.client->getUuid() == uuid) {
					remote.allowToConnect = allowed;
					triggerConnectedClient(remote);
					break;
				}
			}
		}

	private:
		enum class RemoteType {
			Spectator,
			Server,
			Client,
			Connected
		};

		struct Remote {
			bool allowToConnect;
			bool connected;
			RemoteType type;
			std::shared_ptr<DebugClient> client;
		};

		ConnectedClient convertToConnectedClient(const Remote& remote) const {
			return ConnectedClient{
				.uuid = remote.client->getUuid(),
				.connected = remote.connected,
				.allowToConnect = remote.allowToConnect
			};
		}

		void triggerConnectedClient(const Remote& remote) {
			connectedClientListener(convertToConnectedClient(remote));
		}

		void sendToClients(const tp_s2c::Wrapper& wrapper) {
			for (auto& remote : remotes_) {
				sendToClient(*remote.client, wrapper);
			}
		}

		void sendToClient(DebugClient& client, const tp_s2c::Wrapper& wrapper) {
			ProtobufMessage message;
			messageQueue_.acquire(message);
			message.setBuffer(wrapper);
			client.pushReceivedMessage(std::move(message));
		}

		std::map<std::string, game::RemotePlayerPtr> remotePlayers_;
		std::vector<game::PlayerSlot> playerSlots_;
		std::vector<std::string> connectedUuids_;
		bool paused_ = false;

		tp_c2s::Wrapper wrapperFromClient_;
		tp_s2c::Wrapper wrapperToClient_;
		ProtobufMessageQueue messageQueue_;
		std::vector<Remote> remotes_;
	};

	void DebugServer::update(const sdl::DeltaTime& deltaTime) {
		impl_->update(deltaTime);
	}

	std::shared_ptr<Client> DebugServer::createClient() {
		return impl_->createClient(shared_from_this());
	}

	void DebugServer::disconnect(const std::string& uuid) {
		impl_->disconnect(uuid);
	}

	DebugServer::DebugServer()
		: impl_{std::make_unique<DebugServer::Impl>()} {}

	DebugServer::~DebugServer() {}

	mw::signals::Connection DebugServer::addPlayerSlotsCallback(const std::function<void(const std::vector<game::PlayerSlot>&)>& callback) {
		return impl_->playerSlotsUpdated.connect(callback);
	}

	mw::signals::Connection DebugServer::addInitGameCallback(const std::function<void(const game::InitGameEvent&)>& callback) {
		return impl_->initGameEvent.connect(callback);
	}

	void DebugServer::acquire(ProtobufMessage& message) {
		impl_->acquire(message);
	}

	void DebugServer::release(ProtobufMessage&& message) {
		impl_->release(std::move(message));
	}

	void DebugServer::sendPause(bool pause) {
		impl_->sendPause(pause);
	}

	bool DebugServer::isPaused() const {
		return impl_->isPaused();
	}

	void DebugServer::restartGame() {
		impl_->restartGame();
	}

	std::vector<ConnectedClient> DebugServer::getConnectedClients() const {
		return impl_->getConnectedClients();
	}

	void DebugServer::allowClientToConnect(const std::string& uuid, bool allow) {
		impl_->allowClientToConnect(uuid, allow);
	}

	mw::signals::Connection DebugServer::addClientListener(const std::function<void(const ConnectedClient&)>& callback) {
		return impl_->connectedClientListener.connect(callback);
	}

}
