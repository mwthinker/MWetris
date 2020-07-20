#ifndef MWETRIS_GAME_TETRISGAMEEVENT_H
#define MWETRIS_GAME_TETRISGAMEEVENT_H

#include <vector>

#include "player.h"
#include "eventmanager.h"

namespace tetris::game {

	class NewConnection : public tetris::Event {
	public:
	};

	class GameStart : public tetris::Event {
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

	class CountDown : public tetris::Event {
	public:
		CountDown(int timeLeft)
			: timeLeft{timeLeft} {
		}

		int timeLeft{};
	};

	class GamePause : public tetris::Event {
	public:
		GamePause(bool pause, bool printPause)
			: pause{pause}
			, printPause{printPause} {
		}

		bool pause{};
		bool printPause{};
	};

	class GameOver : public tetris::Event {
	public:
		GameOver(const PlayerPtr& player)
			: player{player} {
		}

		PlayerPtr player;
	};

	class InitGame : public tetris::Event {
	public:
		InitGame(const std::vector<PlayerPtr>& players)
			: players{players} {
		}

		std::vector<PlayerPtr> players;
	};

	/*
	class RestartPlayer : public TetrisGameEvent {
	public:
		RestartPlayer(const std::shared_ptr<RemoteConnection>& connection) : connection_(connection) {
		}

		std::shared_ptr<RemoteConnection> connection_;
	};
	*/

	class LevelChange : public tetris::Event {
	public:
		LevelChange(const PlayerPtr& player, int newLevel, int oldLevel)
			: player{player}
			, newLevel{newLevel}
			, oldLevel{oldLevel} {
		}

		int newLevel;
		int oldLevel;
		PlayerPtr player;
	};

	class PointsChange : public tetris::Event {
	public:
		PointsChange(const PlayerPtr& player, int newPoints, int oldPoints)
			: player{player}
			, newPoints{newPoints}
			, oldPoints{oldPoints} {
		}

		int newPoints;
		int oldPoints;
		PlayerPtr player;
	};

}

#endif
