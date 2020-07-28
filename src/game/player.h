#ifndef MWETRIS_GAME_PLAYER_H
#define MWETRIS_GAME_PLAYER_H

#include "tetrisboardwrapper.h"
#include "eventmanager.h"
#include "device.h"

#include <string>
#include <memory>

namespace mwetris::game {

	class Player;
	using PlayerPtr = std::shared_ptr<Player>;

	class Player : public std::enable_shared_from_this<Player> {
	public:
		Player(std::shared_ptr<EventManager> eventManager)
			: eventManager_{eventManager}
			, senderId_{eventManager->generateSenderId()} {
			
		}

		virtual ~Player() {
		}

		virtual std::string getName() const = 0;

		virtual int getLevel() const = 0;

		virtual int getPoints() const = 0;

		virtual int getClearedRows() const = 0;

		virtual int getLevelUpCounter() const = 0;

		virtual bool isGameOver() const = 0;

		virtual int getGameOverPosition() const = 0;

		virtual DevicePtr getDevice() const = 0;

		virtual const TetrisBoardWrapper& getTetrisBoard() const = 0;

		SenderId getSenderId() const {
			return senderId_;
		}

	protected:
		template <class Type, class... Args>
		void publishEvent(Args&&... args) {
			eventManager_->publish<Type>(senderId_, std::forward<Args>(args)...);
		}

		SubscriptionHandle subscribe(SenderId senderId, const EventManager::Callback& callback) {
			return eventManager_->subscribe(senderId, callback);
		}

	private:
		std::shared_ptr<EventManager> eventManager_;
		SenderId senderId_;
	};

}

#endif
