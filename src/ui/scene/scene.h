#ifndef MWETRIS_UI_SCENE_SCENE_H
#define MWETRIS_UI_SCENE_SCENE_H

#include "event.h"

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

		virtual void eventUpdate(const SDL_Event& windowEvent) {};

		virtual void imGuiUpdate(const DeltaTime& deltaTime) {};

	protected:
		class StateMachineWrapper {
		public:
			StateMachineWrapper() = default;
			StateMachineWrapper(StateMachine* stateMachine);

			void emitEvent(Event event);

			explicit operator bool() const;

		private:
			StateMachine* stateMachine_ = nullptr;
		};

		void emitEvent(Event event) {
			if (!stateMachine_)
			{
				spdlog::warn("[Scene] Failed emitEvent, missing dispatcher");
				return;
			}
			stateMachine_.emitEvent(event);
		}

	private:
		virtual void switchedFrom() {};

		virtual void switchedTo() {};

		StateMachineWrapper stateMachine_;
	};

}

#endif
