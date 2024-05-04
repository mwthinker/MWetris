#ifndef MWETRIS_GAME_PLAYER_H
#define MWETRIS_GAME_PLAYER_H

#include "computer.h"
#include "tetrisboardcontroller.h"
#include "device.h"
#include "playerboardevent.h"

#include <tetrisboard.h>

#include <mw/signal.h>

#include <memory>
#include <variant>

namespace mwetris::game {

	class Player;
	using PlayerPtr = std::shared_ptr<Player>;

	class TetrisBoardMoveController;
	
	PlayerPtr createAiPlayer(const tetris::Ai& ai, const PlayerData& playerData = DefaultPlayerData{}, tetris::TetrisBoard&& tetrisBoard = tetris::TetrisBoard{10, 10, tetris::BlockType::L, tetris::BlockType::L});
	
	PlayerPtr createHumanPlayer(DevicePtr device, const PlayerData& playerData = DefaultPlayerData{}, tetris::TetrisBoard&& tetrisBoard = tetris::TetrisBoard{10, 10, tetris::BlockType::L, tetris::BlockType::L});

	PlayerPtr createRemotePlayer(const PlayerData& playerData = DefaultPlayerData{}, tetris::TetrisBoard&& tetrisBoard = tetris::TetrisBoard{10, 10, tetris::BlockType::L, tetris::BlockType::L});

	class Player {
	public:
		enum class Type {
			Human,
			Ai,
			Remote
		};

		//mw::PublicSignal<Player, tetris::BoardEvent, int> gameboardEventUpdate;
		mw::PublicSignal<Player, PlayerBoardEvent> playerBoardUpdate;

		Player(Type type, std::unique_ptr<TetrisBoardMoveController> moveController, tetris::TetrisBoard&& tetrisBoard);

		void update(double deltaTime);

		int getRows() const;

		int getColumns() const;

		tetris::BlockType getBlockType(int x, int y) const;

		tetris::BlockType getNextBlockType() const;

		int getClearedRows() const;

		bool isGameOver() const;

		void updateRestart(tetris::BlockType current, tetris::BlockType next);

		void updatePlayerData(const PlayerData& playerData);

		void updateMove(tetris::Move move);

		void updateNextBlock(tetris::BlockType next);

		tetris::Block getBlockDown() const;

		tetris::Block getBlock() const;

		const PlayerData& getPlayerData() const;

		bool isHuman() const;

		bool isAi() const;

		bool isRemote() const;

		bool isLocal() const;

		void setGravity(double gravity);

		void setClearedRows(int clearedRows);

		const std::vector<tetris::BlockType>& getBoardVector() const;

	protected:
		void handleBoardEvent(tetris::BoardEvent boardEvent, int value);

		void invokePlayerBoardUpdate(PlayerBoardEvent playerBoardEvent);

		std::unique_ptr<TetrisBoardMoveController> moveController_;
		mw::signals::ScopedConnections connections_;
		tetris::TetrisBoard tetrisBoard_;
		int clearedRows_ = 0;
		std::vector<tetris::BlockType> externalRows_;
		PlayerData playerData_;
		Type type_;
	};

}

#endif
