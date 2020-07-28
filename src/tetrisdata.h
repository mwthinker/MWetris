#ifndef MWETRIS_TETRISDATA_H
#define MWETRIS_TETRISDATA_H

#include "block.h"
#include "ai.h"

#include <sdl/sound.h>
#include <sdl/sprite.h>
#include <sdl/color.h>
#include <sdl/font.h>
#include <sdl/music.h>
#include <sdl/textureatlas.h>

#include <nlohmann/json.hpp>
#include <imgui.h>

#include <map>
#include <vector>

namespace tetris {

	constexpr BlockType charToBlockType(char key);
	constexpr const char* blockTypeToString(BlockType blocktype);

	class HighscoreRecord {
	public:
		HighscoreRecord() = default;

		HighscoreRecord(const std::string& name, std::string date, int points, int level, int rows)
			: name{name}
			, date{date}
			, points{points}
			, level{level}
			, rows{rows} {
		}

		std::string name;
		std::string date;
		int points{0};
		int level{1};
		int rows{0};
	};

	class TetrisData {
	public:
		static TetrisData& getInstance() {
			static TetrisData instance;
			return instance;
		}

		void quit() {
			sounds_.clear();
			fonts_.clear();
			musics_.clear();
		}

		TetrisData(TetrisData const&) = delete;
		TetrisData& operator=(const TetrisData&) = delete;

		void save();

		const sdl::Font& loadFont(const std::string& file, int fontSize);
		sdl::Sound loadSound(const std::string& file);
		sdl::Music loadMusic(const std::string& file);
		sdl::Sprite loadSprite(const std::string& file);

		sdl::Sprite getSprite(BlockType blockType);

		const sdl::Font& getDefaultFont(int size);

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

		void bindTextureFromAtlas() const;

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

		int getPort() const;
		void setPort(int port);

		int getTimeToConnectMS() const;

		std::string getIp() const;
		void setIp(const std::string& ip);

		sdl::Sprite getBackgroundSprite();

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

		sdl::Sprite getCheckboxBoxSprite();
		sdl::Sprite getCheckboxCheckSprite();
		sdl::Color getCheckboxTextColor() const;
		sdl::Color getCheckboxBackgroundColor() const;
		sdl::Color getCheckboxBoxColor() const;
		sdl::Color getChecboxCheckColor() const;

		sdl::Sprite getRadioButtonBoxSprite();
		sdl::Sprite getRadioButtonCheckSprite();
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
		sdl::Sprite getComboBoxShowDropDownSprite();

		sdl::Sprite getHumanSprite();
		sdl::Sprite getComputerSprite();
		sdl::Sprite getCrossSprite();
		sdl::Sprite getZoomSprite();

		sdl::Color getMiddleTextColor() const;
		int getMiddleTextBoxSize() const;

		//void setActiveLocalGame(int columns, int rows, const std::vector<PlayerData>& playerDataVector);
		//std::vector<game::PlayerData> getActiveLocalGamePlayers();
		int getActiveLocalGameRows() const;
		int getActiveLocalGameColumns() const;

	private:
		TetrisData();

		std::string jsonPath_;
		sdl::TextureAtlas textureAtlas_;
		std::map<std::string, sdl::Sound> sounds_;
		std::map<std::string, sdl::Font> fonts_;
		std::map<std::string, sdl::Music> musics_;
		nlohmann::json jsonObject_;

		mutable ImFont* defaultFont_{};
		mutable ImFont* headerFont_{};
		mutable ImFont* buttonFont_{};
	};

}

#endif
