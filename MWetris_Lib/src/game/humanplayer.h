#ifndef MWETRIS_GAME_HUMANPLAYER_H
#define MWETRIS_GAME_HUMANPLAYER_H

#include "player.h"
#include "device.h"
#include "localplayerboard.h"

#include <tetrisboard.h>
#include <mw/signal.h>

#include <memory>

namespace mwetris::game {

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

		void updateRestart() override {
			localPlayerBoard_->updateRestart();
		}

		void updatePlayerData(const PlayerData& playerData) override {
			localPlayerBoard_->updatePlayerData(playerData);
		}

		const PlayerData& getPlayerData() const override {
			return localPlayerBoard_->getPlayerData();
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

	private:
		DevicePtr device_;
		LocalPlayerBoardPtr localPlayerBoard_;
	};

}

#endif
