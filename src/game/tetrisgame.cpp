#include "tetrisgame.h"

#include "tetrisboard.h"
#include "tetrisgameevent.h"
#include "localplayer.h"
#include "remoteplayer.h"
#include "tetrisparameters.h"
#include "gamerules.h"
#include "network.h"

#include "localgame.h"

//#include <net/packet.h>

#include <vector>
#include <algorithm>

using namespace tetris;

const double TetrisGame::FIXED_TIMESTEP = 1.0 / 60;

TetrisGame::TetrisGame()
	: status_{WAITING_TO_CONNECT}
	, width_{TETRIS_WIDTH}
	, height_{TETRIS_HEIGHT}
	, maxLevel_{TETRIS_MAX_LEVEL}
	, timeLeftToStart_{-0.0}
	, wholeTimeLeft_{0}
	, game_{std::make_unique<LocalGame>(eventHandler_)} {
}

TetrisGame::~TetrisGame() {
	closeGame();
}

void TetrisGame::resumeGame(int columns, int rows, const std::vector<PlayerData>& playersData) {
	width_ = columns;
	height_ = rows;
	status_ = LOCAL;
	
	players_.clear();
	for (const PlayerData& data : playersData) {
		LocalPlayerBuilder builder;
		builder.widthBoard(data.board_);
		builder.widthClearedRows(data.clearedRows_);
		builder.widthDevice(data.device_);
		builder.widthGameOverPosition(data.lastPosition_);
		builder.widthLevel(data.level_);
		builder.widthLevelUpCounter(data.levelUpCounter_);
		builder.widthMovingBlock(data.current_);
		builder.widthName(data.name_);
		builder.widthNextBlockType(data.next_);
		builder.widthPoints(data.points_);
		builder.withHeight(height_);
		builder.withWidth(width_);
		players_.push_back(builder.build());
	}

	game_->createGame(players_);
	initGame();
}

std::vector<PlayerData> TetrisGame::getPlayerData() const {
	std::vector<PlayerData> playerData;
	
	for (const std::shared_ptr<ILocalPlayer>& player : players_) {
		playerData.emplace_back();
		const TetrisBoard& tetrisBoard = player->getTetrisBoard();
		playerData.back().current_ = tetrisBoard.getBlock();
		playerData.back().lastPosition_ = player->getGameOverPosition();
		playerData.back().next_ = tetrisBoard.getNextBlockType();
		playerData.back().board_ = tetrisBoard.getBoardVector();
		playerData.back().levelUpCounter_ = player->getLevelUpCounter();
		playerData.back().level_ = player->getLevel();
		playerData.back().clearedRows_ = player->getClearedRows();
		playerData.back().name_ = player->getName();
		playerData.back().points_ = player->getPoints();
		playerData.back().device_ = player->getDevice();
	}

	return playerData;
}

void TetrisGame::createLocalPlayers(int columns, int rows, const std::vector<IDevicePtr>& devices) {
	players_.clear();
	LocalPlayerBuilder builder;
	for (const auto& device : devices) {
		builder.widthDevice(device);
		builder.widthClearedRows(0);
		builder.widthGameOverPosition(0);
		builder.widthLevel(1);
		builder.widthLevelUpCounter(0);
		//builder.widthMovingBlock(data.current_);
		//builder.widthName(data.name_);
		builder.widthMovingBlockType(randomBlockType());
		builder.widthNextBlockType(randomBlockType());
		builder.widthPoints(0);
		builder.withHeight(height_);
		builder.withWidth(width_);

		auto player = builder.build();
		players_.push_back(player);
	}
}

void TetrisGame::receiveRemotePlayers(const std::vector<std::shared_ptr<RemotePlayer>>& players) {

}

void TetrisGame::createLocalGame(int columns, int rows, const std::vector<IDevicePtr>& devices) {
	status_ = LOCAL;

	width_ = columns;
	height_ = rows;
		
	createLocalPlayers(columns, rows, devices);
	game_->createGame(players_);
	initGame();
}

void TetrisGame::createGame(const std::vector<IDevicePtr>& devices) {
	switch (status_) {
		case TetrisGame::Status::LOCAL:
			createLocalGame(width_, height_, devices);
			break;
		case TetrisGame::Status::SERVER:
			createLocalPlayers(width_, height_, devices);
			game_->createGame(players_);
			break;
		case TetrisGame::Status::CLIENT:
			break;
	}
}

void TetrisGame::createServerGame(int port, int columns, int rows, const std::vector<IDevicePtr>& devices) {
	if (status_ == WAITING_TO_CONNECT) {
		auto serverGame = std::make_unique<ServerGame>(eventHandler_);
		serverGame->connect(port);
		game_ = std::move(serverGame);

		// game_ 
		width_ = columns;
		height_ = rows;
		status_ = SERVER;

		createLocalPlayers(width_, height_, devices);
		game_->createGame(players_);
		initGame();
	}
}

void TetrisGame::createClientGame(int port, std::string ip) {
	if (status_ == WAITING_TO_CONNECT) {
		auto clientGame = std::make_unique<ServerGame>(eventHandler_);
		status_ = CLIENT;
	}
}

void TetrisGame::initGame() {
	triggerGameStartEvent();

	if (game_->isPaused()) {
		game_->setPaused(false);
		eventHandler_(GamePause{game_->isPaused(), false});
	}

	triggerTriggerInitGameEvent();

	startNewCountDown();
}

void TetrisGame::startNewCountDown() {
	// Must be called last, after all settings are defined for the current game.
	if (currentGameHasCountDown()) {
		timeLeftToStart_ = COUNT_DOWN_TIME;
		wholeTimeLeft_ = COUNT_DOWN_TIME;
		eventHandler_(CountDown{wholeTimeLeft_});
	}
}

void TetrisGame::restartGame() {
	if (status_ != Status::WAITING_TO_CONNECT) {
		game_->restartGame();
		initGame();
	}
}

// Stop the game and abort any active connection.
void TetrisGame::closeGame() {
	status_ = WAITING_TO_CONNECT;
	
}

bool TetrisGame::isPaused() const {
	return game_->isPaused();
}

void TetrisGame::pause() {
	game_->setPaused(!game_->isPaused());
	eventHandler_(GamePause{game_->isPaused(), true});
}

int TetrisGame::getNbrOfPlayers() const {
	return players_.size();
}

void TetrisGame::resizeBoard(int width, int height) {
	if (width > TETRIS_MIN_WIDTH && width <= TETRIS_MAX_WIDTH &&
		height > TETRIS_MIN_HEIGHT && height <= TETRIS_MAX_HEIGHT &&
		(width_ != width || height_ != height)) {

		width_ = width;
		height_ = height;

		game_->createGame(players_);
		initGame();
	}
}

void TetrisGame::update(double deltaTime) {
	if (status_ != Status::WAITING_TO_CONNECT && !game_->isPaused()) {
		updateCurrentCountDown(deltaTime);

		if (!hasActiveCountDown()) {
			updateGame(deltaTime);
		}
	}
}

void TetrisGame::updateCurrentCountDown(double deltaTime) {
	if (timeLeftToStart_ > 0) {
		timeLeftToStart_ -= deltaTime;
	}

	if (currentGameHasCountDown() && wholeTimeLeft_ != (int) (timeLeftToStart_ + 1)
		&& wholeTimeLeft_ > timeLeftToStart_ + 1) {

		wholeTimeLeft_ = (int) (timeLeftToStart_ + 1);
		eventHandler_(CountDown{wholeTimeLeft_});
	}
}

bool TetrisGame::hasActiveCountDown() const {
	return players_.size() > 1 && currentGameHasCountDown() && timeLeftToStart_ > 0;
}

void TetrisGame::updateGame(double deltaTime) {
	if (deltaTime > 0.250) {
		// To avoid spiral of death.
		deltaTime = 0.250;
	}
	
	// Update using fixed time step.
	while (deltaTime >= FIXED_TIMESTEP) {
		deltaTime -= FIXED_TIMESTEP;
		for (auto& player : players_) {
			player->update(FIXED_TIMESTEP);
		}
	}
	
	// Update the remaining time.
	for (auto& player : players_) {
		player->update(deltaTime);
	}
}

bool TetrisGame::isCurrentGameActive() const {
	return game_->getNbrAlivePlayers() > 0;
}

void TetrisGame::triggerGameStartEvent() {
	switch (status_) {
		case TetrisGame::LOCAL:
			eventHandler_(GameStart{GameStart::LOCAL});
			break;
		case TetrisGame::CLIENT:
			eventHandler_(GameStart{GameStart::CLIENT});
			break;
		case TetrisGame::SERVER:
			eventHandler_(GameStart{GameStart::SERVER});
			break;
	}
}

void TetrisGame::triggerTriggerInitGameEvent() {
	std::vector<IPlayerPtr> players;
	for (const auto& p : players_) {
		players.push_back(p);
	}
	eventHandler_(InitGame{players});
}
