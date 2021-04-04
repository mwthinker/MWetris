#ifndef MWETRIS_UI_SCENE_STATEMACHINE_H
#define MWETRIS_UI_SCENE_STATEMACHINE_H

#include "scene.h"
#include "event.h"

#include <sdl/shader.h>

#include <SDL_events.h>
#include <spdlog/spdlog.h>

#include <memory>
#include <map>
#include <typeindex>
#include <algorithm>

namespace mwetris::ui::scene {

	class StateMachine {
	public:
		StateMachine();

		void eventUpdate(const SDL_Event& windowEvent);

		void imGuiUpdate(const DeltaTime& deltaTime);

		void draw(sdl::Shader& shader, const DeltaTime& deltaTime);
		
		template <typename Type>
		std::shared_ptr<Type> add(std::shared_ptr<Type> scene);

		template <typename Type, typename... Args>
		std::shared_ptr<Type> emplace(Args&&... args);
		
		template <typename Type>
		void switchTo();

		template <typename Type>
		bool isCurrentScene() const;

		template<typename Callback>
		void setCallback(Callback&& callback) {
			callback_ = std::forward<Callback>(callback);
		}

		void emitEvent(Event event);

	private:
		void onCallback(scene::Event event);

		template <typename Type>
		static constexpr void staticAssertIsBaseOfScene();

		using Key = size_t;

		template <typename Type>
		static Key getKey();

		std::map<Key, std::shared_ptr<Scene>> scenes_;
		Key currentKey_{};
		std::function<void(scene::Event)> callback_;
	};

	template <typename Type>
	constexpr void StateMachine::staticAssertIsBaseOfScene() {
		static_assert(std::is_base_of<Scene, Type>::value,
			"Type must have Scene as base class");
	}

	template <typename Type>
	std::shared_ptr<Type> StateMachine::add(std::shared_ptr<Type> scenePtr) {
		staticAssertIsBaseOfScene<Type>();

		spdlog::info("[SceneStateMachine] Try to add Scene: {}", typeid(Type).name());

		if (!scenePtr) {
			spdlog::warn("[SceneStateMachine] Tried to add empty scene!");
			return nullptr;
		}

		auto key = getKey<Type>();
		spdlog::debug("[SceneStateMachine] Key: {}", key);
		auto& scene = static_cast<Scene&>(*scenePtr);

		scene.stateMachine_ = this;
		auto it = scenes_.find(key);
		if (it == scenes_.end()) {
			if (scenes_.empty()) {
				currentKey_ = key;
				scene.switchedTo();
			}
			scenes_[key] = scenePtr;
			spdlog::info("[SceneStateMachine] Scene added: {}={}", typeid(Type).name(), getKey<Type>());
		} else {
			spdlog::warn("[SceneStateMachine] Failed to add scene, {}={} already added!", typeid(Type).name(), getKey<Type>());
		}
		return std::move(scenePtr);
	}

	template <typename Type, typename... Args>
	std::shared_ptr<Type> StateMachine::emplace(Args&&... args) {
		return add(std::make_shared<Type>(std::forward<Args>(args)...));
	}

	template <typename Type>
	void StateMachine::switchTo() {
		staticAssertIsBaseOfScene<Type>();
		
		if (auto it = scenes_.find(getKey<Type>()); it != scenes_.end()) {
			auto key = it->first;
			if (currentKey_ != 0) {
				scenes_[currentKey_]->switchedFrom();
			}
			currentKey_ = key;
			scenes_[currentKey_]->switchedTo();
		} else {
			spdlog::warn("[SceneStateMachine] Failed to switch to scene {}={} .", typeid(Type).name(), getKey<Type>());
		}
	}

	template <typename Type>
	bool StateMachine::isCurrentScene() const {
		return getKey<Type>() == currentKey_;
	}

	template <typename Type>
	StateMachine::Key StateMachine::getKey() {
		return typeid(Type).hash_code();
	}

}

#endif
