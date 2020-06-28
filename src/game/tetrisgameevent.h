#ifndef MWETRIS_GAME_TETRISGAMEEVENT_H
#define MWETRIS_GAME_TETRISGAMEEVENT_H

#include <vector>

#include "player.h"

namespace tetris::game {

	class TetrisGameEvent {
	public:
		virtual ~TetrisGameEvent() = default;
	};

	class NewConnection : public TetrisGameEvent {
	public:
	};

	class GameStart : public TetrisGameEvent {
	public:
		enum class Status {
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
		GameOver(const PlayerPtr& player)
			: player_{player} {
		}

		PlayerPtr player_;
	};

	class InitGame : public TetrisGameEvent {
	public:
		InitGame(const std::vector<PlayerPtr>& players)
			: players_{players} {
		}

		std::vector<PlayerPtr> players_;
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
		LevelChange(const PlayerPtr& player, int newLevel, int oldLevel)
			: player_{player}
			, newLevel_{newLevel}
			, oldLevel_{oldLevel} {
		}

		int newLevel_;
		int oldLevel_;
		PlayerPtr player_;
	};

	class PointsChange : public TetrisGameEvent {
	public:
		PointsChange(const PlayerPtr& player, int newPoints, int oldPoints)
			: player_{player}
			, newPoints_{newPoints}
			, oldPoints_{oldPoints} {
		}

		int newPoints_;
		int oldPoints_;
		PlayerPtr player_;
	};

}

#endif
