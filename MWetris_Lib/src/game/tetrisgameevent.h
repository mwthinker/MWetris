#ifndef MWETRIS_GAME_TETRISGAMEEVENT_H
#define MWETRIS_GAME_TETRISGAMEEVENT_H

#include <vector>

//#include "player.h"

#include <tetrisboard.h>

namespace mwetris::game {

	class Player;
	using PlayerPtr = std::shared_ptr<Player>;

	class GameBoardEvent {
	public:
		GameBoardEvent(PlayerPtr player, tetris::BoardEvent type, int nbr)
			: player{player}
			, type{type}
			, nbr{nbr} {
		}

		PlayerPtr player;
		tetris::BoardEvent type;
		int nbr;
	};

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
			: status{status} {
		}

		Status status{Status::LOCAL};
	};

	class GamePause {
	public:
		int countDown;
		bool pause;
	};

	class GameOver {
	public:
		GameOver(PlayerPtr player)
			: player{std::move(player)} {
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
			: player{std::move(player)}
			, newLevel{newLevel}
			, oldLevel{oldLevel} {
		}

		PlayerPtr player;
		int newLevel;
		int oldLevel;
	};

	class PointsChange {
	public:
		PointsChange(PlayerPtr player, int newPoints, int oldPoints)
			: player{std::move(player)}
			, newPoints{newPoints}
			, oldPoints{oldPoints} {
		}

		PlayerPtr player;
		int newPoints;
		int oldPoints;
	};

}

#endif
