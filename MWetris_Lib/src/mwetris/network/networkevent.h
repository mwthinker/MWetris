#ifndef MWETRIS_NETWORK_NETWORKEVENT_H
#define MWETRIS_NETWORK_NETWORKEVENT_H

#include "../game/player.h"
#include "../game/playerslot.h"
#include "id.h"

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

	struct JoinGameRoomEvent {
		bool join;
	};

	struct PauseEvent {
		bool pause;
	};

	struct CreateGameEvent {
		std::vector<game::PlayerPtr> players;
	};

	struct LeaveGameRoomEvent {
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

	using NetworkEvent = std::variant<
		PlayerSlotEvent,
		RestartEvent,
		JoinGameRoomEvent,
		PauseEvent,
		CreateGameEvent,
		LeaveGameRoomEvent,
		ClientDisconnectedEvent,
		GameRoomListEvent
	>;

}

#endif
