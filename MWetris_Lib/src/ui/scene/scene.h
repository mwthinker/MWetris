#ifndef MWETRIS_UI_SCENE_SCENE_H
#define MWETRIS_UI_SCENE_SCENE_H

#include <sdl/shader.h>

#include <SDL_events.h>
#include <spdlog/spdlog.h>

#include <chrono>

namespace mwetris::ui::scene {

	class StateMachine;

	using DeltaTime = std::chrono::high_resolution_clock::duration;

	class Scene {
	public:
		friend class StateMachine;

		virtual ~Scene() = default;

		// Handle event updates.
		// return true when event should bubble up else false.
		virtual bool eventUpdate(const SDL_Event& windowEvent) {
			return true;
		};

		virtual void imGuiUpdate(const DeltaTime& deltaTime) {};

	protected:
		class StateMachineWrapper {
		public:
			StateMachineWrapper() = default;
			StateMachineWrapper(StateMachine* stateMachine);

			explicit operator bool() const;

		private:
			StateMachine* stateMachine_ = nullptr;
		};

	private:
		virtual void switchedFrom() {};

		virtual void switchedTo() {};

		StateMachineWrapper stateMachine_;
	};

}

#endif
