#ifndef MWETRIS_UI_SCENE_STATEMACHINE_H
#define MWETRIS_UI_SCENE_STATEMACHINE_H

#include "scene.h"

#include <sdl/shader.h>

#include <entt/entt.hpp>
#include <SDL_events.h>
#include <spdlog/spdlog.h>

#include <memory>
#include <map>
#include <typeindex>
#include <algorithm>

namespace tetris::ui::scene {

	class StateMachine {
	public:
		explicit StateMachine(std::shared_ptr<entt::dispatcher> dispatcher);

		void eventUpdate(const SDL_Event& windowEvent);

		void imGuiUpdate(const std::chrono::high_resolution_clock::duration& deltaTime);

		void draw(const sdl::Shader& shader, const std::chrono::high_resolution_clock::duration& deltaTime);
		
		template <class Type>
		std::shared_ptr<Type> add(std::shared_ptr<Type> scene);

		template <class Type, class... Args>
		std::shared_ptr<Type> emplace(Args&&... args);
		
		template <class Type>
		void switchTo();

		template <class Type>
		bool isCurrentScene() const;

	private:
		using Key = entt::id_type;

		template <class Type>
		static Key getKey();

		std::map<Key, std::shared_ptr<Scene>> scenes_;
		Key currentKey_;
		std::shared_ptr<entt::dispatcher> dispatcher_;
	};

	template <class Type>
	std::shared_ptr<Type> StateMachine::add(std::shared_ptr<Type> scene) {
		static_assert(std::is_base_of<Scene, Type>::value,
			"Type must have Scene as base class");

		if (scene) {
			auto key = getKey<Type>();
			if (scenes_.empty()) {
				currentKey_ = key;
			}
			static_cast<Scene&>(*scene).dispatcher_ = dispatcher_;
			auto it = scenes_.find(key);
			if (it == scenes_.end()) {
				scenes_[key] = scene;
			} else {
				spdlog::warn("[SceneStateMachine] Tried to add, scene {} already added!", typeid(Type).name());
			}
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

	template <class Type>
	void StateMachine::switchTo() {
		if (auto it = scenes_.find(getKey<Type>()); it != scenes_.end()) {
			currentKey_ = it->first;
		} else {
			spdlog::warn("[SceneStateMachine] Failed to switch to scene {}.", typeid(Type).name());
		}
	}

	template <class Type>
	bool StateMachine::isCurrentScene() const {
		return getKey<Type>() == currentKey_;
	}

	template <class Type>
	StateMachine::Key StateMachine::getKey() {
		return entt::type_info<Type>::id();
	}

}

#endif
