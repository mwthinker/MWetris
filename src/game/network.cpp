#include "network.h"
#include "gamerules.h"

using namespace tetris;

ServerGame::ServerGame(mw::Signal<TetrisGameEvent&>& gameEventSignal) : 
	gameRules_(std::make_unique<GameRules>(gameEventSignal)) {
}

ServerGame::~ServerGame() {
}

void ServerGame::close() {
	server_->disconnect();
	server_ = nullptr;
}

void ServerGame::connect(int port) {
	if (!server_) {
		server_ = net::Server::create(ioContext_);

		server_->setConnectHandler([&](const net::RemoteClientPtr& remoteClientPtr) {
			remoteClientPtr->setReceiveHandler<tetris_protocol::Wrapper>([&](const tetris_protocol::Wrapper& message, std::error_code ec) {
				receiveMessage(remoteClientPtr, message);
			});

			remoteClientPtr->setDisconnectHandler([&](std::error_code ec) {
				disconnected(remoteClientPtr);
			});
		});

		server_->connect(port);
	}
}

namespace {

	tetris_protocol::BlockType toProtoBlockType(BlockType blockType) {
		switch (blockType) {
			case BlockType::EMPTY:
				return tetris_protocol::BlockType::EMPTY;
			case BlockType::I:
				return tetris_protocol::BlockType::I;
			case BlockType::J:
				return tetris_protocol::BlockType::J;
			case BlockType::L:
				return tetris_protocol::BlockType::L;
			case BlockType::O:
				return tetris_protocol::BlockType::O;
			case BlockType::S:
				return tetris_protocol::BlockType::S;
			case BlockType::T:
				return tetris_protocol::BlockType::T;
			case BlockType::WALL:
				return tetris_protocol::BlockType::WALL;
			case BlockType::Z:
				return tetris_protocol::BlockType::Z;
		}
		assert(false);
		return tetris_protocol::BlockType::EMPTY;
	}

	BlockType fromProtoBlockType(tetris_protocol::BlockType blockType) {
		switch (blockType) {
			case tetris_protocol::BlockType::EMPTY:
				return BlockType::EMPTY;
			case tetris_protocol::BlockType::I:
				return BlockType::I;
			case tetris_protocol::BlockType::J:
				return BlockType::J;
			case tetris_protocol::BlockType::L:
				return BlockType::L;
			case tetris_protocol::BlockType::O:
				return BlockType::O;
			case tetris_protocol::BlockType::S:
				return BlockType::S;
			case tetris_protocol::BlockType::T:
				return BlockType::T;
			case tetris_protocol::BlockType::WALL:
				return BlockType::WALL;
			case tetris_protocol::BlockType::Z:
				return BlockType::Z;
		}
		assert(false);
		return BlockType::EMPTY;
	}

	void updateProtoPlayer(tetris_protocol::Player* updatePlayer, const ILocalPlayerPtr& player) {
		updatePlayer->set_id(1);
		updatePlayer->set_ai(player->getDevice()->isAi());
		updatePlayer->set_name(player->getName());
		updatePlayer->set_level(player->getLevel());
		updatePlayer->set_next(toProtoBlockType(player->getTetrisBoard().getNextBlockType()));
		updatePlayer->set_points(player->getPoints());
		updatePlayer->set_current(toProtoBlockType(player->getTetrisBoard().getBlockType()));
	}

}

void ServerGame::createGame(const std::vector<ILocalPlayerPtr>& players) {
	gameRules_->createGame(players);

	tetris_protocol::ConnectionInfo connection;

	for (auto& localPlayer : players) {
		auto* protoPlayer = connection.add_players();
		updateProtoPlayer(protoPlayer, localPlayer);
	}

	server_->sendToAll(connection);
}

void ServerGame::restartGame() {
	gameRules_->restartGame();
}

int ServerGame::getNbrAlivePlayers() const {
	return gameRules_->getNbrAlivePlayers();
}

bool ServerGame::isPaused() const {
	return false;
}

void ServerGame::setPaused(bool pause) {
	
}

void ServerGame::connected(const net::RemoteClientPtr& remoteClientPtr) {
	//players_[remoteClientPtr] = std::make_shared<IRemotePlayer>();
}

void ServerGame::disconnected(const net::RemoteClientPtr& remoteClientPtr) {

}

void ServerGame::receiveMessage(net::RemoteClientPtr remotePtr, const tetris_protocol::Wrapper& wrapper) {
	callbacks_.push([this, remotePtr, wrapper] () {
		if (wrapper.has_game_info()) {
			handleMessage(wrapper.game_info());
		}
	});
}

void ServerGame::handleMessage(const tetris_protocol::GameInfo& gameInfo) {
	int width = gameInfo.width();
	int height = gameInfo.height();
}

void ServerGame::waitForGameEvents() {
	while (!callbacks_.empty()) {
		callbacks_.pop();
	}
}

ClientGame::ClientGame(mw::Signal<TetrisGameEvent&>& gameEventSignal) :
	gameRules_(std::make_unique<GameRules>(gameEventSignal)) {
}

ClientGame::~ClientGame() {
}

void ClientGame::close() {
	client_->disconnect();
	client_ = nullptr;
}

void ClientGame::connect(const std::string& ip, int port) {
	if (!client_) {
		client_ = net::Client::create(ioContext_);
		client_->setReceiveHandler<tetris_protocol::Wrapper>([&](const tetris_protocol::Wrapper& wrapper, std::error_code ec) {
			receiveMessage(wrapper);
		});

		client_->setDisconnectHandler([&](std::error_code ec) {
			handleDisconnect();
		});

		client_->setConnectHandler([&](std::error_code code) {
			connectHandler(code);
		});

		try {
			client_->connect(ip, port);
		} catch (asio::system_error e) {
			throw;
		}
	}
}

void ClientGame::createGame(const std::vector<ILocalPlayerPtr>& players) {
	gameRules_->createGame(players);

	tetris_protocol::ConnectionInfo connection;

	for (auto& localPlayer : players) {
		auto* protoPlayer = connection.add_players();
		updateProtoPlayer(protoPlayer, localPlayer);
	}

	//server_->sendToAll(connection);
}

void ClientGame::restartGame() {
	gameRules_->restartGame();
}

int ClientGame::getNbrAlivePlayers() const {
	return gameRules_->getNbrAlivePlayers();
}

bool ClientGame::isPaused() const {
	return false;
}

void ClientGame::setPaused(bool pause) {

}

void ClientGame::connectHandler(std::error_code code) {

}

void ClientGame::receiveMessage(const tetris_protocol::Wrapper& wrapper) {
	if (wrapper.has_game_info()) {
		handleMessage(wrapper.game_info());
	}
}

void ClientGame::handleMessage(const tetris_protocol::GameInfo& gameInfo) {
	int width = gameInfo.width();
	int height = gameInfo.height();
}

void ClientGame::waitForGameEvents() {
	while (!callbacks_.empty()) {
		callbacks_.pop();
	}
}

void ClientGame::handleDisconnect() {

}
