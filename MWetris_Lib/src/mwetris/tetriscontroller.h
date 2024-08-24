#ifndef MWETRIS_GAME_TETRISCONTROLLER_H
#define MWETRIS_GAME_TETRISCONTROLLER_H

#include "game/device.h"
#include "game/gamerules.h"
#include "game/player.h"
#include "game/playerslot.h"
#include "network/networkevent.h"
#include "game/tetrisgameevent.h"
#include "game/tetrisgame.h"
#include "game/tetrisgameevent.h"
#include "game/devicemanager.h"

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
		GameSession,
		LocalGameRoomLooby,
		NetworkWaitingCreateGameRoom,
		NetworkGameRoomLooby
	};
	
	struct GameRoomEvent {
		GameRoomType type;
	};

	struct CreateGameEvent {
	};

	using TetrisEvent = std::variant<game::GamePause, game::GameOver, game::GameRoomConfigEvent, PlayerSlotEvent, GameRoomEvent, network::GameRoomListEvent, network::NetworkErrorEvent>;

	class TetrisController {
	public:
		mw::PublicSignal<TetrisController, const TetrisEvent&> tetrisEvent;

		TetrisController(std::shared_ptr<game::DeviceManager> deviceManager, std::shared_ptr<network::Network> network, std::shared_ptr<graphic::GameComponent> gameComponent);

		// Updates everything. Should be called each frame.
		void update(double deltaTime);

		void draw(int width, int height, double deltaTime);

		void startNetworkGame(const game::GameRulesConfig& gameRulesConfig, int w, int h);

		void startLocalGame(const game::GameRulesConfig& gameRulesConfig, const std::vector<game::PlayerPtr>& players);
		bool isPaused() const;

		// Pause/Unpause the game depending on the current state of the game.
		void pause();

		// Restart the active game.
		void restartGame();

		void updateGameRulesConfig(const game::GameRulesConfig& gameRulesConfig);

		const char* getGameRoomId() const;

		bool isGameRoomSession() const;

		bool isNetworkGame() const;

		void createDefaultGame(game::DevicePtr device);
		void createLocalGameRoom();
		void createNetworkGameRoom(const std::string& name, bool isPublic);

		void joinGameRoom(const std::string& gameRoomId);

		void leaveGameRoom();

		void setPlayerSlot(const game::PlayerSlot& playerSlot, int slot);

		int getNbrOfPlayers() const;

		void saveDefaultGame();

		void setFixTimestep(double delta);

		void refreshGameRoomList();

		bool isDefaultGame() const;

		const std::vector<network::GameRoomClient>& getGameRoomClients() const;

		bool isConnectedToServer() const;

	private:
		void createGame(const std::vector<game::PlayerPtr>& players, const game::GameRulesConfig& gameRulesConfig);

		void onNetworkEvent(const network::PlayerSlotEvent& playerSlotEvent);
		void onNetworkEvent(const network::RestartEvent& restartEvent);
		void onNetworkEvent(const network::JoinGameRoomEvent& joinGameRoomEvent);
		void onNetworkEvent(const network::GameRoomEvent& gameRoomEvent);
		void onNetworkEvent(const network::PauseEvent& pauseEvent);
		void onNetworkEvent(const network::CreateGameEvent& createGameEvent);
		void onNetworkEvent(const network::LeaveGameRoomEvent& leaveGameRoomEvent);
		void onNetworkEvent(const network::ClientDisconnectedEvent& clientDisconnectedEvent);
		void onNetworkEvent(const network::GameRoomListEvent& gameRoomListEvent);
		void onNetworkEvent(const game::GameRoomConfigEvent& gameRoomConfigEvent);
		void onNetworkEvent(const network::NetworkErrorEvent& networkErrorEvent);

		void setGameRoomType(GameRoomType gameRoomType);

		mw::signals::ScopedConnections connections_;
		std::shared_ptr<game::DeviceManager> deviceManager_;
		std::unique_ptr<game::GameRules> rules_;
		std::shared_ptr<network::Network> network_;
		game::TetrisGame tetrisGame_;
		std::shared_ptr<graphic::GameComponent> gameComponent_;
		GameRoomType gameRoomType_ = GameRoomType::OutsideGameRoom;
		game::GameRulesConfig gameRulesConfig_;
		std::vector<network::GameRoomClient> gameRoomClients_;
	};

}

#endif
