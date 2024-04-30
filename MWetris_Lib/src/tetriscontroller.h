#ifndef MWETRIS_GAME_TETRISCONTROLLER_H
#define MWETRIS_GAME_TETRISCONTROLLER_H

#include "game/device.h"
#include "game/gamerules.h"
#include "game/player.h"
#include "game/playerslot.h"

#include <spdlog/spdlog.h>

#include <mw/signal.h>

#include <variant>

namespace mwetris {

	namespace network {

		class Network;

	}

	namespace game {

		class TetrisGame;

	}

	namespace graphic {

		class GameComponent;

	}

	struct PlayerSlotEvent {
		game::PlayerSlot playerSlot;
		int slot;
	};

	using TetrisEvent = std::variant<game::GamePause, game::GameOver, PlayerSlotEvent>;

	class TetrisController {
	public:
		mw::PublicSignal<TetrisController, TetrisEvent> tetrisEvent;

		TetrisController(std::shared_ptr<network::Network> network, std::shared_ptr<game::TetrisGame> tetrisGame, std::shared_ptr<graphic::GameComponent> gameComponent);

		// Updates everything. Should be called each frame.
		void update(double deltaTime);

		void draw(int width, int height, double deltaTime);

		void createDefaultGame(game::DevicePtr device);

		void startNetworkGame(int w, int h);

		void createGame(std::unique_ptr<game::GameRules> gameRules, const std::vector<game::PlayerPtr>& players);
		bool isPaused() const;

		// Pause/Unpause the game depending on the current state of the game.
		void pause();

		// Restart the active game.
		void restartGame();

		const std::string& getGameRoomUuid() const;

		void createGameRoom(const std::string& name);

		void joinGameRoom(const std::string& gameRoomUuid);

		void setPlayerSlot(const game::PlayerSlot& playerSlot, int slot);

		int getNbrOfPlayers() const;

		void saveDefaultGame();

		void setFixTimestep(double delta);

		bool isDefaultGame() const;

	private:
		std::shared_ptr<network::Network> network_;
		std::shared_ptr<game::TetrisGame> tetrisGame_;
		std::shared_ptr<graphic::GameComponent> gameComponent_;
		mw::signals::ScopedConnections connections_;
	};

}

#endif
