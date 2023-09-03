#ifndef MWETRIS_GAME_REMOTEPLAYER_H
#define MWETRIS_GAME_REMOTEPLAYER_H

#include "remoteplayerboard.h"
#include "tetrisparameters.h"
#include "game/player.h"

#include <tetrisboard.h>

#include <mw/signal.h>

#include <memory>

namespace mwetris::game {

	class RemotePlayer;
	using RemotePlayerPtr = std::shared_ptr<RemotePlayer>;

	class RemotePlayer : public Player {
	public:
		RemotePlayer(tetris::BlockType current, tetris::BlockType next, const std::string& uuid) {
			uuid_ = uuid;
			tetris::TetrisBoard tetrisBoard{TetrisWidth, TetrisHeight, current, next};
			remotePlayerBoard_ = std::make_shared<RemotePlayerBoard>(tetrisBoard, "RemotePlayer");
		}

		void update(double timeStep) override {
			
		}

		void updateMove(tetris::Move move) {
			remotePlayerBoard_->updateMove(move);
		}

		void updateRestart(tetris::BlockType current, tetris::BlockType next) override {
			remotePlayerBoard_->updateRestart(current, next);
		}

		void updateNextBlock(tetris::BlockType next) {
			remotePlayerBoard_->updateNextBlock(next);
		}

		void updateAddExternalRows(const std::vector<tetris::BlockType>& blockTypes) {
			remotePlayerBoard_->updateAddExternalRows(blockTypes);
		}

		[[nodiscard]]
		mw::signals::Connection addPlayerBoardUpdateCallback(std::function<void(game::PlayerBoardEvent)>&& callback) override {
			return {};// remotePlayerBoard_->playerBoardUpdate.connect(callback);
		}

		[[nodiscard]]
		mw::signals::Connection addEventCallback(std::function<void(tetris::BoardEvent, int)>&& callback) override {
			return remotePlayerBoard_->gameboardEventUpdate.connect(callback);
		}

		PlayerBoardPtr getPlayerBoard() const override {
			return remotePlayerBoard_;
		}

		void addRowWithHoles(int nbr) override {
			//remotePlayerBoard_->addRow(nbr);
		}

		void updatePlayerData(const PlayerData& playerData) override {
			// remotePlayerBoard_->updatePlayerData(playerData);
		}

		const PlayerData& getPlayerData() const override {
			return remotePlayerBoard_->getPlayerData();
		}

		void updateGravity(float speed) override {
			//remotePlayerBoard_->updateGravity(speed);
		}

		const std::string& getName() const override {
			return remotePlayerBoard_->getName();
		}

		const std::string& getUuid() const override {
			return uuid_;
		}

		bool isAi() const override {
			return false;
		}

	private:
		RemotePlayerBoardPtr remotePlayerBoard_;
		std::string uuid_;

	};

}

#endif
