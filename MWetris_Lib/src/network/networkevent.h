#ifndef MWETRIS_NETWORK_NETWORKEVENT_H
#define MWETRIS_NETWORK_NETWORKEVENT_H

#include "game/player.h"
#include "game/playerslot.h"
#include "id.h"

#include <block.h>

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

	struct RestartEvent {
		tetris::BlockType current;
		tetris::BlockType next;
	};

	struct JoinGameRoomEvent {
		bool join;
	};

	struct CreateGameRoomEvent {
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

}

#endif
