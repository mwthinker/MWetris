#ifndef APP_UI_SCENE_STATEMACHINE_H
#define APP_UI_SCENE_STATEMACHINE_H

#include "scene.h"

#include <sdl/shader.h>

#include <SDL_events.h>
#include <spdlog/spdlog.h>

#include <memory>
#include <map>
#include <typeindex>
#include <algorithm>
#include <concepts>

namespace app::ui::scene {

	template <typename Type>
	concept DerivedFromScene = std::derived_from<Type, Scene>;

	class StateMachine {
	public:
		enum class StateType {
			Fullscreen,
			Modal
		};

		explicit StateMachine(StateType type = StateType::Fullscreen);

		~StateMachine();

		void imGuiUpdate(const DeltaTime& deltaTime);
		
		template <typename Type> requires DerivedFromScene<Type>
		std::shared_ptr<Type> add(std::shared_ptr<Type> scene);

		template <typename Type, typename... Args> requires DerivedFromScene<Type>
		std::shared_ptr<Type> emplace(Args&&... args);

		template <typename Type> requires DerivedFromScene<Type>
		void switchTo();

		template <typename Type> requires DerivedFromScene<Type>
		void switchTo(const SceneData& sceneData);

		template <typename Type> requires DerivedFromScene<Type>
		bool isCurrentScene() const;

		template <typename Type> requires DerivedFromScene<Type>
		std::shared_ptr<Type> getScene();

	private:
		using Key = size_t;

		template <typename Type> requires DerivedFromScene<Type>
		static Key getKey();

		std::map<Key, std::shared_ptr<Scene>> scenes_;
		Key currentKey_{};
		StateType stateType_;
		bool openPopUp_ = false;
		bool closePopUp_ = false;
	};

	template <typename Type> requires DerivedFromScene<Type>
	std::shared_ptr<Type> StateMachine::add(std::shared_ptr<Type> scenePtr) {
		spdlog::info("[SceneStateMachine] Try to add Scene: {}", typeid(Type).name());

		if (!scenePtr) {
			spdlog::warn("[SceneStateMachine] Tried to add empty scene!");
			return nullptr;
		}

		auto key = getKey<Type>();
		spdlog::debug("[SceneStateMachine] Key: {}", key);
		auto& scene = static_cast<Scene&>(*scenePtr);

		scene.stateMachine_ = this;
		
		if (auto it = scenes_.find(key);  it == scenes_.end()) {
			if (scenes_.empty()) {
				currentKey_ = key;
				SceneData sceneData;
				scene.switchedTo(sceneData);
			}
			scenes_[key] = scenePtr;
			spdlog::info("[SceneStateMachine] Scene added: {}={}", typeid(Type).name(), getKey<Type>());
		} else {
			spdlog::warn("[SceneStateMachine] Failed to add scene, {}={} already added!", typeid(Type).name(), getKey<Type>());
		}
		return std::move(scenePtr);
	}

	template <typename Type, typename... Args> requires DerivedFromScene<Type>
	std::shared_ptr<Type> StateMachine::emplace(Args&&... args) {
		return add(std::make_shared<Type>(std::forward<Args>(args)...));
	}

	template <typename Type> requires DerivedFromScene<Type>
	void StateMachine::switchTo() {
		SceneData sceneData;
		switchTo<Type>(sceneData);
		if (isCurrentScene<EmptyScene>()) {
			openPopUp_ = false;
			closePopUp_ = true;
		} else {
			openPopUp_ = true;
			closePopUp_ = false;
		}
	}

	template <typename Type> requires DerivedFromScene<Type>
	void StateMachine::switchTo(const SceneData& sceneData) {
		if (isCurrentScene<Type>()) {
			return;
		}

		if (auto it = scenes_.find(getKey<Type>()); it != scenes_.end()) {
			auto key = it->first;
			if (currentKey_ != 0) {
				scenes_[currentKey_]->switchedFrom();
			}
			currentKey_ = key;
			scenes_[currentKey_]->switchedTo(sceneData);
		} else {
			spdlog::warn("[SceneStateMachine] Failed to switch to scene {}={} .", typeid(Type).name(), getKey<Type>());
		}
	}

	template <typename Type> requires DerivedFromScene<Type>
	bool StateMachine::isCurrentScene() const {
		return getKey<Type>() == currentKey_;
	}

	template <typename Type> requires DerivedFromScene<Type>
	StateMachine::Key StateMachine::getKey() {
		return typeid(Type).hash_code();
	}

	template <typename Type> requires DerivedFromScene<Type>
	std::shared_ptr<Type> StateMachine::getScene() {
		if (auto it = scenes_.find(getKey<Type>()); it != scenes_.end()) {
			return std::static_pointer_cast<Type>(it.second);
		}
		return nullptr;
	}

}

#endif
