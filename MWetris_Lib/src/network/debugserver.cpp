#include "debugserver.h"
#include "debugclient.h"
#include "protobufmessagequeue.h"
#include "util.h"
#include "util/uuid.h"
#include "game/player.h"
#include "game/remoteplayer.h"
#include "protocol.h"
#include "gameroom.h"
#include "server.h"
#include "util/uuid.h"

#include <helper.h>

#include <server_to_client.pb.h>
#include <client_to_server.pb.h>

#include <spdlog/spdlog.h>

#include <queue>
#include <variant>

namespace mwetris::network {

	struct Remote {
		std::shared_ptr<DebugClient> client;
		std::string uuid;
	};

	class DebugServer::Impl : public Server {
	public:
		mw::PublicSignal<DebugServer::Impl, const std::vector<Slot>&> playerSlotsUpdated;
		mw::PublicSignal<DebugServer::Impl, const std::vector<game::RemotePlayerPtr>&> gameCreated;
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
			}

			if (auto it = gameRoomUuidByClientUuid_.find(fromRemote.uuid); it != gameRoomUuidByClientUuid_.end()) {
				auto& gameRoom = gameRooms_.at(gameRoomUuidByClientUuid_.at(fromRemote.uuid));
				gameRoom.receiveMessage(*this, fromRemote.uuid, wrapper);
			}
		}

		void handleCreateGameRoom(Remote& remote, const tp_c2s::CreateGameRoom& createGameRoom) {
			if (gameRoomUuidByClientUuid_.contains(remote.uuid)) {
				spdlog::warn("[DebugServer] Client with uuid {} already in a GameRoom", remote.uuid);
				return;
			}

			GameRoom gameRoom;
			gameRoomUuidByClientUuid_.emplace(remote.uuid, gameRoom.getUuid());
			gameRoom.receiveMessage(*this, remote.uuid, wrapperFromClient_);
			
			gameRooms_.emplace(gameRoom.getUuid(), std::move(gameRoom));
		}

		void handleJoinGameRoom(Remote& remote, const tp_c2s::JoinGameRoom& joinGameRoom) {
			if (gameRoomUuidByClientUuid_.contains(remote.uuid)) {
				spdlog::warn("[DebugServer] Client with uuid {} already in a GameRoom", remote.uuid);
				return;
			}

			if (auto it = gameRooms_.find(joinGameRoom.server_uuid()); it != gameRooms_.end()) {
				auto& gameRoom = it->second;
				gameRoomUuidByClientUuid_.emplace(remote.uuid, gameRoom.getUuid());
				gameRooms_.emplace(gameRoom.getUuid(), std::move(gameRoom));

				gameRoom.receiveMessage(*this, remote.uuid, wrapperFromClient_);
			} else {
				spdlog::warn("[DebugServer] GameRoom with uuid {} not found", joinGameRoom.server_uuid());
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

		void sendPause(const std::string& clientUuid, bool pause) {
			gameRooms_[clientUuid].sendPause(*this, pause);
		}

		void restartGame(const std::string& clientUuid) {
			gameRooms_[clientUuid].restartGame(*this);
		}

		bool isPaused(const std::string& clientUuid) const {
			return gameRooms_.at(clientUuid).isPaused();
		}

		void disconnect(const std::string& uuid) {
			gameRooms_.erase(uuid);
		}

		std::shared_ptr<Client> createDisconnectedClient(std::shared_ptr<DebugServer> debugServer) {
			auto client = std::make_shared<DebugClient>(debugServer);
			const auto& remote = remotes_.emplace_back(Remote{
				.client = client,
				.uuid = util::generateUuid()
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

		void sendToClient(const std::string& clientUuid, const google::protobuf::MessageLite& message) override {
			for (auto& remote : remotes_) {
				if (remote.uuid == clientUuid) {
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

		void triggerInitGameEvent(const game::InitGameEvent& initGame) override {
			initGameEvent(initGame);
		}

	private:
		ConnectedClient convertToConnectedClient(const Remote& remote) const {
			return ConnectedClient{
				.uuid = remote.uuid
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

		std::map<std::string, std::string> gameRoomUuidByClientUuid_;
		std::map<std::string, GameRoom> gameRooms_;

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

	void DebugServer::disconnect(const std::string& uuid) {
		impl_->disconnect(uuid);
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

	void DebugServer::sendPause(const std::string& clientUuuid, bool pause) {
		impl_->sendPause(clientUuuid, pause);
	}

	bool DebugServer::isPaused(const std::string& clientUuuid) const {
		return impl_->isPaused(clientUuuid);
	}

	void DebugServer::restartGame(const std::string& clientUuuid) {
		impl_->restartGame(clientUuuid);
	}

	std::vector<ConnectedClient> DebugServer::getConnectedClients() const {
		return impl_->getConnectedClients();
	}

	mw::signals::Connection DebugServer::addClientListener(const std::function<void(const ConnectedClient&)>& callback) {
		return impl_->connectedClientListener.connect(callback);
	}

}
