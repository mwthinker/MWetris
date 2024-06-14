#include "statemachine.h"

namespace mwetris::ui::scene {

	StateMachine::StateMachine() {
	}

	StateMachine::~StateMachine() {
		if (currentKey_ != 0) {
			scenes_[currentKey_]->switchedFrom();
		}
	}

	void StateMachine::imGuiUpdate(const DeltaTime& deltaTime) {
		if (currentKey_) {
			scenes_[currentKey_]->imGuiUpdate(deltaTime);
		}
	}
	
	Scene::StateMachineWrapper::StateMachineWrapper(StateMachine* stateMachine)
		: stateMachine_{stateMachine} {
	}

	Scene::StateMachineWrapper::operator bool() const {
		return stateMachine_ != nullptr;
	}

}
