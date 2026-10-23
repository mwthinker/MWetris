#ifndef APP_CONFIGURATION_H
#define APP_CONFIGURATION_H

#include <tetris/block.h>
#include <tetris/ai.h>

#include <nlohmann/json.hpp>

#include "controlprofiles.h"
#include "game/inputbindings.h"
#include "textureview.h"
#include <sdl/color.h>

#include <sdl/gpu.h>
#include <imgui.h>
#include <mw/signal.h>

#include <array>
#include <expected>
#include <memory>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace app {
	using ControlProfilesLoadResult = std::expected<ControlProfilesState, std::string>;
	using DeviceBindingsLoadResult = std::expected<std::vector<game::DeviceBinding>, std::string>;

	constexpr tetris::BlockType charToBlockType(char key);
	constexpr const char* blockTypeToString(tetris::BlockType blocktype);

	struct HighscoreRecord {
		std::string name;
		std::string date;
		int points = 0;
		int level = 1;
		int rows = 0;
	};

	class Configuration {
	public:
		struct Endpoint {
			std::string ip;
			int port = 0;

			bool operator==(const Endpoint&) const = default;
		};

		struct Ai {
			std::string name;
			tetris::Ai ai;
		};

		struct Network {
			Endpoint global;
			Endpoint server;

			bool operator==(const Network&) const = default;
		};

		struct WindowSettings {
			int width = 0;
			int height = 0;
			int minWidth = 0;
			int minHeight = 0;
			bool resizable = true;
			bool bordered = true;
			bool maximized = false;
			bool pauseOnLostFocus = true;
			bool fullscreenOnDoubleClick = false;
			bool moveWindowByHoldingDownMouse = false;
			bool vsync = true;
			bool limitFps = false;
			float uiScale = 1.f;

			bool operator==(const WindowSettings&) const = default;
		};

		struct BoardSettings {
			float borderSize = 0.f;
			bool showDownBlock = true;
			std::string colorScheme = "custom";
			std::string outerSquareColor;
			std::string innerSquareColor;
			std::string startAreaColor;
			std::string borderColor;
			std::string downBlockColor;
			std::string playerCardBackgroundColor;
			std::string playerCardBorderColor;
			std::string sidebarBackgroundColor;
			std::string sidebarBorderColor;
			std::string previewBackgroundColor;
			std::string previewBorderColor;
			std::string playerCardTextColor;

			bool operator==(const BoardSettings&) const = default;
		};

		struct ColorSchemeInfo {
			std::string_view id;
			std::string_view name;
		};

		struct AppearanceSettings {
			std::string labelTextColor;
			std::string buttonFocusColor;
			std::string buttonTextColor;
			std::string buttonHoverColor;
			std::string buttonBackgroundColor;
			std::string buttonBorderColor;

			bool operator==(const AppearanceSettings&) const = default;
		};

		struct AssetSettings {
			std::string windowFont;
			std::string boardFont;
			std::string icon;
			std::string background;
			std::array<std::string, 7> blockSprites;

			bool operator==(const AssetSettings&) const = default;
		};

		struct AiDefinition {
			std::string name;
			std::string valueFunction;

			bool operator==(const AiDefinition&) const = default;
		};

		struct EditableSettings {
			WindowSettings window;
			BoardSettings board;
			AppearanceSettings appearance;
			AssetSettings assets;
			Network network;
			std::array<std::string, 4> activeAiNames;
			std::vector<AiDefinition> ais;

			bool operator==(const EditableSettings&) const = default;
		};

		struct SettingsApplyResult {
			bool restartRequired = false;
			std::vector<std::string> restartReasons;
		};

		using SettingsValidationResult = std::expected<void, std::string>;
		using SettingsPersistenceResult = std::expected<SettingsApplyResult, std::string>;

		mw::PublicSignal<Configuration, const EditableSettings&, const EditableSettings&> settingsChanged;

		static Configuration& getInstance();

		void resetRuntimeResources();

		Configuration(const Configuration&) = delete;
		Configuration& operator=(const Configuration&) = delete;

		void init(SDL_GPUDevice* gpuDevice);
		void initImGuiFonts() const;

		app::TextureView getSprite(tetris::BlockType blockType);

		ImFont* getImGuiDefaultFont() const;
		ImFont* getImGuiRegularSmallFont() const;
		ImFont* getImGuiBoardRegularFont() const;
		ImFont* getImGuiMediumFont() const;
		ImFont* getImGuiMediumValueFont() const;
		ImFont* getImGuiSemiBoldFont() const;
		ImFont* getImGuiScoreFont() const;

		sdl::Color getOuterSquareColor() const;
		sdl::Color getInnerSquareColor() const;
		sdl::Color getStartAreaColor() const;
		sdl::Color getPlayerAreaColor() const;
		sdl::Color getBorderColor() const;
		sdl::Color getPlayerCardBackgroundColor() const;
		sdl::Color getPlayerCardBorderColor() const;
		sdl::Color getSidebarBackgroundColor() const;
		sdl::Color getSidebarBorderColor() const;
		sdl::Color getPreviewBackgroundColor() const;
		sdl::Color getPreviewBorderColor() const;
		sdl::Color getPlayerCardTextColor() const;

		static std::span<const ColorSchemeInfo> getColorSchemes();
		static void applyColorScheme(BoardSettings& settings, std::string_view id);
		static std::string detectColorScheme(const BoardSettings& settings);

		float getTetrisSquareSize() const;
		float getTetrisBorderSize() const;

		bool isShowDownBlock() const;
		void setShowDownBlock(bool showDownColor);
		sdl::Color getDownBlockColor() const;

		bool isLimitFps() const;
		void setLimitFps(bool limited);

		int getWindowPositionX() const;
		int getWindowPositionY() const;

		void setWindowPositionX(int x);
		void setWindowPositionY(int y);

		int getWindowWidth() const;
		int getWindowHeight() const;

		void setWindowWidth(int width);
		void setWindowHeight(int height);

		bool isWindowResizable() const;
		void setWindowResizable(bool resizeable);

		int getWindowMinWidth() const;
		int getWindowMinHeight() const;
		std::string getWindowIcon() const;

		bool isWindowPauseOnLostFocus() const;
		void setWindowPauseOnLostFocus(bool pauseOnFocus);

		bool isWindowBordered() const;
		void setWindowBordered(bool border);

		bool isWindowMaximized() const;
		void setWindowMaximized(bool maximized);

		bool isWindowVsync() const;
		void setWindowVsync(bool activate);

		int getMultiSampleBuffers() const;
		int getMultiSampleSamples() const;

		float getRowFadingTime() const;
		void setRowFadingTime(float time);

		float getRowMovingTime() const;
		void setRowMovingTime(float time);

		bool isFullscreenOnDoubleClick() const;
		void setFullscreenOnDoubleClick(bool activate);

		bool isMoveWindowByHoldingDownMouse() const;
		void setMoveWindowByHoldingDownMouse(bool activate);

		Network getNetwork() const;

		EditableSettings getEditableSettings() const;
		std::expected<EditableSettings, std::string> getDefaultEditableSettings() const;
		SettingsValidationResult validateEditableSettings(const EditableSettings& candidate) const;
		SettingsPersistenceResult persistEditableSettings(const EditableSettings& candidate);

		app::TextureView getBackgroundSprite();

		std::string getAi1Name() const;
		std::string getAi2Name() const;
		std::string getAi3Name() const;
		std::string getAi4Name() const;

		void setAi1Name(const std::string& name);
		void setAi2Name(const std::string& name);
		void setAi3Name(const std::string& name);
		void setAi4Name(const std::string& name);

		std::vector<Ai> getAiVector() const;

		std::vector<HighscoreRecord> getHighscoreRecordVector() const;
		void setHighscoreRecordVector(const std::vector<HighscoreRecord>& highscoreVector);

		float getWindowBarHeight() const;

		sdl::Color getWindowBarColor() const;
		
		sdl::Color getCheckboxTextColor() const;
		sdl::Color getCheckboxBackgroundColor() const;
		sdl::Color getCheckboxBoxColor() const;
		sdl::Color getChecboxCheckColor() const;
		
		sdl::Color getRadioButtonTextColor() const;
		sdl::Color getRadioButtonBackgroundColor() const;
		sdl::Color getRadioButtonBoxColor() const;
		sdl::Color getRadioButtonCheckColor() const;

		sdl::Color getLabelTextColor() const;
		sdl::Color getLabelBackgroundColor() const;

		sdl::Color getButtonFocusColor() const;
		sdl::Color getButtonTextColor() const;
		sdl::Color getButtonHoverColor() const;
		sdl::Color getButtonPushColor() const;
		sdl::Color getButtonBackgroundColor() const;
		sdl::Color getButtonBorderColor() const;

		sdl::Color getComboBoxFocusColor() const;
		sdl::Color getComboBoxTextColor() const;
		sdl::Color getComboBoxSelectedTextColor() const;
		sdl::Color getComboBoxSelectedBackgroundColor() const;
		sdl::Color getComboBoxBackgroundColor() const;
		sdl::Color getComboBoxBorderColor() const;
		sdl::Color getComboBoxShowDropDownColor() const;

		sdl::Color getMiddleTextColor() const;
		int getMiddleTextBoxSize() const;

		int getActiveLocalGameRows() const;
		int getActiveLocalGameColumns() const;

		ControlProfilesLoadResult loadControlProfiles();
		ControlProfilesPersistenceResult persistControlProfiles(const ControlProfilesState& state);
		static ControlProfilesState normalizeControlProfiles(nlohmann::json& jsonObject);
		DeviceBindingsLoadResult loadDeviceBindings();
		game::DeviceBindingsPersistenceResult persistDeviceBindings(const std::vector<game::DeviceBinding>& state);
		static std::vector<game::DeviceBinding> normalizeDeviceBindings(nlohmann::json& jsonObject);

		// Every TextureView is bound to the atlas texture
		ImTextureID getTextureAtlasBinding() {
			return ImTextureID(atlasTexture_.get());
		}

	private:
		Configuration();

		app::TextureView blockTypeI_;
		app::TextureView blockTypeJ_;
		app::TextureView blockTypeL_;
		app::TextureView blockTypeO_;
		app::TextureView blockTypeS_;
		app::TextureView blockTypeT_;
		app::TextureView blockTypeZ_;
		app::TextureView background_;

		struct Impl;
		std::unique_ptr<Impl> impl_;
		sdl::GpuTexture atlasTexture_;
	};

}

#endif
