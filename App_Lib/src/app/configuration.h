#ifndef APP_CONFIGURATION_H
#define APP_CONFIGURATION_H

#include <tetris/block.h>
#include <tetris/ai.h>

#include "controlprofiles.h"
#include "game/inputbindings.h"
#include "textureview.h"

#include <imgui.h>
#include <sdl/color.h>
#include <sdl/gpu.h>
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
			sdl::Color outerSquareColor;
			sdl::Color innerSquareColor;
			sdl::Color startAreaColor;
			sdl::Color borderColor;
			sdl::Color downBlockColor;
			sdl::Color playerCardBackgroundColor;
			sdl::Color playerCardBorderColor;
			sdl::Color sidebarBackgroundColor;
			sdl::Color sidebarBorderColor;
			sdl::Color previewBackgroundColor;
			sdl::Color previewBorderColor;
			sdl::Color playerCardTextColor;

			bool operator==(const BoardSettings&) const = default;
		};

		struct ColorSchemeInfo {
			std::string_view id;
			std::string_view name;
		};

		struct AppearanceSettings {
			sdl::Color labelTextColor;
			sdl::Color buttonFocusColor;
			sdl::Color buttonTextColor;
			sdl::Color buttonHoverColor;
			sdl::Color buttonBackgroundColor;
			sdl::Color buttonBorderColor;

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

		struct AudioSettings {
			bool masterEnabled = true;
			bool musicEnabled = true;
			bool effectsEnabled = true;
			float masterVolume = 1.f;
			float musicVolume = 0.6f;
			float effectsVolume = 0.8f;

			bool operator==(const AudioSettings&) const = default;
		};

		struct UiSoundAssets {
			std::string click = "audio/ui/click.wav";
			std::string confirm = "audio/ui/confirm.wav";
			std::string error = "audio/ui/error.wav";

			bool operator==(const UiSoundAssets&) const = default;
		};

		struct GameSoundAssets {
			std::string move = "audio/game/move.wav";
			std::string hardDrop = "audio/game/hard-drop.wav";
			std::string lock = "audio/game/lock.wav";
			std::string lineClear = "audio/game/line-clear.wav";
			std::string lineClear4 = "audio/game/tetris.wav";
			std::string levelUp = "audio/game/level-up.wav";
			std::string garbage = "audio/game/garbage.wav";
			std::string countdown = "audio/game/countdown.wav";
			std::string go = "audio/game/go.wav";
			std::string pause = "audio/game/pause.wav";
			std::string resume = "audio/game/resume.wav";
			std::string gameOver = "audio/game/game-over.wav";

			bool operator==(const GameSoundAssets&) const = default;
		};

		struct AudioAssetSettings {
			std::string music = "audio/music/tetris-theme.ogg";
			UiSoundAssets ui;
			GameSoundAssets game;

			bool operator==(const AudioAssetSettings&) const = default;
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
			AudioSettings audio;
			AudioAssetSettings audioAssets;
			Network network;
			std::array<std::string, 4> activeAiNames;
			std::vector<AiDefinition> ais;

			bool operator==(const EditableSettings&) const = default;
		};

		struct SettingsApplyResult {
			bool restartRequired = false;
			std::vector<std::string> restartReasons;
		};

		struct SinglePlayerControls {
			std::string deviceGuid = "Keyboard 1";
			ProfileId profileId = ControlProfiles::BalancedProfileId;
			int startLevel = 0;

			bool operator==(const SinglePlayerControls&) const = default;
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
		sdl::Color getDownBlockColor() const;

		int getWindowPositionX() const;
		int getWindowPositionY() const;

		int getWindowWidth() const;
		int getWindowHeight() const;

		std::string getWindowIcon() const;

		bool isWindowPauseOnLostFocus() const;

		bool isFullscreenOnDoubleClick() const;

		bool isMoveWindowByHoldingDownMouse() const;

		Network getNetwork() const;

		EditableSettings getEditableSettings() const;
		std::expected<EditableSettings, std::string> getDefaultEditableSettings() const;
		SettingsValidationResult validateEditableSettings(const EditableSettings& candidate) const;
		SettingsPersistenceResult persistEditableSettings(const EditableSettings& candidate);

		app::TextureView getBackgroundSprite();

		std::string getAi1Name() const;
		std::string getAi2Name() const;
		std::string getAi3Name() const;

		std::vector<Ai> getAiVector() const;

		sdl::Color getChecboxCheckColor() const;

		sdl::Color getLabelTextColor() const;

		sdl::Color getButtonFocusColor() const;
		sdl::Color getButtonTextColor() const;
		sdl::Color getButtonHoverColor() const;
		sdl::Color getButtonPushColor() const;
		sdl::Color getButtonBackgroundColor() const;
		sdl::Color getButtonBorderColor() const;

		sdl::Color getComboBoxFocusColor() const;
		sdl::Color getComboBoxBackgroundColor() const;
		sdl::Color getComboBoxBorderColor() const;

		ControlProfilesLoadResult loadControlProfiles();
		ControlProfilesPersistenceResult persistControlProfiles(const ControlProfilesState& state);
		DeviceBindingsLoadResult loadDeviceBindings();
		game::DeviceBindingsPersistenceResult persistDeviceBindings(const std::vector<game::DeviceBinding>& state);
		SinglePlayerControls getLastSinglePlayerControls() const;
		std::expected<void, std::string> persistLastSinglePlayerControls(const SinglePlayerControls& controls);

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
