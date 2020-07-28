#include "statemachine.h"

namespace mwetris::ui::scene {

	StateMachine::StateMachine(std::shared_ptr<entt::dispatcher> dispatcher)
		: dispatcher_{std::move(dispatcher)} {
	}

	void StateMachine::eventUpdate(const SDL_Event& windowEvent) {
		if (currentKey_) {
			scenes_[currentKey_]->eventUpdate(windowEvent);
		}
	};

	void StateMachine::imGuiUpdate(const std::chrono::high_resolution_clock::duration& deltaTime) {
		if (currentKey_) {
			scenes_[currentKey_]->imGuiUpdate(deltaTime);
		}
	}

	void StateMachine::draw(const sdl::Shader& shader, const std::chrono::high_resolution_clock::duration& deltaTime) {
		if (currentKey_) {
			scenes_[currentKey_]->draw(shader, deltaTime);
		}
	}

}
