#ifndef MWETRIS_GAME_TETRISGAMEEVENT_H
#define MWETRIS_GAME_TETRISGAMEEVENT_H

#include <vector>

//#include "player.h"

#include <tetrisboard.h>

namespace mwetris::game {

	class PlayerBoard;
	using PlayerBoardPtr = std::shared_ptr<PlayerBoard>;

	class GameBoardEvent {
	public:
		GameBoardEvent(PlayerBoardPtr playerBoard, tetris::BoardEvent type, int nbr)
			: playerBoard{playerBoard}
			, type{type}
			, nbr{nbr} {}

		PlayerBoardPtr playerBoard;
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
		GameOver(PlayerBoardPtr playerBoard)
			: playerBoard{playerBoard} {}

		PlayerBoardPtr playerBoard;
	};

	class InitGameEvent {
	public:
		InitGameEvent(std::input_iterator auto begin, std::input_iterator auto end)
			: playerBoards(begin, end) {}

		std::vector<PlayerBoardPtr> playerBoards;
	};

	class LevelChange {
	public:
		LevelChange(PlayerBoardPtr playerBoard, int newLevel, int oldLevel)
			: playerBoard{playerBoard}
			, newLevel{newLevel}
			, oldLevel{oldLevel} {}

		PlayerBoardPtr playerBoard;
		int newLevel;
		int oldLevel;
	};

	class PointsChange {
	public:
		PointsChange(PlayerBoardPtr playerBoard, int newPoints, int oldPoints)
			: playerBoard{playerBoard}
			, newPoints{newPoints}
			, oldPoints{oldPoints} {}

		PlayerBoardPtr playerBoard;
		int newPoints;
		int oldPoints;
	};

}

#endif
