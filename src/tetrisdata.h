#ifndef TETRISDATA_H
#define TETRISDATA_H

#include "square.h"
#include "block.h"
#include "ai.h"
//#include "tetrisgame.h"

#include <sdl/sound.h>
#include <sdl/sprite.h>
//#include <mw/color.h>
#include <sdl/font.h>
#include <sdl/music.h>
#include <sdl/textureatlas.h>

#include <nlohmann/json.hpp>
#include <imgui.h>

#include <map>
#include <vector>

namespace tetris {

	BlockType charToBlockType(char key);
	std::string blockTypeToString(BlockType blocktype);

	class HighscoreRecord {
	public:
		HighscoreRecord() : points_(0), level_(1), rows_(0) {
		}

		HighscoreRecord(const std::string& name, std::string date, int points, int level, int rows)
			: name_(name), date_(date), points_(points), level_(level), rows_(rows) {
		}

		std::string name_;
		std::string date_;
		int points_;
		int level_;
		int rows_;
	};

	class TetrisData {
	public:
		static TetrisData& getInstance() {
			static TetrisData instance;
			return instance;
		}

		TetrisData(TetrisData const&) = delete;
		TetrisData& operator=(const TetrisData&) = delete;

		void save();

		//sdl::Font loadFont(const std::string& file, unsigned int fontSize);
		sdl::Sound loadSound(const std::string& file);
		sdl::Music loadMusic(const std::string& file);
		sdl::Sprite loadSprite(const std::string& file);

		sdl::Sprite getSprite(BlockType blockType);

		sdl::Font getDefaultFont(int size);

		ImColor getOuterSquareColor() const;
		ImColor getInnerSquareColor() const;
		ImColor getStartAreaColor() const;
		ImColor getPlayerAreaColor() const;
		ImColor getBorderColor() const;

		float getTetrisSquareSize() const;
		float getTetrisBorderSize() const;

		bool isShowDownBlock() const;
		void setShowDownBlock(bool showDownColor);
		ImColor getDownBlockColor() const;

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

		ImColor getWindowBarColor() const;

		sdl::Sprite getCheckboxBoxSprite();
		sdl::Sprite getCheckboxCheckSprite();
		ImColor getCheckboxTextColor() const;
		ImColor getCheckboxBackgroundColor() const;
		ImColor getCheckboxBoxColor() const;
		ImColor getChecboxCheckColor() const;

		sdl::Sprite getRadioButtonBoxSprite();
		sdl::Sprite getRadioButtonCheckSprite();
		ImColor getRadioButtonTextColor() const;
		ImColor getRadioButtonBackgroundColor() const;
		ImColor getRadioButtonBoxColor() const;
		ImColor getRadioButtonCheckColor() const;

		ImColor getLabelTextColor() const;
		ImColor getLabelBackgroundColor() const;

		ImColor getButtonFocusColor() const;
		ImColor getButtonTextColor() const;
		ImColor getButtonHoverColor() const;
		ImColor getButtonPushColor() const;
		ImColor getButtonBackgroundColor() const;
		ImColor getButtonBorderColor() const;

		ImColor getComboBoxFocusColor() const;
		ImColor getComboBoxTextColor() const;
		ImColor getComboBoxSelectedTextColor() const;
		ImColor getComboBoxSelectedBackgroundColor() const;
		ImColor getComboBoxBackgroundColor() const;
		ImColor getComboBoxBorderColor() const;
		ImColor getComboBoxShowDropDownColor() const;
		sdl::Sprite getComboBoxShowDropDownSprite();

		sdl::Sprite getHumanSprite();
		sdl::Sprite getComputerSprite();
		sdl::Sprite getCrossSprite();
		sdl::Sprite getZoomSprite();

		ImColor getMiddleTextColor() const;
		int getMiddleTextBoxSize() const;

		//void setActiveLocalGame(int columns, int rows, const std::vector<PlayerData>& playerDataVector);
		//std::vector<PlayerData> getActiveLocalGamePlayers();
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
	};

}

#endif // TETRISDATA_H
