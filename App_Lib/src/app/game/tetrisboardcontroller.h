#ifndef APP_GAME_TETRISBOARDCONTROLLER_H
#define APP_GAME_TETRISBOARDCONTROLLER_H

#include "input.h"
#include "actionhandler.h"
#include "playerboardevent.h"

#include <tetris/tetrisboard.h>

namespace app::game {

	class TetrisBoardController {
	public:
		TetrisBoardController() = default;

		void update(tetris::TetrisBoard& tetriBoard, Input input, double deltaTime, PlayerBoardEventCallback auto&& eventCallback) {
			// The time between each "gravity" move.
			double downTime = 1.0 / speed_;
			gravityMove_.setWaitingTime(downTime);

			gravityMove_.update(deltaTime, true);
			if (gravityMove_.doAction()) {
				update(tetriBoard, tetris::Move::DownGravity, eventCallback);
			}

			leftHandler_.update(deltaTime, input.left && !input.right);
			if (leftHandler_.doAction()) {
				update(tetriBoard, tetris::Move::Left, eventCallback);
			}

			rightHandler_.update(deltaTime, input.right && !input.left);
			if (rightHandler_.doAction()) {
				update(tetriBoard, tetris::Move::Right, eventCallback);
			}

			downHandler_.update(deltaTime, input.down);
			if (downHandler_.doAction()) {
				update(tetriBoard, tetris::Move::Down, eventCallback);
			}

			rotateHandler_.update(deltaTime, input.rotate);
			if (rotateHandler_.doAction()) {
				update(tetriBoard, tetris::Move::RotateLeft, eventCallback);
			}

			downGroundHandler_.update(deltaTime, input.downGround);
			if (downGroundHandler_.doAction()) {
				update(tetriBoard, tetris::Move::DownGround, eventCallback);
			}
		}

		void reset() {
			leftHandler_.reset();
			rightHandler_.reset();
			rotateHandler_.reset();
			downGroundHandler_.reset();
			downHandler_.reset();
			gravityMove_.reset();
		}

		void updateGravity(float speed) {
			speed_ = speed;
		}

	private:
		void update(tetris::TetrisBoard& tetrisBoard, tetris::Move move, PlayerBoardEventCallback auto&& eventCallback) {
			eventCallback(UpdateMove{move});
			tetrisBoard.update(move, [&](tetris::BoardEvent boardEvent, int value) {
				if (boardEvent == tetris::BoardEvent::CurrentBlockUpdated) {
					leftHandler_.reset();
					rightHandler_.reset();
					downHandler_.reset();
				}
				eventCallback(
					TetrisBoardEvent{
						.event = boardEvent,
						.value = value
					}
				);
			});
		}

		ActionHandler leftHandler_{0.09, false};
		ActionHandler rightHandler_{0.09, false};
		ActionHandler rotateHandler_{0.0, true};
		ActionHandler downGroundHandler_{0.0, true};
		ActionHandler gravityMove_{1, false};  // Value doesn't matter! Changes every frame.
		ActionHandler downHandler_{0.04, false};

		float speed_ = 1.f;
	};

}

#endif
