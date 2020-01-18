#include "remoteplayer.h"

using namespace tetris;

RemotePlayer::RemotePlayer()
	: tetrisBoard_{10,10, BlockType::I, BlockType::I} {
}

void RemotePlayer::update(double deltaTime) {
	
}

void RemotePlayer::update(Move move) {

}

const TetrisBoard& RemotePlayer::getTetrisBoard() const {
	return tetrisBoard_;
}

void RemotePlayer::addRow(int holes) {
	std::vector<BlockType> blockTypes = generateRow(tetrisBoard_.getColumns(), 2);
	tetrisBoard_.addRows(blockTypes);
}

void RemotePlayer::updateName(const std::string& name) {
	name_ = name;
}

void RemotePlayer::updatePoints(int points) {
	points_ = points;
}

void RemotePlayer::updateLevelUpCounter(int counter) {
	levelUpCounter_ = counter;
}

void RemotePlayer::updateClearRows(int counter) {
	
}

void RemotePlayer::updateLevel(int level) {
	level_ = level;
}

void RemotePlayer::updateGameOverPosition(int gameOverPosition) {
	gameOverPosition_ = gameOverPosition;
}

void RemotePlayer::updateRestart() {
	level_ = 1;
	points_ = 0;
	gameOverPosition_ = 0;
	levelUpCounter_ = 0;
	tetrisBoard_.updateRestart(randomBlockType(), randomBlockType());
}

void RemotePlayer::updateGameOver() {
	tetrisBoard_.update(Move::GAME_OVER);
}

/*
#include <net/connection.h>
#include <net/packet.h>

RemotePlayer::RemotePlayer(int id, int width, int height, bool ai, BlockType current, BlockType next) :
	Player(id, width, height, current, next), ai_(ai) {

}

void RemotePlayer::receive(net::Packet& packet) {
	packet.reset();
	PacketType type;
	packet >> type;
	int id; // Not used;
	packet >> id;
	int playerId; // Not used;
	packet >> playerId;
	switch (type) {
		case PacketType::PLAYER_MOVE:
		{
			Move move;
			packet >> move;
			BlockType next;
			packet >> next;
			tetrisBoard_.update(move);
			tetrisBoard_.updateNextBlock(next);
			break;
		}
		case PacketType::PLAYER_TETRIS:
		{
			std::vector<BlockType> blockTypes;
			while (packet.dataLeftToRead() > 0) {
				BlockType type;
				packet >> type;
				blockTypes.push_back(type);
			}
			tetrisBoard_.addRows(blockTypes);
			break;
		}
		case PacketType::PLAYER_NAME:
			packet >> name_;
			break;
		case PacketType::PLAYER_LEVEL:
		{
			int level;
			packet >> level;
			setLevel(level);
			break;
		}			
	}
}

void RemotePlayer::resizeBoard(int width, int height) {
	tetrisBoard_.updateRestart(width, height, tetrisBoard_.getBlockType(), tetrisBoard_.getNextBlockType());
	level_ = 1;
	points_ = 0;
}

void RemotePlayer::restart(BlockType current, BlockType next) {
	tetrisBoard_.restart(current, next);
	level_ = 1;
	points_ = 0;
}

void RemotePlayer::endGame() {
	tetrisBoard_.update(Move::GAME_OVER);
}

*/