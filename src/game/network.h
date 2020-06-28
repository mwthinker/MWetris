#ifndef MWETRIS_GAME_NETWORK_H
#define MWETRIS_GAME_NETWORK_H

#include "player.h"
#include "localplayer.h"
#include "tetrisgameevent.h"
#include "localgame.h"

#include <signal.h>
#include <net/server.h>
#include <net/client.h>
#include <message.pb.h>

#include <list>
#include <vector>
#include <deque>
#include <queue>
#include <memory>
#include <map>

namespace tetris::game {

	class GameRules;

	class ServerGame : public GameManager {
	public:
		ServerGame(mw::Signal<TetrisGameEvent&>& gameEventSignal);

		~ServerGame();

		void close() override;

		void connect(int port);

		void createGame(const std::vector<LocalPlayerPtr>& players) override;

		void restartGame() override;

		int getNbrAlivePlayers() const override;

		bool isPaused() const override;

		void setPaused(bool pause) override;

		void waitForGameEvents() override;

	private:
		void connected(const net::RemoteClientPtr& remoteClientPtr);
		void disconnected(const net::RemoteClientPtr& ptr);
		
		void receiveMessage(net::RemoteClientPtr ptr, const tetris_protocol::Wrapper& message);

		void handleMessage(const tetris_protocol::GameInfo& gameInfo);

		std::unique_ptr<GameRules> gameRules_;
		std::shared_ptr<net::Server> server_;
		tetris_protocol::Wrapper wrapper_;
		std::map<net::RemoteClientPtr, PlayerPtr> players_;

		std::queue<std::function<void()>> callbacks_;
		asio::io_context ioContext_;
	};

	class ClientGame : public GameManager {
	public:
		ClientGame(mw::Signal<TetrisGameEvent&>& gameEventSignal);

		~ClientGame();

		void close() override;

		void connect(const std::string& ip, int port);

		void createGame(const std::vector<LocalPlayerPtr>& players) override;

		void restartGame() override;

		int getNbrAlivePlayers() const override;

		bool isPaused() const override;

		void setPaused(bool pause) override;

		void waitForGameEvents() override;

	private:
		void connectHandler(std::error_code code);

		void receiveMessage(const tetris_protocol::Wrapper& message);

		void handleMessage(const tetris_protocol::GameInfo& gameInfo);

		void handleDisconnect();

		std::queue<std::function<void()>> callbacks_;
		std::unique_ptr<GameRules> gameRules_;
		std::shared_ptr<net::Client> client_;
		tetris_protocol::Wrapper wrapper_;
		asio::io_context ioContext_;
	};

}

#endif
