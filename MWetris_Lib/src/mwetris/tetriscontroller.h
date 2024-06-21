#ifndef MWETRIS_GAME_TETRISCONTROLLER_H
#define MWETRIS_GAME_TETRISCONTROLLER_H

#include "game/device.h"
#include "game/gamerules.h"
#include "game/player.h"
#include "game/playerslot.h"
#include "network/networkevent.h"
#include "game/tetrisgameevent.h"

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

	enum class GameRoomType {
		OutsideGameRoom,
		LocalInsideGameRoom,
		NetworkWaitingCreateGameRoom,
		NetworkInsideGameRoom
	};
	
	struct GameRoomEvent {
		GameRoomType type;
	};

	struct CreateGameEvent {
	};

	using TetrisEvent = std::variant<game::GamePause, game::GameOver, PlayerSlotEvent, GameRoomEvent, CreateGameEvent, network::GameRoomListEvent>;

	class TetrisController {
	public:
		mw::PublicSignal<TetrisController, TetrisEvent> tetrisEvent;

		TetrisController(std::shared_ptr<network::Network> network, std::shared_ptr<game::TetrisGame> tetrisGame, std::shared_ptr<graphic::GameComponent> gameComponent);

		// Updates everything. Should be called each frame.
		void update(double deltaTime);

		void draw(int width, int height, double deltaTime);

		void createDefaultGame(game::DevicePtr device);

		void startNetworkGame(int w, int h);

		void startLocalGame(std::unique_ptr<game::GameRules> gameRules, const std::vector<game::PlayerPtr>& players);
		bool isPaused() const;

		// Pause/Unpause the game depending on the current state of the game.
		void pause();

		// Restart the active game.
		void restartGame();

		const char* getGameRoomId() const;

		bool isInsideGameRoom() const;

		void createLocalGameRoom();
		void createNetworkGameRoom(const std::string& name);

		void joinGameRoom(const std::string& gameRoomUuid);

		void leaveGameRoom();

		void setPlayerSlot(const game::PlayerSlot& playerSlot, int slot);

		int getNbrOfPlayers() const;

		void saveDefaultGame();

		void setFixTimestep(double delta);

		bool isDefaultGame() const;

		void refreshGameRoomList();

	private:
		void setGameRoomType(GameRoomType gameRoomType);

		std::shared_ptr<network::Network> network_;
		std::shared_ptr<game::TetrisGame> tetrisGame_;
		std::shared_ptr<graphic::GameComponent> gameComponent_;
		mw::signals::ScopedConnections connections_;
		GameRoomType gameRoomType_ = GameRoomType::OutsideGameRoom;
	};

}

#endif
