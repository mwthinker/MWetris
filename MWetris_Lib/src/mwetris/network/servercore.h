#ifndef MWETRIS_NETWORK_SERVERCORE_H
#define MWETRIS_NETWORK_SERVERCORE_H

#include "asio.h"
#include "client.h"
#include "gameroom.h"
#include "id.h"
#include "protobufmessage.h"
#include "protobufmessagequeue.h"
#include "server.h"

#include "../game/player.h"
#include "../util/auxiliary.h"
#include "../util/uuid.h"

#include <protocol/client_to_server.pb.h>
#include <protocol/server_to_client.pb.h>

#include <optional>

namespace mwetris::network {

	struct Remote {
		std::shared_ptr<Client> client;
		ClientId clientId;
	};

	template <typename T>
	using OptionalRef = std::optional<std::reference_wrapper<T>>;

	class ServerCore : public Server {
	public:
		mw::PublicSignal<ServerCore, const std::vector<Slot>&> playerSlotsUpdated;
		mw::PublicSignal<ServerCore, const std::vector<game::PlayerPtr>&> gameCreated;
		mw::PublicSignal<ServerCore, const ConnectedClient&> connectedClientListener;

		~ServerCore() override;

		/// @brief Stop all active coroutines.
		/// 
		/// I.e. makes the coroutines drop the ownership of "this" object and all
		/// connected client coroutines. But current jobs must still be run to
		/// completion. I.e. ioContext.poll() must be called until all jobs are
		/// done to garantee that the coroutines are finished.
		void stop();

		/// @brief Start the server. Spawns a coroutine.
		void start();

		void release(ProtobufMessage&& message);

		void acquire(ProtobufMessage& message);

		void sendFailedToConnect(Client& client);

		void sendPause(const GameRoomId& gameRoomId, bool pause);

		void restartGame(const GameRoomId& gameRoomId);

		bool isPaused(const GameRoomId& gameRoomId) const;

		void disconnect(const GameRoomId& gameRoomId);

		std::vector<ConnectedClient> getConnectedClients() const;

		asio::io_context& getIoContext() {
			return ioContext_;
		}

	protected:
		explicit ServerCore(asio::io_context& ioContext);

		virtual asio::awaitable<void> run() = 0;

		asio::awaitable<void> receivedFromClient(Remote remote);

		void receivedFromRemote(Remote& fromRemote, const tp_c2s::Wrapper& wrapper);

		void handleCreateGameRoom(Remote& remote, const tp_c2s::CreateGameRoom& createGameRoom);

		void handleJoinGameRoom(Remote& remote, const tp_c2s::JoinGameRoom& joinGameRoom);

		void handleLeaveGameRoom(Remote& remote, const tp_c2s::LeaveGameRoom& leaveGameRoom);

		void handleRequestGameRoomList(Remote& server, const tp_c2s::RequestGameRoomList& requestGameRoomList);

		void sendToClient(const ClientId& clientId, const google::protobuf::MessageLite& message) override;

		void triggerConnectedClientEvent(const ConnectedClient& connectedClient) override;

		void triggerPlayerSlotEvent(const std::vector<Slot>& slots) override;

		ConnectedClient convertToConnectedClient(const Remote& remote) const;

		void triggerConnectedClient(const Remote& remote);

		void sendToClients(const google::protobuf::MessageLite& wrapper);

		void sendToClient(Client& client, const google::protobuf::MessageLite& wrapper);

		OptionalRef<GameRoom> findGameRoom(const ClientId& clientId);

		asio::io_context& ioContext_;
		std::map<ClientId, GameRoomId> roomIdByClientId_;
		std::map<GameRoomId, GameRoom> gameRoomById_;
		std::map<ClientId, Remote> remoteByClientId_;

		tp_c2s::Wrapper wrapperFromClient_;
		tp_s2c::Wrapper wrapperToClient_;
		ProtobufMessageQueue messageQueue_;
		bool isStopped_ = false;
	};

}

#endif
