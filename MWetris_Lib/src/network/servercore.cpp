#include "servercore.h"

#include <spdlog/spdlog.h>

namespace mwetris::network {

	ServerCore::ServerCore(asio::io_context& ioContext)
		: messageQueue_{100}
		, ioContext_{ioContext} {}

	void ServerCore::start() {
		asio::co_spawn(ioContext_, run(), asio::detached);
	}

	void ServerCore::stop() {
		for (auto& remote : remotes_) {
			remote.client->stop();
		}
		remotes_.clear();
		isStopped_ = true;
	}

	asio::awaitable<void> ServerCore::receivedFromClient(Remote remote) try {
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
	} catch (const std::exception& e) {
		spdlog::error("[ServerCore] Exception: {}", e.what());
		//throw;
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
		}

		if (auto it = roomIdByClientId_.find(fromRemote.clientId); it != roomIdByClientId_.end()) {
			auto& gameRoom = gameRoomById_.at(roomIdByClientId_.at(fromRemote.clientId));
			gameRoom.receiveMessage(*this, fromRemote.clientId, wrapper);
		}
	}

	void ServerCore::handleCreateGameRoom(Remote& remote, const tp_c2s::CreateGameRoom& createGameRoom) {
		if (roomIdByClientId_.contains(remote.clientId)) {
			spdlog::warn("[DebugServer] Client with uuid {} already in a GameRoom", remote.clientId);
			return;
		}

		GameRoom gameRoom;
		roomIdByClientId_.emplace(remote.clientId, gameRoom.getGameRoomId());
		gameRoomById_.emplace(gameRoom.getGameRoomId(), std::move(gameRoom));
	}

	void ServerCore::handleJoinGameRoom(Remote& remote, const tp_c2s::JoinGameRoom& joinGameRoom) {
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

	void ServerCore::handleLeaveGameRoom(Remote& remote, const tp_c2s::LeaveGameRoom& leaveGameRoom) {
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
		for (const Remote& remote : remotes_) {
			connectedClients.push_back(convertToConnectedClient(remote));
		}
		return connectedClients;
	}

	void ServerCore::sendToClient(const ClientId& clientId, const google::protobuf::MessageLite& message) {
		for (auto& remote : remotes_) {
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
		for (auto& remote : remotes_) {
			sendToClient(*remote.client, wrapper);
		}
	}

	void ServerCore::sendToClient(Client& client, const google::protobuf::MessageLite& wrapper) {
		ProtobufMessage message;
		messageQueue_.acquire(message);
		message.setBuffer(wrapper);
		client.send(std::move(message));
	}

}
