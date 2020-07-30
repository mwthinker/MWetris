#ifndef MWETRIS_GAME_TETRISGAMEEVENT_H
#define MWETRIS_GAME_TETRISGAMEEVENT_H

#include <vector>

#include "player.h"
#include "eventmanager.h"

namespace mwetris::game {

	class GameBoardEvent : public mwetris::Event {
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

	class NewConnection : public mwetris::Event {
	public:
	};

	class GameStart : public mwetris::Event {
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

	class CountDown : public mwetris::Event {
	public:
		CountDown(int timeLeft)
			: timeLeft{timeLeft} {
		}

		int timeLeft{};
	};

	class GamePause : public mwetris::Event {
	public:
		GamePause(bool pause, bool printPause)
			: pause{pause}
			, printPause{printPause} {
		}

		bool pause{};
		bool printPause{};
	};

	class GameOver : public mwetris::Event {
	public:
		GameOver(PlayerPtr player)
			: player{std::move(player)} {
		}

		PlayerPtr player;
	};

	class InitGame : public mwetris::Event {
	public:
		InitGame(const std::vector<PlayerPtr>& players)
			: players{players} {
		}

		std::vector<PlayerPtr> players;
	};

	class LevelChange : public mwetris::Event {
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

	class PointsChange : public mwetris::Event {
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
