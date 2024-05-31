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

#include <server_to_client.pb.h>
#include <client_to_server.pb.h>

#include <asio.hpp>

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

		~ServerCore() override;

		void start();

		asio::awaitable<void> receivedFromClient(Remote remote);

		void receivedFromRemote(Remote& fromRemote, const tp_c2s::Wrapper& wrapper);

		void handleCreateGameRoom(Remote& remote, const tp_c2s::CreateGameRoom& createGameRoom);

		void handleJoinGameRoom(Remote& remote, const tp_c2s::JoinGameRoom& joinGameRoom);

		void handleLeaveGameRoom(Remote& remote, const tp_c2s::LeaveGameRoom& leaveGameRoom);

		void release(ProtobufMessage&& message);

		void acquire(ProtobufMessage& message);

		void sendFailedToConnect(Client& client);

		void sendPause(const GameRoomId& gameRoomId, bool pause);

		void restartGame(const GameRoomId& gameRoomId);

		bool isPaused(const GameRoomId& gameRoomId) const;

		void disconnect(const GameRoomId& gameRoomId);

		std::vector<ConnectedClient> getConnectedClients() const;

		void sendToClient(const ClientId& clientId, const google::protobuf::MessageLite& message) override;

		void triggerConnectedClientEvent(const ConnectedClient& connectedClient) override;

		void triggerPlayerSlotEvent(const std::vector<Slot>& slots) override;

		asio::io_context& getIoContext() {
			return ioContext_;
		}

	protected:
		virtual asio::awaitable<void> run() = 0;

		explicit ServerCore(asio::io_context& ioContext);

		ConnectedClient convertToConnectedClient(const Remote& remote) const;

		void triggerConnectedClient(const Remote& remote);

		void sendToClients(const google::protobuf::MessageLite& wrapper);

		void sendToClient(Client& client, const google::protobuf::MessageLite& wrapper);

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
