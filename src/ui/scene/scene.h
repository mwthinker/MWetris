#ifndef MWETRIS_UI_SCENE_SCENE_H
#define MWETRIS_UI_SCENE_SCENE_H

#include "graphic/graphic.h"
#include "event.h"

#include <sdl/shader.h>

#include <SDL_events.h>
#include <spdlog/spdlog.h>

#include <chrono>

namespace mwetris::ui::scene {

	class StateMachine;

	class Scene {
	public:
		friend class StateMachine;

		virtual ~Scene() = default;

		virtual void eventUpdate(const SDL_Event& windowEvent) {};

		virtual void imGuiUpdate(const std::chrono::high_resolution_clock::duration& deltaTime) {};

		virtual void draw(const sdl::Shader& shader, const std::chrono::high_resolution_clock::duration& deltaTime) {};

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
