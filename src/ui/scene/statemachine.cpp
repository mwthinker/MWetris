#include "statemachine.h"

namespace tetris::ui::scene {

	StateMachine::StateMachine(std::shared_ptr<entt::dispatcher> dispatcher)
		: dispatcher_{std::move(dispatcher)} {
	}

	void StateMachine::eventUpdate(const SDL_Event& windowEvent) {
		if (currentScene_) {
			currentScene_->eventUpdate(windowEvent);
		}
	};

	void StateMachine::imGuiUpdate(const std::chrono::high_resolution_clock::duration& deltaTime) {
		if (currentScene_) {
			currentScene_->imGuiUpdate(deltaTime);
		}
	}

	void StateMachine::draw(const std::chrono::high_resolution_clock::duration& deltaTime) {
		if (currentScene_) {
			currentScene_->draw(deltaTime);
		}
	}

}
