#ifndef MWETRIS_GAME_TETRISGAMEEVENT_H
#define MWETRIS_GAME_TETRISGAMEEVENT_H

#include <vector>

//#include "player.h"

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

	class CountDown {
	public:
		CountDown(int timeLeft)
			: timeLeft{timeLeft} {
		}

		int timeLeft{};
	};

	class GamePause {
	public:
		GamePause(bool pause, bool printPause)
			: pause{pause}
			, printPause{printPause} {
		}

		bool pause{};
		bool printPause{};
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
		template <typename InputIt>
		InitGameEvent(InputIt begin, InputIt end)
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

		int newLevel;
		int oldLevel;
		PlayerPtr player;
	};

	class PointsChange {
	public:
		PointsChange(PlayerPtr player, int newPoints, int oldPoints)
			: player{std::move(player)}
			, newPoints{newPoints}
			, oldPoints{oldPoints} {
		}

		int newPoints;
		int oldPoints;
		PlayerPtr player;
	};

}

#endif
