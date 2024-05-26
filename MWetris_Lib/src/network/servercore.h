#ifndef MWETRIS_NETWORK_SERVERCORE_H
#define MWETRIS_NETWORK_SERVERCORE_H

#include "client.h"
#include "protobufmessage.h"
#include "protobufmessagequeue.h"
#include "util.h"
#include "util/uuid.h"
#include "game/player.h"
#include "gameroom.h"
#include "server.h"
#include "util/uuid.h"
#include "id.h"
#include "tcpclient.h"

#include <helper.h>

#include <server_to_client.pb.h>
#include <client_to_server.pb.h>

#include <spdlog/spdlog.h>
#include <asio.hpp>

#include <queue>
#include <variant>

namespace mwetris::network {

	struct Remote {
		std::shared_ptr<Client> client;
		ClientId clientId;
	};

	class ServerCore : public Server {
	public:
		mw::PublicSignal<ServerCore, const std::vector<Slot>&> playerSlotsUpdated;
		mw::PublicSignal<ServerCore, const std::vector<game::PlayerPtr>&> gameCreated;
		mw::PublicSignal<ServerCore, const game::InitGameEvent&> initGameEvent;
		mw::PublicSignal<ServerCore, const ConnectedClient&> connectedClientListener;

		explicit ServerCore(asio::io_context& ioContext)
			: messageQueue_{100}
			, ioContext_{ioContext} {
		}

		~ServerCore() override {
		}

		void start() {
			asio::co_spawn(ioContext_, run(), asio::detached);
		}

		virtual asio::awaitable<void> run() = 0;

		asio::awaitable<void> receivedFromClient(Remote remote) {
			while (true) {
				ProtobufMessage message = co_await remote.client->receive();
				bool valid = message.getSize() > 0;
				if (valid) {
					wrapperFromClient_.Clear();
					valid = message.parseBodyInto(wrapperFromClient_);
					if (valid) {
						receivedFromRemote(remote, wrapperFromClient_);
					} else {
						spdlog::info("[ServerCore] Invalid data");
					}
					remote.client->release(std::move(message));
				} else {
					spdlog::info("[ServerCore] Invalid message size");
				}
			}
			co_return;
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

		void sendFailedToConnect(Client& client) {
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

		asio::io_context& getIoContext() {
			return ioContext_;
		}

	protected:
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

		void sendToClient(Client& client, const google::protobuf::MessageLite& wrapper) {
			ProtobufMessage message;
			messageQueue_.acquire(message);
			message.setBuffer(wrapper);
			client.send(std::move(message));
		}

		std::map<ClientId, GameRoomId> roomIdByClientId_;
		std::map<GameRoomId, GameRoom> gameRoomById_;

		asio::io_context& ioContext_;
		tp_c2s::Wrapper wrapperFromClient_;
		tp_s2c::Wrapper wrapperToClient_;
		ProtobufMessageQueue messageQueue_;
		std::vector<Remote> remotes_;
	};

}

#endif
