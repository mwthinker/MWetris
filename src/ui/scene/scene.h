#ifndef MWETRIS_UI_SCENE_SCENE_H
#define MWETRIS_UI_SCENE_SCENE_H

#include "graphic/graphic.h"

#include <sdl/shader.h>

#include <SDL_events.h>
#include <spdlog/spdlog.h>
#include <entt/entt.hpp>

#include <chrono>

namespace mwetris::ui::scene {

	class Scene {
	public:
		friend class StateMachine;

		virtual ~Scene() = default;

		virtual void eventUpdate(const SDL_Event& windowEvent) {};

		virtual void imGuiUpdate(const std::chrono::high_resolution_clock::duration& deltaTime) {};

		virtual void draw(const sdl::Shader& shader, const std::chrono::high_resolution_clock::duration& deltaTime) {};

	protected:
		template <class Event, class... Args>
		void emitEvent(Args&&... args) {
			if (!dispatcher_) {
				spdlog::warn("[Scene] Failed emitEvent, missing dispatcher");
				return;
			}
			dispatcher_->enqueue<Event>(std::forward<Args>(args)...);
		}

	private:
		std::shared_ptr<entt::dispatcher> dispatcher_;
	};

}

#endif
