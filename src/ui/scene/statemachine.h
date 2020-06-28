#ifndef MWETRIS_UI_SCENE_STATEMACHINE_H
#define MWETRIS_UI_SCENE_STATEMACHINE_H

#include "scene.h"

#include <entt/entt.hpp>
#include <SDL_events.h>
#include <spdlog/spdlog.h>

#include <memory>
#include <set>

namespace tetris::ui::scene {

	class StateMachine {
	public:
		StateMachine(std::shared_ptr<entt::dispatcher> dispatcher);

		void eventUpdate(const SDL_Event& windowEvent);

		void imGuiUpdate(const std::chrono::high_resolution_clock::duration& deltaTime);

		void draw(const std::chrono::high_resolution_clock::duration& deltaTime);
		
		template <class Type>
		std::shared_ptr<Type> add(std::shared_ptr<Type> scene);

		template <class Type, class... Args>
		std::shared_ptr<Type> emplace(Args&&... args);

		void switchTo(const std::shared_ptr<Scene>& scene);

		void remove(const std::shared_ptr<Scene>& scene);

	private:
		std::set<std::shared_ptr<Scene>> scenes_;
		
		std::shared_ptr<Scene> currentScene_;
		std::shared_ptr<entt::dispatcher> dispatcher_;
	};

	template <class Type>
	std::shared_ptr<Type> StateMachine::add(std::shared_ptr<Type> scene) {
		static_assert(std::is_base_of<Scene, Type>::value,
			"Type must have Scene as base class");

		if (scene) {
			if (scenes_.empty()) {
				currentScene_ = scene;
			}

			static_cast<Scene&>(*scene).dispatcher_ = dispatcher_;
			scenes_.insert(scene);
			return std::move(scene);
		} else {
			spdlog::warn("[SceneStateMachine] Tried to add empty scene!");
		}
		return scene;
	}

	template <class Type, class... Args>
	std::shared_ptr<Type> StateMachine::emplace(Args&&... args) {
		return add(std::make_shared<Type>(std::forward<Args>(args)...));
	}

}

#endif
