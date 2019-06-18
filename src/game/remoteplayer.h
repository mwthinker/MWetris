#ifndef REMOTEPLAYER_H
#define REMOTEPLAYER_H

#include "player.h"
#include "protocol.h"
#include "tetrisboard.h"

namespace tetris {

	class RemotePlayer : public IPlayer {
	public:
		

		virtual ~RemotePlayer() = default;

		virtual void update(double deltaTime);

		virtual const TetrisBoard& getTetrisBoard() const;

		//virtual void update(Move move, IBoardEventHandler& eventHandler) = 0;

		//virtual void update(Move move, const std::function<void(BoardEvent)>& func) = 0;

		virtual void update(Move move);

		virtual void addRow(int holes);

		virtual void updateName(const std::string& name);

		virtual void updateLevelUpCounter(int counter);

		virtual void updateLevel(int level);

		virtual void updatePoints(int points);

		virtual void updateClearRows(int clearRows);

		virtual void updateRestart();

		virtual void updateGameOver();

		virtual void updateGameOverPosition(int gameOverPosition);

	private:
		RemotePlayer();

		std::string name_;
		int points_, level_, levelUpCounter_;
		TetrisBoard tetrisBoard_;
		int gameOverPosition_;
		mw::signals::Connection connection_;
	};

}

#endif // REMOTEPLAYER_H
