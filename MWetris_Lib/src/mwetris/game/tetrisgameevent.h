#ifndef MWETRIS_GAME_TETRISGAMEEVENT_H
#define MWETRIS_GAME_TETRISGAMEEVENT_H

#include <memory>

namespace mwetris::game {

	class Player;
	using PlayerPtr = std::shared_ptr<Player>;

	class GamePause {
	public:
		int countDown;
		bool pause;
	};

	struct GameOver {
		PlayerPtr player;
	};

}

#endif
