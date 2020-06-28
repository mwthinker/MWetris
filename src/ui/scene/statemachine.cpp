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
	
	void StateMachine::switchTo(const std::shared_ptr<Scene>& scene) {
		if (auto it = scenes_.find(scene); it != scenes_.end()) {
			currentScene_ = scene;
		} else {
			spdlog::warn("[SceneStateMachine] Failed to switch scene.");
		}
	}

	void StateMachine::remove(const std::shared_ptr<Scene>& scene) {
		if (auto it = scenes_.find(scene); it != scenes_.end()) {
			if (currentScene_ == scene) {
				currentScene_ = nullptr;
			}
			scenes_.erase(it);
		} else {
			spdlog::warn("[SceneStateMachine] Failed to remove scene.");
		}
	}

}
