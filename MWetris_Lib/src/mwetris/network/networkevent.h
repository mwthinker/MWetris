#ifndef MWETRIS_NETWORK_NETWORKEVENT_H
#define MWETRIS_NETWORK_NETWORKEVENT_H

#include "../game/player.h"
#include "../game/playerslot.h"
#include "../game/tetrisgameevent.h"

#include <network/id.h>

#include <tetris/block.h>

#include <vector>

namespace mwetris::network {

	struct NetworkSlot {
		game::PlayerSlot playerSlot;
		ClientId clientId;
	};

	struct PlayerSlotEvent {
		game::PlayerSlot playerSlot;
		int index;
	};

	struct RestartEvent {};

	struct GameRoomEvent {
		std::vector<GameRoomClient> gameRoomClients;
	};

	struct JoinGameRoomEvent {
		ClientId clientId;
	};

	struct PauseEvent {
		bool pause;
	};

	struct CreateGameEvent {
		std::vector<game::PlayerPtr> players;
		game::GameRulesConfig gameRulesConfig;
	};

	struct LeaveGameRoomEvent {
		ClientId clientId;
	};

	struct ClientDisconnectedEvent {
		ClientId clientId;
	};

	struct GameRoomListEvent {
		struct GameRoom {
			GameRoomId id;
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
