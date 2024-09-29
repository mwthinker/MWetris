#include "gamepad.h"

#include <SDL.h>

namespace app::game {

	GamePad::GamePad(sdl::GameController&& gameController, int rotateButton, int downButton)
		: gameController_ {std::move(gameController)}
		, rotateButton_{rotateButton}
		, downButton_{downButton} {
	}

	Input GamePad::getInput() const {
		return input_;
	}

	const char* GamePad::getName() const {
		return gameController_.getName();
	}

	int GamePad::getInstanceId() const {
		return gameController_.getInstanceId();
	}

	const std::string& GamePad::getGuid() const {
		if (guid_.empty()) {
			guid_ = sdl::guidToString(gameController_.getGuid());
		}
		return guid_;
	}

	bool GamePad::eventUpdate(const SDL_Event& windowEvent) {
		switch (windowEvent.type) {
			case SDL_CONTROLLERBUTTONDOWN:
				if (gameController_.isAttached() && windowEvent.cbutton.which == gameController_.getInstanceId()) {
					return updateInput(windowEvent.cbutton.button, true);
				}
				break;
			case SDL_CONTROLLERBUTTONUP:
				if (gameController_.isAttached() && windowEvent.cbutton.which == gameController_.getInstanceId()) {
					return updateInput(windowEvent.cbutton.button, false);
				}
				break;
		}
		return false;
	}

	void GamePad::setGameController(sdl::GameController&& gameController) {
		gameController_ = std::move(gameController);
	}

	bool GamePad::updateInput(Uint8 button, bool state) {
		switch (button) {
			case SDL_CONTROLLER_BUTTON_A:
				input_.downGround = state;
				return true;
			case SDL_CONTROLLER_BUTTON_X:
				input_.rotate = state;
				return true;
			case SDL_CONTROLLER_BUTTON_DPAD_UP:
				input_.rotate = state;
				return true;
			case SDL_CONTROLLER_BUTTON_DPAD_DOWN:
				input_.down = state;
				return true;
			case SDL_CONTROLLER_BUTTON_DPAD_LEFT:
				input_.left = state;
				return true;
			case SDL_CONTROLLER_BUTTON_DPAD_RIGHT:
				input_.right = state;
				return true;
		}
		return false;
	}

}