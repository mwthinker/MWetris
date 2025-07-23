#ifndef APP_UI_SCENE_SCENE_H
#define APP_UI_SCENE_SCENE_H

#include <sdl/shader.h>

#include <SDL3/SDL_events.h>
#include <spdlog/spdlog.h>

#include <chrono>
#include <imgui.h>

namespace app::ui::scene {

	class StateMachine;

	using DeltaTime = std::chrono::high_resolution_clock::duration;

	class SceneData {
	public:
		virtual ~SceneData() = default;
	};

	class Scene {
	public:
		friend class StateMachine;

		virtual ~Scene() = default;

		virtual void imGuiUpdate(const DeltaTime& deltaTime) = 0;
		
		virtual ImVec2 getSize() const {
			return {};
		}

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

		virtual void switchedTo(const SceneData& data) {};
		
		StateMachineWrapper stateMachine_;
	};

	class EmptyScene : public Scene {
	public:
		void imGuiUpdate(const DeltaTime& deltaTime) override {}
	};

}

#endif
