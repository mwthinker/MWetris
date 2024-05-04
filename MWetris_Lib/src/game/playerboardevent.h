#ifndef MWETRIS_GAME_PLAYERBOARDEVENT_H
#define MWETRIS_GAME_PLAYERBOARDEVENT_H

#include <tetrisboard.h>
#include <variant>
#include <concepts>

namespace mwetris::game {

	struct DefaultPlayerData {
		int level;
		int points;
	};

	struct SurvivalPlayerData {
		int opponentRows;
	};

	using PlayerData = std::variant<DefaultPlayerData, SurvivalPlayerData>;

	struct UpdateNextBlock {
		tetris::BlockType next;
	};

	struct UpdateRestart {
		tetris::BlockType current;
		tetris::BlockType next;
	};
	struct UpdatePlayerData {
		PlayerData playerData;
	};
	struct UpdateMove {
		tetris::Move move;
	};
	struct TetrisBoardEvent {
		tetris::BoardEvent event;
		int value;
	};

	struct ExternalRows {};

	using PlayerBoardEvent = std::variant<UpdateRestart, UpdatePlayerData, ExternalRows, UpdateMove, UpdateNextBlock, TetrisBoardEvent>;

	template <typename F>
	concept PlayerBoardEventCallback = std::invocable<F, const PlayerBoardEvent&>;

}

#endif
