#include "debugserver.h"
#include "debugclient.h"
#include "protobufmessagequeue.h"
#include "util.h"
#include "util/uuid.h"
#include "game/player.h"
#include "gameroom.h"
#include "server.h"
#include "util/uuid.h"
#include "id.h"

#include <helper.h>

#include <server_to_client.pb.h>
#include <client_to_server.pb.h>

#include <spdlog/spdlog.h>

#include <queue>
#include <variant>

namespace mwetris::network {

	struct Remote {
		std::shared_ptr<DebugClient> client;
		ClientId clientId;
	};

	class DebugServer::Impl : public Server {
	public:
		mw::PublicSignal<DebugServer::Impl, const std::vector<Slot>&> playerSlotsUpdated;
		mw::PublicSignal<DebugServer::Impl, const std::vector<game::PlayerPtr>&> gameCreated;
		mw::PublicSignal<DebugServer::Impl, const game::InitGameEvent&> initGameEvent;
		mw::PublicSignal<DebugServer::Impl, const ConnectedClient&> connectedClientListener;

		Impl() {}
		~Impl() {}

		void update(const sdl::DeltaTime& deltaTime) {
			for (auto& remote : remotes_) {
				receivedFromClient(remote);
			}
		}

		void receivedFromClient(Remote& remote) {
			ProtobufMessage message;
			while (remote.client->pollSentMessage(message)) {
				wrapperFromClient_.Clear();
				bool valid = wrapperFromClient_.ParseFromArray(message.getBodyData(), message.getBodySize());
				if (valid) {
					receivedFromRemote(remote, wrapperFromClient_);
				} else {
					spdlog::error("Protocol error");
				}

				messageQueue_.release(std::move(message));
			}
		}

		void receivedFromRemote(Remote& fromRemote, const tp_c2s::Wrapper& wrapper) {
			wrapperToClient_.Clear();

			if (wrapperFromClient_.has_create_game_room()) {
				handleCreateGameRoom(fromRemote, wrapperFromClient_.create_game_room());
			} else if (wrapperFromClient_.has_join_game_room()) {
				handleJoinGameRoom(fromRemote, wrapperFromClient_.join_game_room());
			} else if (wrapperFromClient_.has_leave_game_room()) {
				handleLeaveGameRoom(fromRemote, wrapperFromClient_.leave_game_room());
			}

			if (auto it = roomIdByClientId_.find(fromRemote.clientId); it != roomIdByClientId_.end()) {
				auto& gameRoom = gameRoomById_.at(roomIdByClientId_.at(fromRemote.clientId));
				gameRoom.receiveMessage(*this, fromRemote.clientId, wrapper);
			}
		}

		void handleCreateGameRoom(Remote& remote, const tp_c2s::CreateGameRoom& createGameRoom) {
			if (roomIdByClientId_.contains(remote.clientId)) {
				spdlog::warn("[DebugServer] Client with uuid {} already in a GameRoom", remote.clientId);
				return;
			}

			GameRoom gameRoom;
			roomIdByClientId_.emplace(remote.clientId, gameRoom.getGameRoomId());
			gameRoomById_.emplace(gameRoom.getGameRoomId(), std::move(gameRoom));
		}

		void handleJoinGameRoom(Remote& remote, const tp_c2s::JoinGameRoom& joinGameRoom) {
			if (roomIdByClientId_.contains(remote.clientId)) {
				spdlog::warn("[DebugServer] Client with uuid {} already in a GameRoom", remote.clientId);
				return;
			}

			if (auto it = gameRoomById_.find(joinGameRoom.game_room_id()); it != gameRoomById_.end()) {
				auto& gameRoom = it->second;
				roomIdByClientId_.emplace(remote.clientId, gameRoom.getGameRoomId());
				gameRoomById_.emplace(gameRoom.getGameRoomId(), std::move(gameRoom));
			} else {
				spdlog::warn("[DebugServer] GameRoom with uuid {} not found", joinGameRoom.game_room_id());
			}
		}

		void handleLeaveGameRoom(Remote& remote, const tp_c2s::LeaveGameRoom& leaveGameRoom) {
			if (auto it = roomIdByClientId_.find(remote.clientId); it != roomIdByClientId_.end()) {
				auto gameRoomId = it->second;
				auto& gameRoom = gameRoomById_.at(gameRoomId);

				roomIdByClientId_.erase(it);
				if (gameRoom.getConnectedClientUuids().size() == 1) {
					gameRoom.disconnect(*this);
					gameRoomById_.erase(gameRoomId);
					spdlog::info("[DebugServer] GameRoom with uuid {} is removed", gameRoomId);
				}
			}
		}

		void release(ProtobufMessage&& message) {
			messageQueue_.release(std::move(message));
		}

		void acquire(ProtobufMessage& message) {
			messageQueue_.acquire(message);
		}

		void sendFailedToConnect(DebugClient& client) {
			wrapperToClient_.Clear();
			wrapperToClient_.mutable_failed_to_connect();
			sendToClient(client, wrapperToClient_);
		}

		void sendPause(const GameRoomId& gameRoomId, bool pause) {
			gameRoomById_[gameRoomId].sendPause(*this, pause);
		}

		void restartGame(const GameRoomId& gameRoomId) {
			gameRoomById_[gameRoomId].requestRestartGame(*this);
		}

		bool isPaused(const GameRoomId& gameRoomId) const {
			return gameRoomById_.at(gameRoomId).isPaused();
		}

		void disconnect(const GameRoomId& gameRoomId) {
			gameRoomById_.erase(gameRoomId);
		}

		std::shared_ptr<Client> createDisconnectedClient(std::shared_ptr<DebugServer> debugServer) {
			auto client = std::make_shared<DebugClient>(debugServer);
			const auto& remote = remotes_.emplace_back(Remote{
				.client = client,
				.clientId = ClientId::generateUniqueId()
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

		void sendToClient(const ClientId& clientId, const google::protobuf::MessageLite& message) override {
			for (auto& remote : remotes_) {
				if (remote.clientId == clientId) {
					sendToClient(*remote.client, message);
					break;
				}
			}
		}

		void triggerConnectedClientEvent(const ConnectedClient& connectedClient) override {
			connectedClientListener(connectedClient);
		}

		void triggerPlayerSlotEvent(const std::vector<Slot>& slots) override {
			playerSlotsUpdated(slots);
		}

	private:
		ConnectedClient convertToConnectedClient(const Remote& remote) const {
			return ConnectedClient{
				.clientId = remote.clientId
			};
		}

		void triggerConnectedClient(const Remote& remote) {
			connectedClientListener(convertToConnectedClient(remote));
		}

		void sendToClients(const google::protobuf::MessageLite& wrapper) {
			for (auto& remote : remotes_) {
				sendToClient(*remote.client, wrapper);
			}
		}

		void sendToClient(DebugClient& client, const google::protobuf::MessageLite& wrapper) {
			ProtobufMessage message;
			messageQueue_.acquire(message);
			message.setBuffer(wrapper);
			client.pushReceivedMessage(std::move(message));
		}

		std::map<ClientId, GameRoomId> roomIdByClientId_;
		std::map<GameRoomId, GameRoom> gameRoomById_;

		tp_c2s::Wrapper wrapperFromClient_;
		tp_s2c::Wrapper wrapperToClient_;
		ProtobufMessageQueue messageQueue_;
		std::vector<Remote> remotes_;
	};

	void DebugServer::update(const sdl::DeltaTime& deltaTime) {
		impl_->update(deltaTime);
	}

	std::shared_ptr<Client> DebugServer::createClient() {
		return impl_->createDisconnectedClient(shared_from_this());
	}

	void DebugServer::disconnect(const GameRoomId& gameRoomId) {
		impl_->disconnect(gameRoomId);
	}

	DebugServer::DebugServer()
		: impl_{std::make_unique<DebugServer::Impl>()} {}

	DebugServer::~DebugServer() {}

	mw::signals::Connection DebugServer::addPlayerSlotsCallback(const std::function<void(const std::vector<Slot>&)>& callback) {
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

	void DebugServer::sendPause(const GameRoomId& gameRoomId, bool pause) {
		impl_->sendPause(gameRoomId, pause);
	}

	bool DebugServer::isPaused(const GameRoomId& gameRoomId) const {
		return impl_->isPaused(gameRoomId);
	}

	void DebugServer::restartGame(const GameRoomId& gameRoomId) {
		impl_->restartGame(gameRoomId);
	}

	std::vector<ConnectedClient> DebugServer::getConnectedClients() const {
		return impl_->getConnectedClients();
	}

	mw::signals::Connection DebugServer::addClientListener(const std::function<void(const ConnectedClient&)>& callback) {
		return impl_->connectedClientListener.connect(callback);
	}

}
