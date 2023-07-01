#include "statemachine.h"

namespace mwetris::ui::scene {

	StateMachine::StateMachine() {
	}

	StateMachine::~StateMachine() {
		if (currentKey_ != 0) {
			scenes_[currentKey_]->switchedFrom();
		}
	}

	bool StateMachine::eventUpdate(const SDL_Event& windowEvent) {
		if (!firstRun_ && currentKey_) {
			return scenes_[currentKey_]->eventUpdate(windowEvent);
		}
		return true;
	}

	void StateMachine::imGuiUpdate(const DeltaTime& deltaTime) {
		if (currentKey_) {
			auto& scene = *scenes_[currentKey_];
			if (firstRun_) {
				firstRun_ = false;
				scene.switchedTo();
			}

			scene.imGuiUpdate(deltaTime);
		}
	}
	
	Scene::StateMachineWrapper::StateMachineWrapper(StateMachine* stateMachine)
		: stateMachine_{stateMachine} {
	}

	Scene::StateMachineWrapper::operator bool() const {
		return stateMachine_ != nullptr;
	}

}
