#ifndef MWETRIS_UI_SCENE_SCENE_H
#define MWETRIS_UI_SCENE_SCENE_H

#include "graphic/graphic.h"
#include "event.h"

#include <sdl/shader.h>

#include <SDL_events.h>
#include <spdlog/spdlog.h>

#include <chrono>

namespace mwetris::ui::scene {

	using IdType = size_t;

	template<typename T>
	struct TypeInfo {
		static IdType id() {
			// Generate a uniqe id per type. reinterpret_cast makes it impossible to make it constexpr.
			return reinterpret_cast<size_t>(&TypeInfo<T>::id);
		}
	};

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
