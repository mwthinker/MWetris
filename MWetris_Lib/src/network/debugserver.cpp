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

#include <helper.h>

#include <server_to_client.pb.h>
#include <client_to_server.pb.h>

#include <spdlog/spdlog.h>

#include <queue>
#include <variant>

namespace mwetris::network {

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
			wrapperToClient_.Clear();

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
			if (wrapperFromClient_.has_connect_to_game()) {
				handleConnectToGame(wrapperFromClient_.connect_to_game());
			}

			if (auto it = gameRooms_.find(client.getUuid()); it != gameRooms_.end()) {
				auto& gameRoom = it->second;
				gameRoom.receiveMessage(*this, client.getUuid(), wrapper);
			}
		}

		void handleConnectToGame(const tp_c2s::ConnectToGame& connectToGame) {
		}

		void handleConnect(DebugClient& client, const tp_c2s::Connect& connect) {
			if (gameRooms_.contains(client.getUuid())) {
				spdlog::warn("[DebugServer] Client with uuid {} already connect", client.getUuid());
				return;
			}

			GameRoom gameRoom{connect.name()};
			gameRoom.connectMaster(*this);
			client.setUuid(gameRoom.getMasterUuid());
			gameRooms_.emplace(gameRoom.getMasterUuid(), std::move(gameRoom));
		}

		void release(ProtobufMessage&& message) {
			messageQueue_.release(std::move(message));
		}

		/*
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
		*/

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
				.allowToConnect = true,
				.type = RemoteType::Disconnected,
				.client = client
			});
			triggerConnectedClient(remote);
			return client;
		}

		std::vector<ConnectedClient> getConnectedClients() const {
			std::vector<ConnectedClient> connectedClients;
			for (const Remote& remote : remotes_) {
				if (remote.type == RemoteType::Connected) {
					connectedClients.push_back(convertToConnectedClient(remote));
				}
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

		void sendToClient(const std::string& clientUuid, const google::protobuf::MessageLite& message) override {
			for (auto& remote : remotes_) {
				if (remote.client->getUuid() == clientUuid) {
					sendToClient(*remote.client, message);
					break;
				}
			}
		}

		void sendToAllClients(const google::protobuf::MessageLite& message) override {
			for (auto& remote : remotes_) {
				sendToClient(*remote.client, message);
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
		enum class RemoteType {
			Spectator,
			Server,
			Client,
			Connected,
			Disconnected
		};

		struct Remote {
			bool allowToConnect;
			RemoteType type;
			std::shared_ptr<DebugClient> client;
		};

		ConnectedClient convertToConnectedClient(const Remote& remote) const {
			return ConnectedClient{
				.uuid = remote.client->getUuid(),
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

	void DebugServer::allowClientToConnect(const std::string& uuid, bool allow) {
		impl_->allowClientToConnect(uuid, allow);
	}

	mw::signals::Connection DebugServer::addClientListener(const std::function<void(const ConnectedClient&)>& callback) {
		return impl_->connectedClientListener.connect(callback);
	}

}
