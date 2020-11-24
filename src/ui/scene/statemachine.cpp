#include "statemachine.h"

namespace mwetris::ui::scene {

	StateMachine::StateMachine()
		: dispatcher_{std::make_shared<entt::dispatcher>()} {

		dispatcher_->sink<scene::Event>().connect<&StateMachine::onCallback>(*this);
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
		dispatcher_->update();
	}

	void StateMachine::draw(const sdl::Shader& shader, const std::chrono::high_resolution_clock::duration& deltaTime) {
		if (currentKey_) {
			scenes_[currentKey_]->draw(shader, deltaTime);
		}
	}

	void StateMachine::onCallback(scene::Event event) {
		if (callback_) {
			callback_(event);
		}
	}

}
