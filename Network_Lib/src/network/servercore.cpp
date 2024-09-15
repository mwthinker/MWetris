#include "servercore.h"

#include <spdlog/spdlog.h>

namespace network {

	ServerCore::ServerCore(asio::io_context& ioContext)
		: messageQueue_{100}
		, ioContext_{ioContext} {
	}

	void ServerCore::start() {
		asio::co_spawn(ioContext_, run(), asio::detached);
	}

	void ServerCore::stop() {
		for (auto& [_, remote] : remoteByClientId_) {
			remote.client->stop();
		}
		remoteByClientId_.clear();
		isStopped_ = true;
	}

	asio::awaitable<void> ServerCore::receivedFromClient(Remote remote) {
		while (!isStopped_) {
			auto client = remote.client;
			ProtobufMessage message = co_await client->receive();
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

	void ServerCore::receivedFromRemote(Remote& fromRemote, const tp_c2s::Wrapper& wrapper) {
		wrapperToClient_.Clear();

		if (wrapperFromClient_.has_create_game_room()) {
			handleCreateGameRoom(fromRemote, wrapperFromClient_.create_game_room());
		} else if (wrapperFromClient_.has_join_game_room()) {
			handleJoinGameRoom(fromRemote, wrapperFromClient_.join_game_room());
		} else if (wrapperFromClient_.has_leave_game_room()) {
			handleLeaveGameRoom(fromRemote, wrapperFromClient_.leave_game_room());
		} else if (wrapperFromClient_.has_request_game_room_list()) {
			handleRequestGameRoomList(fromRemote, wrapperFromClient_.request_game_room_list());
		}

		if (auto it = roomIdByClientId_.find(fromRemote.clientId); it != roomIdByClientId_.end()) {
			auto& gameRoom = gameRoomById_.at(roomIdByClientId_.at(fromRemote.clientId));
			gameRoom.receiveMessage(*this, fromRemote.clientId, wrapper);
		}
	}

	void ServerCore::handleCreateGameRoom(Remote& remote, const tp_c2s::CreateGameRoom& createGameRoom) {
		if (roomIdByClientId_.contains(remote.clientId)) {
			spdlog::warn("[DebugServer] Client with id {} already in a GameRoom", remote.clientId);
			return;
		}
		
		GameRoom gameRoom{createGameRoom.name(), createGameRoom.is_public()};
		roomIdByClientId_.emplace(remote.clientId, gameRoom.getGameRoomId());
		gameRoomById_.emplace(gameRoom.getGameRoomId(), std::move(gameRoom));
		spdlog::info("[DebugServer] GameRoom with id {} is created", gameRoom.getGameRoomId());
	}

	void ServerCore::handleJoinGameRoom(Remote& remote, const tp_c2s::JoinGameRoom& joinGameRoom) {
		if (roomIdByClientId_.contains(remote.clientId)) {
			spdlog::warn("[DebugServer] Client with id {} already in a GameRoom", remote.clientId);
			return;
		}

		if (auto it = gameRoomById_.find(joinGameRoom.game_room_id()); it != gameRoomById_.end()) {
			auto& gameRoom = it->second;
			if (gameRoom.isFull()) {
				spdlog::warn("[DebugServer] GameRoom with id {} is full", joinGameRoom.game_room_id());
				return;
			}
			roomIdByClientId_.emplace(remote.clientId, gameRoom.getGameRoomId());
			gameRoomById_.emplace(gameRoom.getGameRoomId(), std::move(gameRoom));
			spdlog::info("[DebugServer] GameRoom with id {} is joined by client {}", gameRoom.getGameRoomId(), remote.clientId);
		} else {
			spdlog::warn("[DebugServer] GameRoom with id {} not found", joinGameRoom.game_room_id());
		}
	}

	void ServerCore::handleLeaveGameRoom(Remote& remote, const tp_c2s::LeaveGameRoom& leaveGameRoom) {
		if (auto it = roomIdByClientId_.find(remote.clientId); it != roomIdByClientId_.end()) {
			auto gameRoomId = it->second;
			auto& gameRoom = gameRoomById_.at(gameRoomId);

			roomIdByClientId_.erase(it);
			gameRoom.disconnect(*this, remote.clientId);
			if (gameRoom.getConnectedClientSize() == 0) {
				gameRoomById_.erase(gameRoomId);
				spdlog::info("[DebugServer] Last client {} left GameRoom {} therefore it is closed", remote.clientId, gameRoomId);
			} else {
				spdlog::info("[DebugServer] Client {} left GameRoom {}", remote.clientId, gameRoomId);
			}
		}
	}

	void ServerCore::handleRequestGameRoomList(Remote& server, const tp_c2s::RequestGameRoomList& requestGameRoomList) {
		wrapperToClient_.Clear();
		auto gameRoomList = wrapperToClient_.mutable_game_room_list();
		for (const auto& [gameRoomId, gameRoom] : gameRoomById_) {
			if (!gameRoom.isPublic()) {
				continue;
			}

			auto gameRoomInfo = gameRoomList->add_game_rooms();
			fromCppToProto(gameRoomId, *gameRoomInfo->mutable_game_room_id());
			gameRoomInfo->set_name(gameRoom.getName());
			gameRoomInfo->set_max_player_count(4);
			gameRoomInfo->set_player_count(gameRoom.getConnectedClientSize());
		}
		sendToClient(*server.client, wrapperToClient_);
	}

	void ServerCore::release(ProtobufMessage&& message) {
		messageQueue_.release(std::move(message));
	}

	void ServerCore::acquire(ProtobufMessage& message) {
		messageQueue_.acquire(message);
	}

	void ServerCore::sendFailedToConnect(Client& client) {
		wrapperToClient_.Clear();
		wrapperToClient_.mutable_failed_to_connect();
		sendToClient(client, wrapperToClient_);
	}

	void ServerCore::sendPause(const GameRoomId& gameRoomId, bool pause) {
		gameRoomById_[gameRoomId].sendPause(*this, pause);
	}

	void ServerCore::restartGame(const GameRoomId& gameRoomId) {
		gameRoomById_[gameRoomId].requestRestartGame(*this);
	}

	bool ServerCore::isPaused(const GameRoomId& gameRoomId) const {
		return gameRoomById_.at(gameRoomId).isPaused();
	}

	void ServerCore::disconnect(const GameRoomId& gameRoomId) {
		gameRoomById_.erase(gameRoomId);
	}

	std::vector<ConnectedClient> ServerCore::getConnectedClients() const {
		std::vector<ConnectedClient> connectedClients;
		for (const auto& [_, remote] : remoteByClientId_) {
			connectedClients.push_back(convertToConnectedClient(remote));
		}
		return connectedClients;
	}

	void ServerCore::sendToClient(const ClientId& clientId, const google::protobuf::MessageLite& message) {
		for (const auto& [_, remote] : remoteByClientId_) {
			if (remote.clientId == clientId) {
				sendToClient(*remote.client, message);
				break;
			}
		}
	}

	void ServerCore::triggerConnectedClientEvent(const ConnectedClient& connectedClient) {
		connectedClientListener(connectedClient);
	}

	void ServerCore::triggerPlayerSlotEvent(const std::vector<Slot>& slots) {
		playerSlotsUpdated(slots);
	}

	ServerCore::~ServerCore() {
		stop();
	}

	ConnectedClient ServerCore::convertToConnectedClient(const Remote& remote) const {
		return ConnectedClient{
			.clientId = remote.clientId
		};
	}

	void ServerCore::triggerConnectedClient(const Remote& remote) {
		connectedClientListener(convertToConnectedClient(remote));
	}

	void ServerCore::sendToClients(const google::protobuf::MessageLite& wrapper) {
		for (const auto& [_, remote] : remoteByClientId_) {
			sendToClient(*remote.client, wrapper);
		}
	}

	void ServerCore::sendToClient(Client& client, const google::protobuf::MessageLite& wrapper) {
		ProtobufMessage message;
		messageQueue_.acquire(message);
		message.setBuffer(wrapper);
		client.send(std::move(message));
	}

	OptionalRef<GameRoom> ServerCore::findGameRoom(const ClientId& clientId) {
		if (auto it = roomIdByClientId_.find(clientId); it != roomIdByClientId_.end()) {
			auto gameRoomId = it->second;
			if (auto it = gameRoomById_.find(gameRoomId); it != gameRoomById_.end()) {
				return it->second;
			}
		}
		return std::nullopt;
	}

}
