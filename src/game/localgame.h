#ifndef MWETRIS_GAME_LOCALGAME_H
#define MWETRIS_GAME_LOCALGAME_H

#include "player.h"
#include "localplayer.h"
#include "tetrisgameevent.h"

namespace tetris::game {

	class GameManager {
	public:
		virtual ~GameManager() = default;

		//svirtual int getWidth() const = 0;
		//virtual int getHeight() const = 0;

		//virtual std::vector<ILocalPlayerPtr> createLocalPlayers(const std::vector<IDevicePtr>& devices) = 0;
		//virtual bool setPause(bool pause);
		//virtual bool getPause() const;

		virtual void createGame(const std::vector<LocalPlayerPtr>& players) = 0;

		virtual void restartGame() = 0;

		virtual int getNbrAlivePlayers() const = 0;

		virtual bool isPaused() const = 0;

		virtual void setPaused(bool pause) = 0;

		virtual void close() = 0;

		virtual void waitForGameEvents() = 0;
	};

	class GameRules;

	class LocalGame : public GameManager {
	public:
		LocalGame(mw::Signal<TetrisGameEvent&>& gameEventSignal);

		void createGame(const std::vector<LocalPlayerPtr>& players) override;

		void restartGame() override;

		int getNbrAlivePlayers() const override;

		bool isPaused() const override {
			return false;
		}

		void setPaused(bool pause) override {
			pause_ = pause;
		}

		void close() override {
		}

		void waitForGameEvents() override {
		}

	private:
		std::unique_ptr<GameRules> gameRules_;
		bool pause_{};
	};

	class GameFactory {
	public:
		virtual ~GameFactory() = default;

		virtual std::unique_ptr<GameManager> createLocalGame() const = 0;

		virtual std::unique_ptr<GameManager> createClientGame() const = 0;

		virtual std::unique_ptr<GameManager> createServerGame() const = 0;
	};

	class PlayerFactory {
	public:
		virtual ~PlayerFactory() = default;

		//virtual ILocalPlayerPtr createLocalPlayer() = 0;

		//virtual IRemotePlayerPtr createRemotePlayer() = 0;
	};

}

#endif
