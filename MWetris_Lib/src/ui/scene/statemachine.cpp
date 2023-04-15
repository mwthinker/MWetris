#include "statemachine.h"

namespace mwetris::ui::scene {

	StateMachine::StateMachine() {
	}

	StateMachine::~StateMachine() {
		if (currentKey_ != 0) {
			scenes_[currentKey_]->switchedFrom();
		}
	}

	void StateMachine::eventUpdate(const SDL_Event& windowEvent) {
		if (currentKey_) {
			scenes_[currentKey_]->eventUpdate(windowEvent);
		}
	}

	void StateMachine::imGuiUpdate(const DeltaTime& deltaTime) {
		if (currentKey_) {
			scenes_[currentKey_]->imGuiUpdate(deltaTime);
		}
	}

	void StateMachine::onCallback(scene::Event event) {
		if (callback_) {
			callback_(event);
		}
	}

	void StateMachine::emitEvent(Event event) {
		if (callback_) {
			callback_(event);
		}
	}
	
	Scene::StateMachineWrapper::StateMachineWrapper(StateMachine* stateMachine)
		: stateMachine_{stateMachine} {
	}

	void Scene::StateMachineWrapper::emitEvent(Event event) {
		if (stateMachine_) {
			stateMachine_->emitEvent(event);
		}
	}

	Scene::StateMachineWrapper::operator bool() const {
		return stateMachine_ != nullptr;
	}

}
