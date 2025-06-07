#ifndef APP_CONFIGURATION_H
#define APP_CONFIGURATION_H

#include <optional>

#include <tetris/block.h>
#include <tetris/ai.h>

#include <sdl/textureview.h>
#include <sdl/sound.h>
#include <sdl/color.h>
#include <sdl/font.h>
#include <sdl/music.h>

#include <imgui.h>

namespace app {

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
		struct Ai {
			std::string name;
			tetris::Ai ai;
		};

		struct Network {
			struct {
				std::string ip;
				int port;
			} global;
			struct {
				std::string ip;
				int port;
			} server;
		};

		struct Device {
			std::string guid;
			double das;
			double arr;
		};

		struct DeviceChangeEvent {
			std::string guid;
			double das;
			double arr;
		};

		static Configuration& getInstance();

		void quit();

		Configuration(const Configuration&) = delete;
		Configuration& operator=(const Configuration&) = delete;

		void save();

		sdl::Sound loadSound(const std::string& file);
		sdl::TextureView loadSprite(const std::string& file);
		sdl::TextureView getSprite(tetris::BlockType blockType);

		ImFont* getImGuiDefaultFont() const;
		ImFont* getImGuiHeaderFont() const;
		ImFont* getImGuiButtonFont() const;

		sdl::Color getOuterSquareColor() const;
		sdl::Color getInnerSquareColor() const;
		sdl::Color getStartAreaColor() const;
		sdl::Color getPlayerAreaColor() const;
		sdl::Color getBorderColor() const;

		float getTetrisSquareSize() const;
		float getTetrisBorderSize() const;

		bool isShowDownBlock() const;
		void setShowDownBlock(bool showDownColor);
		sdl::Color getDownBlockColor() const;

		void bindTextureFromAtlas();

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

		sdl::TextureView getBackgroundSprite();

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

		sdl::TextureView getHumanSprite();
		sdl::TextureView getComputerSprite();
		sdl::TextureView getCrossSprite();
		sdl::TextureView getZoomSprite();

		sdl::Color getMiddleTextColor() const;
		int getMiddleTextBoxSize() const;

		int getActiveLocalGameRows() const;
		int getActiveLocalGameColumns() const;

		std::optional<Device> getDevice(std::string_view guid) const;
		void setDevice(std::string_view guid, double das, double arr);

	private:
		Configuration();

		struct Impl;
		std::unique_ptr<Impl> impl_;
	};

}

#endif
