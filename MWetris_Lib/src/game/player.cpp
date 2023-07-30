#include "player.h"
#include "tetrisgameevent.h"
#include "helper.h"

#include <random>

namespace {

	constexpr int UniqueIdSize = 8;

	constexpr std::string_view Characters = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

	std::string generateRandomString(int size) {
		static std::mt19937 generator{std::random_device{}()};
		static std::uniform_int_distribution<> distribution{0, static_cast<int>(Characters.size() - 1)};

		std::string unique(size, 'X');

		for ([[maybe_unused]] auto _ : unique) {
			unique = Characters[distribution(generator)];
		}

		return unique;
	}

}


namespace mwetris::game {

	Player::Player(const tetris::TetrisBoard& tetrisBoard, const std::string& name)
		: tetrisBoard_{tetrisBoard}
		, name_{name}
		, uniqueId_{generateRandomString(UniqueIdSize)} {
	}

	bool Player::isGameOver() const {
		return tetrisBoard_.isGameOver();
	}

	void Player::restartTetrisBoard(tetris::BlockType current, tetris::BlockType next) {
		tetrisBoard_.restart(current, next);
	}

	void Player::setNextTetrisBlock(tetris::BlockType next) {
		tetrisBoard_.setNextBlock(next);
	}

	void Player::updateTetrisBoard(tetris::Move move) {
		tetrisBoard_.update(move, [this](tetris::BoardEvent boardEvent, int value) {
			handleBoardEvent(boardEvent, value);
		});
	}

	void Player::handleBoardEvent(tetris::BoardEvent event, int value) {
		gameboardEventUpdate.invoke(event, value);
	}

	const std::string& Player::getUniqueId() const {
		return uniqueId_;
	}
}
