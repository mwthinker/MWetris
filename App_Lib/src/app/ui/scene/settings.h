#ifndef APP_UI_SCENE_SETTINGS_H
#define APP_UI_SCENE_SETTINGS_H

#include "scene.h"
#include "configuration.h"

#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace app::audio {

	class AudioEngine;

}

namespace app::ui {

	class ControlsPresenter;

}

namespace app::ui::scene {

	class Settings : public Scene {
	public:
		Settings(std::shared_ptr<ControlsPresenter> controlsPresenter, std::shared_ptr<app::audio::AudioEngine> audioEngine, std::function<void()> requestRestart);

		void imGuiUpdate(const DeltaTime& deltaTime) override;
		ImVec2 getSize() const override;
		ImGuiWindowFlags getWindowFlags() const override {
			return ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;
		}

	private:
		void switchedTo(const SceneData& data) override;
		void switchedFrom() override;
		void drawBasicSettings();
		void drawAudioSettings();
		void drawAdvancedSettings();
		void drawColorSchemeSelector();
		void drawAiSelectors();
		void drawCustomAis();
		void apply();

		std::shared_ptr<ControlsPresenter> controlsPresenter_;
		std::shared_ptr<app::audio::AudioEngine> audioEngine_;
		std::function<void()> requestRestart_;
		Configuration::EditableSettings committed_;
		Configuration::EditableSettings draft_;
		std::optional<Configuration::EditableSettings> defaults_;
		std::string defaultsError_;
		std::string error_;
		std::vector<std::string> restartReasons_;
		bool showAdvanced_ = false;
		bool loaded_ = false;
	};

}

#endif
