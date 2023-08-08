#include "remoteplayerboard.h"
#include "actionhandler.h"
#include "tetrisgameevent.h"

#include <string>
#include <functional>

namespace mwetris::game {

	RemotePlayerBoard::RemotePlayerBoard(const tetris::TetrisBoard& board, const std::string& name)
		: PlayerBoard{board, name} {
	}

	void RemotePlayerBoard::handleBoardEvent(tetris::BoardEvent boardEvent, int value) {
		PlayerBoard::handleBoardEvent(boardEvent, value);
		if (boardEvent == tetris::BoardEvent::CurrentBlockUpdated) {
			
		}
		if (boardEvent == tetris::BoardEvent::RowsRemoved) {
			clearedRows_ += value;
		}
	}

}
