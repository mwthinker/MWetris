#ifndef MWETRIS_GAME_TETRISGAMEEVENT_H
#define MWETRIS_GAME_TETRISGAMEEVENT_H

#include <tetrisboard.h>

#include <vector>
#include <memory>

namespace mwetris::game {

	class Player;
	using PlayerPtr = std::shared_ptr<Player>;

	class NewConnection {
	public:
	};

	class GameStart {
	public:
		enum class Status {
			LOCAL,
			SERVER,
			CLIENT
		};

		GameStart(Status status)
			: status{status} {}

		Status status{Status::LOCAL};
	};

	class GamePause {
	public:
		int countDown;
		bool pause;
	};

	class GameRestart {
	public:
		tetris::BlockType current;
		tetris::BlockType next;
	};

	class GameOver {
	public:
		GameOver(PlayerPtr player)
			: player{player} {
		}

		PlayerPtr player;
	};

	class InitGameEvent {
	public:
		InitGameEvent(std::input_iterator auto begin, std::input_iterator auto end)
			: players(begin, end) {
		}

		std::vector<PlayerPtr> players;
	};

	class LevelChange {
	public:
		LevelChange(PlayerPtr player, int newLevel, int oldLevel)
			: player{player}
			, newLevel{newLevel}
			, oldLevel{oldLevel} {}

		PlayerPtr player;
		int newLevel;
		int oldLevel;
	};

	class PointsChange {
	public:
		PointsChange(PlayerPtr player, int newPoints, int oldPoints)
			: player{player}
			, newPoints{newPoints}
			, oldPoints{oldPoints} {}

		PlayerPtr player;
		int newPoints;
		int oldPoints;
	};

}

#endif
