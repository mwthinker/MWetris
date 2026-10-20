#ifndef APP_UI_SCENE_SETTINGS_H
#define APP_UI_SCENE_SETTINGS_H

#include "scene.h"
#include "configuration.h"

#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace app::ui {

	class ControlsPresenter;

}

namespace app::ui::scene {

	class Settings : public Scene {
	public:
		Settings(std::shared_ptr<ControlsPresenter> controlsPresenter, std::function<void()> requestRestart);

		void imGuiUpdate(const DeltaTime& deltaTime) override;
		ImVec2 getSize() const override;

	private:
		void switchedTo(const SceneData& data) override;
		void switchedFrom() override;
		void drawBasicSettings();
		void drawAdvancedSettings();
		void drawColorSchemeSelector();
		void drawAiSelectors();
		void drawCustomAis();
		void apply();

		std::shared_ptr<ControlsPresenter> controlsPresenter_;
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
