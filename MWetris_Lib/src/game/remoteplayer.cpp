#include "remoteplayer.h"
#include "actionhandler.h"
#include "tetrisgameevent.h"

#include <string>
#include <functional>

namespace mwetris::game {

	RemotePlayer::RemotePlayer(const tetris::TetrisBoard& board, const std::string& name)
		: Player{board, name} {}



	void RemotePlayer::handleBoardEvent(tetris::BoardEvent boardEvent, int value) {
		Player::handleBoardEvent(boardEvent, value);
		if (boardEvent == tetris::BoardEvent::CurrentBlockUpdated) {
			
		}
		if (boardEvent == tetris::BoardEvent::RowsRemoved) {
			clearedRows_ += value;
		}
	}

}
