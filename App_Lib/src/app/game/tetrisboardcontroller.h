#ifndef APP_GAME_TETRISBOARDCONTROLLER_H
#define APP_GAME_TETRISBOARDCONTROLLER_H

#include "input.h"
#include "actionhandler.h"
#include "dasarrhandler.h"
#include "playerboardevent.h"
#include "../ringbuffer.h"

#include <tetris/tetrisboard.h>

#include <spdlog/spdlog.h>

#include <cstdint>
#include <optional>

namespace app::game {

	class TetrisBoardController {
	public:
		TetrisBoardController() = default;

		[[nodiscard]] bool enqueue(InputCommand command) {
			if (awaitingNextPiece()) {
				switch (command.action) {
					case InputAction::Left:
					case InputAction::Right:
						return false;
					case InputAction::Rotate:
						if (bufferedRotation_) {
							return false;
						}
						command.deferredUntilNextPiece = true;
						bufferedRotation_ = command;
						return true;
					case InputAction::SoftDrop:
					case InputAction::HardDrop:
						return false;
				}
			}
			if (pendingCommands_.tryPush(command)) {
				if (command.action == InputAction::HardDrop) {
					hardDropQueued_ = true;
				}
				return true;
			}
			++overflowCount_;
			spdlog::warn("[InputQueue] rejected newest command; overflow count={}", overflowCount_);
			return false;
		}

		void update(tetris::TetrisBoard& tetrisBoard, Input input, double deltaTime, PlayerBoardEventCallback auto&& eventCallback) {
			update(tetrisBoard, input, deltaTime, eventCallback, [](const InputCommand&) {});
		}

		void update(tetris::TetrisBoard& tetriBoard, Input input, double deltaTime, PlayerBoardEventCallback auto&& eventCallback, auto&& commandConsumed) {
			if (areActive_) {
				areTimer_ -= deltaTime;
				if (areTimer_ <= 0.0) {
					areActive_ = false;
					// Spawn next piece here, or signal board to do so
					
					update(tetriBoard, tetris::Move::DownGravity, eventCallback);
					if (input.left) {
						leftHandler_.press();
						update(tetriBoard, tetris::Move::Left, eventCallback);
					}
					if (input.right) {
						rightHandler_.press();
						update(tetriBoard, tetris::Move::Right, eventCallback);
					}
					areTimer_ = 0.0;
				}
				return; // Skip input and gravity while ARE is active
			}

			while (!pendingCommands_.empty()) {
				const auto command = pendingCommands_.front();
				pendingCommands_.pop();
				dispatch(tetriBoard, command.action, eventCallback);
				commandConsumed(command);
			}
			
			// The time between each "gravity" move.
			double downTime = 1.0 / speed_;
			gravityMove_.setWaitingTime(downTime);

			gravityMove_.update(deltaTime, true);
			if (gravityMove_.doAction()) {
				if (tetriBoard.isBlockDown()) {
					areActive_ = true;
					areTimer_ = ARE_DURATION;
				} else {
					update(tetriBoard, tetris::Move::DownGravity, eventCallback);
				}
			}
			if (awaitingNextPiece()) {
				return;
			}

			if (spawnedFromAre_) {
				if (input.down) {
					downHandler_.press();
				}
				spawnedFromAre_ = false;
			}

			leftHandler_.update(deltaTime, input.left);
			if (leftHandler_.doAction()) {
				update(tetriBoard, tetris::Move::Left, eventCallback);
			}

			rightHandler_.update(deltaTime, input.right);
			if (rightHandler_.doAction()) {
				update(tetriBoard, tetris::Move::Right, eventCallback);
			}
			
			downHandler_.update(deltaTime, input.down);
			if (downHandler_.doAction()) {
				update(tetriBoard, tetris::Move::Down, eventCallback);
			}
		}

		void reset() {
			leftHandler_.reset();
			rightHandler_.reset();
			downHandler_.reset();
			gravityMove_.reset();
			pendingCommands_.clear();
			hardDropQueued_ = false;
			bufferedRotation_.reset();
			spawnedFromAre_ = false;
			areActive_ = false;
			areTimer_ = 0.0;
		}

		[[nodiscard]] int pendingCommandCount() const {
			return pendingCommands_.size();
		}

		[[nodiscard]] std::uint64_t overflowCount() const {
			return overflowCount_;
		}

		void setInputTiming(double das, double arr) {
			leftHandler_.setTiming(das, arr);
			rightHandler_.setTiming(das, arr);
		}

		void updateGravity(float speed) {
			speed_ = speed;
		}

	private:
		[[nodiscard]] bool awaitingNextPiece() const {
			return hardDropQueued_ || areActive_;
		}

		void queueBufferedRotation() {
			if (!bufferedRotation_) {
				return;
			}
			if (!pendingCommands_.tryPush(*bufferedRotation_)) {
				++overflowCount_;
				spdlog::warn("[InputQueue] rejected buffered rotation; overflow count={}", overflowCount_);
			}
			bufferedRotation_.reset();
		}

		void dispatch(tetris::TetrisBoard& board, InputAction action, PlayerBoardEventCallback auto&& eventCallback) {
			switch (action) {
				case InputAction::Left:
					leftHandler_.press();
					update(board, tetris::Move::Left, eventCallback);
					break;
				case InputAction::Right:
					rightHandler_.press();
					update(board, tetris::Move::Right, eventCallback);
					break;
				case InputAction::SoftDrop:
					downHandler_.press();
					update(board, tetris::Move::Down, eventCallback);
					break;
				case InputAction::Rotate:
					update(board, tetris::Move::RotateLeft, eventCallback);
					break;
				case InputAction::HardDrop:
					update(board, tetris::Move::DownGround, eventCallback);
					break;
			}
		}

		void update(tetris::TetrisBoard& tetrisBoard, tetris::Move move, PlayerBoardEventCallback auto&& eventCallback) {
			eventCallback(UpdateMove{move});
				tetrisBoard.update(move, [&](tetris::BoardEvent boardEvent, int value) {
				if (boardEvent == tetris::BoardEvent::CurrentBlockUpdated) {
					leftHandler_.reset();
					rightHandler_.reset();
					downHandler_.reset();
					hardDropQueued_ = false;
					spawnedFromAre_ = true;
					queueBufferedRotation();
				}
				eventCallback(
					TetrisBoardEvent{
						.event = boardEvent,
						.value = value
					}
				);
			});
		}

		DasArrHandler leftHandler_{0.167, 0.033};
		DasArrHandler rightHandler_{0.167, 0.033};
		ActionHandler gravityMove_{1, false};  // Value doesn't matter! Changes every frame.
		DasArrHandler downHandler_{0.04, 0.04};
		app::RingBuffer<InputCommand, 16> pendingCommands_;
		std::uint64_t overflowCount_ = 0;
		bool hardDropQueued_ = false;
		std::optional<InputCommand> bufferedRotation_;
		bool spawnedFromAre_ = false;

		float speed_ = 1.f;

		// Active ARE (Auto Repeat Enable) state
		// This is used to prevent input from being processed during the ARE phase.
		bool areActive_ = false;
		double areTimer_ = 0.0;
		constexpr static double ARE_DURATION = 0.5; // 0.5 seconds, adjust as needed
	};

}

#endif
