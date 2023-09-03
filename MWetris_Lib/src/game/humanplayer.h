#ifndef MWETRIS_GAME_HUMANPLAYER_H
#define MWETRIS_GAME_HUMANPLAYER_H

#include "player.h"
#include "device.h"
#include "localplayerboard.h"

#include <tetrisboard.h>
#include <mw/signal.h>

#include <memory>

namespace mwetris::game {

	class HumanPlayer;
	using HumanPlayerPtr = std::shared_ptr<HumanPlayer>;

	class HumanPlayer : public Player {
	public:
		HumanPlayer(DevicePtr device, LocalPlayerBoardPtr localPlayerBoard)
			: device_{device}
			, localPlayerBoard_{localPlayerBoard} {}

		const DevicePtr& getDevice() const {
			return device_;
		}

		void update(double deltaTime) override {
			localPlayerBoard_->update(device_->getInput(), deltaTime);
		}

		void updateRestart(tetris::BlockType current, tetris::BlockType next) override {
			localPlayerBoard_->updateRestart(current, next);
		}

		void updatePlayerData(const PlayerData& playerData) override {
			localPlayerBoard_->updatePlayerData(playerData);
		}

		const PlayerData& getPlayerData() const override {
			return localPlayerBoard_->getPlayerData();
		}
		
		[[nodiscard]]
		mw::signals::Connection addPlayerBoardUpdateCallback(std::function<void(game::PlayerBoardEvent)>&& callback) override {
			return localPlayerBoard_->playerBoardUpdate.connect(callback);
		}

		[[nodiscard]]
		mw::signals::Connection addEventCallback(std::function<void(tetris::BoardEvent, int)>&& callback) override {
			return localPlayerBoard_->gameboardEventUpdate.connect(callback);
		}

		PlayerBoardPtr getPlayerBoard() const override {
			return localPlayerBoard_;
		}

		void addRowWithHoles(int nbr) override {
			localPlayerBoard_->addRow(nbr);
		}

		void updateGravity(float speed) override {
			localPlayerBoard_->updateGravity(speed);
		}

		const std::string& getName() const override {
			return localPlayerBoard_->getName();
		}

		const std::string& getUuid() const override {
			return localPlayerBoard_->getUniqueId();;
		}

		bool isAi() const override {
			return false;
		}

	private:
		DevicePtr device_;
		LocalPlayerBoardPtr localPlayerBoard_;
	};

}

#endif
