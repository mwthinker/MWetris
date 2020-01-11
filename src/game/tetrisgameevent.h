#ifndef TETRISGAMEEVENT_H
#define TETRISGAMEEVENT_H

#include <vector>

#include "player.h"

namespace tetris {

	class Connection;

	class TetrisGameEvent {
	public:
		virtual ~TetrisGameEvent() {
		}
	};

	class NewConnection : public TetrisGameEvent {
	public:
	};

	class GameStart : public TetrisGameEvent {
	public:
		enum Status {
			LOCAL,
			SERVER,
			CLIENT
		};

		GameStart(Status status)
			: status_{status} {
		}

		Status status_;
	};

	class CountDown : public TetrisGameEvent {
	public:
		CountDown(int timeLeft)
			: timeLeft_{timeLeft} {
		}

		int timeLeft_;
	};

	class GamePause : public TetrisGameEvent {
	public:
		GamePause(bool pause, bool printPause)
			: pause_{pause}
			, printPause_{printPause} {
		}

		bool pause_;
		bool printPause_;
	};

	class GameOver : public TetrisGameEvent {
	public:
		GameOver(const std::shared_ptr<IPlayer>& player)
			: player_{player} {
		}

		std::shared_ptr<IPlayer> player_;
	};

	class InitGame : public TetrisGameEvent {
	public:
		InitGame(const std::vector<std::shared_ptr<IPlayer>>& players)
			: players_{players} {
		}

		std::vector<std::shared_ptr<IPlayer>> players_;
	};

	/*
	class RestartPlayer : public TetrisGameEvent {
	public:
		RestartPlayer(const std::shared_ptr<RemoteConnection>& connection) : connection_(connection) {
		}

		std::shared_ptr<RemoteConnection> connection_;
	};
	*/

	class LevelChange : public TetrisGameEvent {
	public:
		LevelChange(const std::shared_ptr<IPlayer>& player, int newLevel, int oldLevel)
			: player_{player}
			, newLevel_{newLevel}
			, oldLevel_{oldLevel} {
		}

		int newLevel_;
		int oldLevel_;
		std::shared_ptr<IPlayer> player_;
	};

	class PointsChange : public TetrisGameEvent {
	public:
		PointsChange(const std::shared_ptr<IPlayer>& player, int newPoints, int oldPoints)
			: player_{player}
			, newPoints_{newPoints}
			, oldPoints_{oldPoints} {
		}

		int newPoints_;
		int oldPoints_;
		std::shared_ptr<IPlayer> player_;
	};

}

#endif // TETRISGAMEEVENT_H
