#ifndef APP_GAME_TETRISGAMEEVENT_H
#define APP_GAME_TETRISGAMEEVENT_H

#include "defaultgamerules.h"

#include <memory>

namespace app::game {

	class Player;
	using PlayerPtr = std::shared_ptr<Player>;

	enum class GamePauseAction {
		Countdown,
		CountdownFinished,
		Started,
		Paused,
		Resumed
	};

	struct GamePause {
		int countDown;
		bool pause;
		GamePauseAction action;
	};

	struct GameOver {
		PlayerPtr player;
	};

	struct GameRoomConfigEvent {
		game::GameRulesConfig gameRulesConfig;
	};

}

#endif
