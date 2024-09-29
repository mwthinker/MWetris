#ifndef MWETRIS_NETWORK_NETWORKEVENT_H
#define MWETRIS_NETWORK_NETWORKEVENT_H

#include "../game/player.h"
#include "../game/playerslot.h"
#include "../game/tetrisgameevent.h"

#include <network/id.h>

#include <tetris/block.h>

#include <vector>

namespace mwetris::cnetwork {

	struct NetworkSlot {
		game::PlayerSlot playerSlot;
		network::ClientId clientId;
	};

	struct PlayerSlotEvent {
		game::PlayerSlot playerSlot;
		int index;
	};

	struct RestartEvent {};

	struct GameRoomEvent {
		std::vector<network::GameRoomClient> gameRoomClients;
	};

	struct JoinGameRoomEvent {
		network::ClientId clientId;
	};

	struct PauseEvent {
		bool pause;
	};

	struct CreateGameEvent {
		std::vector<game::PlayerPtr> players;
		game::GameRulesConfig gameRulesConfig;
	};

	struct LeaveGameRoomEvent {
		network::ClientId clientId;
	};

	struct ClientDisconnectedEvent {
		network::ClientId clientId;
	};

	struct GameRoomListEvent {
		struct GameRoom {
			network::GameRoomId id;
			std::string name;
			int playerCount;
			int maxPlayerCount;
		};

		std::vector<GameRoom> gameRooms;
	};

	struct NetworkErrorEvent {
		bool insideGameRoom;
	};

	using NetworkEvent = std::variant<
		PlayerSlotEvent,
		RestartEvent,
		JoinGameRoomEvent,
		GameRoomEvent,
		PauseEvent,
		CreateGameEvent,
		LeaveGameRoomEvent,
		ClientDisconnectedEvent,
		GameRoomListEvent,
		game::GameRoomConfigEvent,
		NetworkErrorEvent
	>;

}

#endif
