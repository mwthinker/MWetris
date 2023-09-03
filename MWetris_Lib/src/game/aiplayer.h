#ifndef MWETRIS_GAME_AIPLAYER_H
#define MWETRIS_GAME_AiPLAYER_H

#include "player.h"
#include "localplayerboard.h"

#include "computer.h"

#include <ai.h>

namespace mwetris::game {

	class AiPlayer : public Player {
	public:
		AiPlayer(const tetris::Ai& ai, LocalPlayerBoardPtr localPlayerBoard)
			: computer_{ai}
			, localPlayerBoard_{localPlayerBoard} {

			connections_ += localPlayerBoard_->gameboardEventUpdate.connect([this](tetris::BoardEvent boardEvent, int nbr) {
				computer_.onGameboardEvent(localPlayerBoard_->getTetrisBoard(), boardEvent, nbr);
			});
		}

		void update(double deltaTime) override {
			localPlayerBoard_->update(computer_.getInput(), deltaTime);
		}

		[[nodiscard]]
		mw::signals::Connection addPlayerBoardUpdateCallback(std::function<void(game::PlayerBoardEvent)>&& callback) override {
			return localPlayerBoard_->playerBoardUpdate.connect(callback);
		}

		[[nodiscard]]
		mw::signals::Connection addEventCallback(std::function<void(tetris::BoardEvent, int)>&& callback) override {
			return localPlayerBoard_->gameboardEventUpdate.connect(callback);
		}

		void updateRestart(tetris::BlockType current, tetris::BlockType next) override {
			localPlayerBoard_->updateRestart(current, next);
		}
		
		PlayerBoardPtr getPlayerBoard() const override {
			return localPlayerBoard_;
		}

		void addRowWithHoles(int nbr) override {
			localPlayerBoard_->addRow(nbr);
		}

		void updatePlayerData(const PlayerData& playerData) override {
			localPlayerBoard_->updatePlayerData(playerData);
		}

		const PlayerData& getPlayerData() const override {
			return localPlayerBoard_->getPlayerData();
		}

		void updateGravity(float speed) override {
			return localPlayerBoard_->updateGravity(speed);
		}

		const std::string& getName() const override {
			return localPlayerBoard_->getName();
		}

		const std::string& getUuid() const override {
			return localPlayerBoard_->getUniqueId();
		}

		bool isAi() const override {
			return true;
		}

	private:
		mw::signals::ScopedConnections connections_;

		Computer computer_;
		LocalPlayerBoardPtr localPlayerBoard_;
	};

}

#endif
