#include "tetrisdata.h"

#include <sdl/color.h>

#include <nlohmann/json.hpp>

#include <fstream>
#include <sstream>

using nlohmann::json;
using namespace mwetris;

namespace sdl {

	void from_json(const json& j, Color& color) {
		auto textColor = j.get<std::string>();
		if (sdl::color::isValidHexColor(textColor)) {
			color = Color{textColor};
		} else {
			std::stringstream stream{textColor};
			float r;
			if (!(stream >> r)) {
				throw std::runtime_error{"Red value invalid"};
			}
			float g;
			if (!(stream >> g)) {
				throw std::runtime_error{"Green value invalid"};
			}
			float b;
			if (!(stream >> b)) {
				throw std::runtime_error{"Blue value invalid"};
			}
			float a = 1.f;
			if (!stream.eof()) {
				if (!(stream >> a)) {
					throw std::runtime_error{"Alpha value invalid"};
				}
			}
			color = Color{r, g, b, a};
		}
	}

}

namespace tetris {

	void from_json(const json& j, tetris::Ai& ai) {
		ai = tetris::Ai{j.at("name").get<std::string>(), j.at("valueFunction").get<std::string>()};
	}

	void from_json(const json& j, tetris::BlockType& blockType) {
		blockType = charToBlockType(j.get<std::string>()[0]);
	}

	void to_json(json& j, const tetris::BlockType& blockType) {
		j = json{blockTypeToString(blockType)};
	}

	void from_json(const json& j, tetris::Block& block) {
		block = tetris::Block{j.at("blockType").get<tetris::BlockType>(),
			j.at("leftColumn").get<int>(),
			j.at("bottomRow").get<int>(),
			j.at("currentRotation").get<int>()};
	}

	void to_json(json& j, const tetris::Block& block) {
		j = json{{
			{"bottomRow", block.getLowestStartRow()},
			{"blockType", block.getBlockType()},
			{"leftColumn", block.getStartColumn()},
			{"currentRotation", block.getCurrentRotation()}
		}};
	}

}

namespace mwetris {

	constexpr tetris::BlockType charToBlockType(char key) {
		switch (key) {
			case 'z': case 'Z':
				return tetris::BlockType::Z;
			case 'w': case 'W':
				return tetris::BlockType::Wall;
			case 't': case 'T':
				return tetris::BlockType::T;
			case 's': case 'S':
				return tetris::BlockType::S;
			case 'o': case 'O':
				return tetris::BlockType::O;
			case 'l': case 'L':
				return tetris::BlockType::L;
			case 'j': case 'J':
				return tetris::BlockType::J;
			case 'I': case 'i':
				return tetris::BlockType::I;
			default:
				return tetris::BlockType::Empty;
		}
	}

	constexpr const char* blockTypeToString(tetris::BlockType blocktype) {
		switch (blocktype) {
			case tetris::BlockType::Z:
				return "Z";
			case tetris::BlockType::Wall:
				return "W";
			case tetris::BlockType::T:
				return "T";
			case tetris::BlockType::S:
				return "S";
			case tetris::BlockType::O:
				return "O";
			case tetris::BlockType::L:
				return "L";
			case tetris::BlockType::J:
				return "J";
			case tetris::BlockType::I:
				return "I";
			default:
				return "E";
		}
	}

	std::string convertBlockTypesToString(const std::vector<tetris::BlockType>& board) {
		std::stringstream stream;
		for (auto chr : board) {
			stream << blockTypeToString(chr);
		}
		return stream.str();
	}

	std::vector<tetris::BlockType> convertStringToBlockTypes(const std::string& str) {
		std::vector<tetris::BlockType> blocktypes_;
		for (auto key : str) {
			blocktypes_.push_back(charToBlockType(key));
		}
		return blocktypes_;
	}

	void from_json(const json& j, HighscoreRecord& highscoreRecord) {
		highscoreRecord.name = j.at("name").get<std::string>();
		highscoreRecord.date = j.at("date").get<std::string>();
		highscoreRecord.points = j.at("points").get<int>();

		try {
			highscoreRecord.level = j.at("level").get<int>();
		} catch (nlohmann::detail::out_of_range) {
			highscoreRecord.level = 0;
		}

		try {
			highscoreRecord.rows = j.at("rows").get<int>();
		} catch (nlohmann::detail::out_of_range) {
			highscoreRecord.rows = 0;
		}
	}

	void to_json(json& j, const HighscoreRecord& highscoreRecord) {
		j = json{
			{"name", highscoreRecord.name},
			{"date", highscoreRecord.date},
			{"points", highscoreRecord.points},
			{"level", highscoreRecord.level},
			{"rows", highscoreRecord.rows}
		};
	}

	TetrisData::TetrisData() : textureAtlas_(2048, 2048, []() {
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	}) {
		std::ifstream defaultStream{"USE_APPLICATION_JSON"};
		bool applicationJson;
		defaultStream >> applicationJson;
		const std::string APPLICATION_JSON{"tetris.json"};
		if (applicationJson) {
			jsonPath_ = APPLICATION_JSON;
		} else {
			// Find default path to save/load file from.
			jsonPath_ = SDL_GetPrefPath("mwthinker", "MWetris2") + APPLICATION_JSON;
		}
		std::ifstream stream(jsonPath_);
		if (!stream.is_open()) {
			// Assume that the file does not exist, load file from application folder.
			stream = std::ifstream(APPLICATION_JSON);
		}
		stream >> jsonObject_;
	}

	void TetrisData::save() {
		std::ofstream stream{jsonPath_};
		stream << jsonObject_.dump(1);
	}
	
	const sdl::Font& TetrisData::loadFont(const std::string& file, int fontSize) {
		assert(fontSize > 0);

		size_t size = fonts_.size();
		std::string key = file;
		key += fontSize;
		sdl::Font& font = fonts_[key];
		if (fonts_.size() > size) {
			font = sdl::Font{file, fontSize};
		}
		return font;
	}

	sdl::Sound TetrisData::loadSound(const std::string& file) {
		size_t size = sounds_.size();
		sdl::Sound& sound = sounds_[file];

		// Sound not found?
		if (sounds_.size() > size) {
			sound = sdl::Sound(file);
		}

		return sound;
	}

	sdl::Music TetrisData::loadMusic(const std::string& file) {
		size_t size = musics_.size();
		sdl::Music& music = musics_[file];

		// Music not found?
		if (musics_.size() > size) {
			music = sdl::Music{file};
		}

		return music;
	}

	sdl::Sprite TetrisData::loadSprite(const std::string& file) {
		return textureAtlas_.add(file, 1);;
	}

	sdl::Sprite TetrisData::getSprite(tetris::BlockType blockType) {
		switch (blockType) {
			case tetris::BlockType::I:
				return loadSprite(jsonObject_["window"]["tetrisBoard"]["sprites"]["squareI"].get<std::string>());
			case tetris::BlockType::J:
				return loadSprite(jsonObject_["window"]["tetrisBoard"]["sprites"]["squareJ"].get<std::string>());
			case tetris::BlockType::L:
				return loadSprite(jsonObject_["window"]["tetrisBoard"]["sprites"]["squareL"].get<std::string>());
			case tetris::BlockType::O:
				return loadSprite(jsonObject_["window"]["tetrisBoard"]["sprites"]["squareO"].get<std::string>());
			case tetris::BlockType::S:
				return loadSprite(jsonObject_["window"]["tetrisBoard"]["sprites"]["squareS"].get<std::string>());
			case tetris::BlockType::T:
				return loadSprite(jsonObject_["window"]["tetrisBoard"]["sprites"]["squareT"].get<std::string>());
			case tetris::BlockType::Z:
				return loadSprite(jsonObject_["window"]["tetrisBoard"]["sprites"]["squareZ"].get<std::string>());
		}
		return {};
	}

	const sdl::Font& TetrisData::getDefaultFont(int size) {
		return loadFont(jsonObject_["window"]["font"].get<std::string>(), size);
	}

	ImFont* TetrisData::getImGuiDefaultFont() const {
		if (!defaultFont_) {
			defaultFont_ = ImGui::GetIO().Fonts->AddFontFromFileTTF("fonts/Ubuntu-B.ttf", 16);
		}

		return defaultFont_;
	}

	ImFont* TetrisData::getImGuiHeaderFont() const {
		if (!headerFont_) {
			headerFont_ = ImGui::GetIO().Fonts->AddFontFromFileTTF("fonts/Ubuntu-B.ttf", 50);
		}

		return headerFont_;
	}

	ImFont* TetrisData::getImGuiButtonFont() const {
		if (!buttonFont_) {
			buttonFont_ = ImGui::GetIO().Fonts->AddFontFromFileTTF("fonts/Ubuntu-B.ttf", 35);
		}

		return buttonFont_;
	}

	void TetrisData::bindTextureFromAtlas() const {
		textureAtlas_.get().bindTexture();
	}

	sdl::Color TetrisData::getOuterSquareColor() const {
		return jsonObject_["window"]["tetrisBoard"]["outerSquareColor"].get<sdl::Color>();
	}

	sdl::Color TetrisData::getInnerSquareColor() const {
		return jsonObject_["window"]["tetrisBoard"]["innerSquareColor"].get<sdl::Color>();
	}

	sdl::Color TetrisData::getStartAreaColor() const {
		return jsonObject_["window"]["tetrisBoard"]["startAreaColor"].get<sdl::Color>();
	}

	sdl::Color TetrisData::getPlayerAreaColor() const {
		return jsonObject_["window"]["tetrisBoard"]["playerAreaColor"].get<sdl::Color>();
	}

	sdl::Color TetrisData::getBorderColor() const {
		return jsonObject_["window"]["tetrisBoard"]["borderColor"].get<sdl::Color>();
	}


	bool TetrisData::isShowDownBlock() const {
		try {
			return jsonObject_.at("window").at("tetrisBoard").at("showDownBlock").get<bool>();
		} catch (nlohmann::detail::out_of_range&) {
			return true;
		}
	}

	void TetrisData::setShowDownBlock(bool showDownColor) {
		jsonObject_["window"]["tetrisBoard"]["showDownBlock"] = showDownColor;
	}

	sdl::Color TetrisData::getDownBlockColor() const {
		try {
			return jsonObject_.at("window").at("tetrisBoard").at("downBlockColor").get<sdl::Color>();
		} catch (nlohmann::detail::out_of_range) {
			return sdl::Color{1.f, 1.f, 1.f, 0.15f};
		}
	}

	float TetrisData::getTetrisSquareSize() const {
		return jsonObject_["window"]["tetrisBoard"]["squareSize"].get<float>();
	}

	float TetrisData::getTetrisBorderSize() const {
		return jsonObject_["window"]["tetrisBoard"]["borderSize"].get<float>();
	}

	bool TetrisData::isLimitFps() const {
		try {
			return jsonObject_.at("window").at("limitFps").get<bool>();
		} catch (const nlohmann::detail::out_of_range&) {
			return false;
		}
	}

	void TetrisData::setLimitFps(bool limited) {
		jsonObject_["window"]["limitFps"] = limited;
	}

	int TetrisData::getWindowPositionX() const {
		return jsonObject_["window"]["positionX"].get<int>();
	}

	int TetrisData::getWindowPositionY() const {
		return jsonObject_["window"]["positionY"].get<int>();
	}

	void TetrisData::setWindowPositionX(int x) {
		jsonObject_["window"]["positionX"] = x;
	}

	void TetrisData::setWindowPositionY(int y) {
		jsonObject_["window"]["positionY"] = y;
	}

	int TetrisData::getWindowWidth() const {
		return jsonObject_["window"]["width"].get<int>();
	}

	int TetrisData::getWindowHeight() const {
		return jsonObject_["window"]["height"].get<int>();
	}

	void TetrisData::setWindowWidth(int width) {
		jsonObject_["window"]["width"] = width;
	}

	void TetrisData::setWindowHeight(int height) {
		jsonObject_["window"]["height"] = height;
	}

	bool TetrisData::isWindowResizable() const {
		return jsonObject_["window"]["resizeable"].get<bool>();
	}

	void TetrisData::setWindowResizable(bool resizeable) {
		jsonObject_["window"]["resizeable"] = resizeable;
	}

	int TetrisData::getWindowMinWidth() const {
		return jsonObject_["window"]["minWidth"].get<int>();
	}

	int TetrisData::getWindowMinHeight() const {
		return jsonObject_["window"]["minHeight"].get<int>();
	}

	std::string TetrisData::getWindowIcon() const {
		return jsonObject_["window"]["icon"].get<std::string>();
	}

	bool TetrisData::isWindowBordered() const {
		return jsonObject_["window"]["border"].get<bool>();
	}

	void TetrisData::setWindowBordered(bool border) {
		jsonObject_["window"]["border"] = border;
	}

	bool TetrisData::isWindowPauseOnLostFocus() const {
		try {
			return jsonObject_.at("window").at("pauseOnLostFocus").get<bool>();
		} catch (const nlohmann::detail::out_of_range&) {
			return true;
		}
	}

	void TetrisData::setWindowPauseOnLostFocus(bool pauseOnFocus) {
		jsonObject_["window"]["pauseOnLostFocus"] = pauseOnFocus;
	}

	bool TetrisData::isWindowMaximized() const {
		return jsonObject_["window"]["maximized"].get<bool>();
	}

	void TetrisData::setWindowMaximized(bool maximized) {
		jsonObject_["window"]["maximized"] = maximized;
	}

	bool TetrisData::isWindowVsync() const {
		return jsonObject_["window"]["vsync"].get<bool>();
	}

	void TetrisData::setWindowVsync(bool activate) {
		jsonObject_["window"]["vsync"] = activate;
	}

	int TetrisData::getMultiSampleBuffers() const {
		return jsonObject_["window"]["multiSampleBuffers"].get<int>();
	}

	int TetrisData::getMultiSampleSamples() const {
		return jsonObject_["window"]["multiSampleSamples"].get<int>();
	}

	float TetrisData::getRowFadingTime() const {
		return jsonObject_["window"]["rowFadingTime"].get<float>();
	}

	void TetrisData::setRowFadingTime(float time) {
		jsonObject_["window"]["rowFadingTime"] = time;
	}

	float TetrisData::getRowMovingTime() const {
		return jsonObject_["window"]["rowMovingTime"].get<float>();
	}

	void TetrisData::setRowMovingTime(float time) {
		jsonObject_["window"]["rowMovingTime"] = time;
	}

	sdl::Sprite TetrisData::getBackgroundSprite() {
		return loadSprite(jsonObject_["window"]["sprites"]["background"].get<std::string>());
	}

	std::string TetrisData::getAi1Name() const {
		return jsonObject_["ai1"].get<std::string>();
	}
	std::string TetrisData::getAi2Name() const {
		return jsonObject_["ai2"].get<std::string>();
	}
	std::string TetrisData::getAi3Name() const {
		return jsonObject_["ai3"].get<std::string>();
	}
	std::string TetrisData::getAi4Name() const {
		return jsonObject_["ai4"].get<std::string>();
	}

	void TetrisData::setAi1Name(const std::string& name) {
		jsonObject_["ai1"] = name;
	}

	void TetrisData::setAi2Name(const std::string& name) {
		jsonObject_["ai2"] = name;
	}

	void TetrisData::setAi3Name(const std::string& name) {
		jsonObject_["ai3"] = name;
	}

	void TetrisData::setAi4Name(const std::string& name) {
		jsonObject_["ai4"] = name;
	}

	std::vector<tetris::Ai> TetrisData::getAiVector() const {
		std::vector<tetris::Ai> ais;
		ais.push_back({});
		ais.insert(ais.end(), jsonObject_["ais"].begin(), jsonObject_["ais"].end());
		return ais;
	}

	std::vector<HighscoreRecord> TetrisData::getHighscoreRecordVector() const {
		return std::vector<HighscoreRecord>(jsonObject_["highscore"].begin(), jsonObject_["highscore"].end());
	}

	void TetrisData::setHighscoreRecordVector(const std::vector<HighscoreRecord>& highscoreVector) {
		jsonObject_["highscore"].clear();
		for (const auto& record : highscoreVector) {
			jsonObject_["highscore"].push_back(record);
		}
	}

	int TetrisData::getActiveLocalGameRows() const {
		return jsonObject_["activeGames"]["localGame"]["rows"].get<int>();
	}

	int TetrisData::getActiveLocalGameColumns() const {
		return jsonObject_["activeGames"]["localGame"]["columns"].get<int>();
	}

	bool TetrisData::isFullscreenOnDoubleClick() const {
		return jsonObject_["window"]["fullscreenOnDoubleClick"].get<bool>();
	}

	void TetrisData::setFullscreenOnDoubleClick(bool activate) {
		jsonObject_["window"]["fullscreenOnDoubleClick"] = activate;
	}

	bool TetrisData::isMoveWindowByHoldingDownMouse() const {
		return jsonObject_["window"]["moveWindowByHoldingDownMouse"].get<bool>();
	}

	void TetrisData::setMoveWindowByHoldingDownMouse(bool activate) {
		jsonObject_["window"]["moveWindowByHoldingDownMouse"] = activate;
	}

	int TetrisData::getPort() const {
		return jsonObject_["window"]["port"].get<int>();
	}

	void TetrisData::setPort(int port) {
		jsonObject_["window"]["port"] = port;
	}

	int TetrisData::getTimeToConnectMS() const {
		return jsonObject_["window"]["timeToConnectMS"].get<int>();
	}

	std::string TetrisData::getIp() const {
		return jsonObject_["window"]["ip"].get<std::string>();
	}

	void TetrisData::setIp(const std::string& ip) {
		jsonObject_["window"]["ip"] = ip;
	}

	float TetrisData::getWindowBarHeight() const {
		return jsonObject_["window"]["bar"]["height"].get<float>();
	}

	sdl::Color TetrisData::getWindowBarColor() const {
		return jsonObject_["window"]["bar"]["color"].get<sdl::Color>();
	}

	sdl::Sprite TetrisData::getCheckboxBoxSprite() {
		return loadSprite(jsonObject_["window"]["checkBox"]["boxImage"].get<std::string>());
	}

	sdl::Sprite TetrisData::getCheckboxCheckSprite() {
		return loadSprite(jsonObject_["window"]["checkBox"]["checkImage"].get<std::string>());
	}

	sdl::Color TetrisData::getCheckboxTextColor() const {
		return jsonObject_["window"]["checkBox"]["textColor"].get<sdl::Color>();
	}

	sdl::Color TetrisData::getCheckboxBackgroundColor() const {
		return jsonObject_["window"]["checkBox"]["backgroundColor"].get<sdl::Color>();
	}

	sdl::Color TetrisData::getCheckboxBoxColor() const {
		return jsonObject_["window"]["checkBox"]["boxColor"].get<sdl::Color>();
	}

	sdl::Color TetrisData::getChecboxCheckColor() const {
		return jsonObject_["window"]["checkBox"]["checkColor"].get<sdl::Color>();
	}

	sdl::Sprite TetrisData::getRadioButtonBoxSprite() {
		return loadSprite(jsonObject_["window"]["radioButton"]["boxImage"].get<std::string>());
	}

	sdl::Sprite TetrisData::getRadioButtonCheckSprite() {
		return loadSprite(jsonObject_["window"]["radioButton"]["checkImage"].get<std::string>());
	}

	sdl::Color TetrisData::getRadioButtonTextColor() const {
		return jsonObject_["window"]["radioButton"]["textColor"].get<sdl::Color>();
	}

	sdl::Color TetrisData::getRadioButtonBackgroundColor() const {
		return jsonObject_["window"]["radioButton"]["backgroundColor"].get<sdl::Color>();
	}

	sdl::Color TetrisData::getRadioButtonBoxColor() const {
		return jsonObject_["window"]["radioButton"]["boxColor"].get<sdl::Color>();
	}

	sdl::Color TetrisData::getRadioButtonCheckColor() const {
		return jsonObject_["window"]["radioButton"]["checkColor"].get<sdl::Color>();
	}

	sdl::Color TetrisData::getLabelTextColor() const {
		return jsonObject_["window"]["label"]["textColor"].get<sdl::Color>();
	}

	sdl::Color TetrisData::getLabelBackgroundColor() const {
		return jsonObject_["window"]["label"]["backgroundColor"].get<sdl::Color>();
	}

	sdl::Color TetrisData::getButtonFocusColor() const {
		return jsonObject_["window"]["button"]["focusColor"].get<sdl::Color>();
	}

	sdl::Color TetrisData::getButtonTextColor() const {
		return jsonObject_["window"]["button"]["textColor"].get<sdl::Color>();
	}

	sdl::Color TetrisData::getButtonHoverColor() const {
		return jsonObject_["window"]["button"]["hoverColor"].get<sdl::Color>();
	}

	sdl::Color TetrisData::getButtonPushColor() const {
		return jsonObject_["window"]["button"]["pushColor"].get<sdl::Color>();
	}

	sdl::Color TetrisData::getButtonBackgroundColor() const {
		return jsonObject_["window"]["button"]["backgroundColor"].get<sdl::Color>();
	}

	sdl::Color TetrisData::getButtonBorderColor() const {
		return jsonObject_["window"]["button"]["borderColor"].get<sdl::Color>();
	}

	sdl::Color TetrisData::getComboBoxFocusColor() const {
		return jsonObject_["window"]["comboBox"]["focusColor"].get<sdl::Color>();
	}

	sdl::Color TetrisData::getComboBoxTextColor() const {
		return jsonObject_["window"]["comboBox"]["textColor"].get<sdl::Color>();
	}

	sdl::Color TetrisData::getComboBoxSelectedTextColor() const {
		return jsonObject_["window"]["comboBox"]["selectedTextColor"].get<sdl::Color>();
	}

	sdl::Color TetrisData::getComboBoxSelectedBackgroundColor() const {
		return jsonObject_["window"]["comboBox"]["selectedBackgroundColor"].get<sdl::Color>();
	}

	sdl::Color TetrisData::getComboBoxBackgroundColor() const {
		return jsonObject_["window"]["comboBox"]["backgroundColor"].get<sdl::Color>();
	}

	sdl::Color TetrisData::getComboBoxBorderColor() const {
		return jsonObject_["window"]["comboBox"]["borderColor"].get<sdl::Color>();
	}

	sdl::Color TetrisData::getComboBoxShowDropDownColor() const {
		return jsonObject_["window"]["comboBox"]["showDropDownColor"].get<sdl::Color>();
	}

	sdl::Sprite TetrisData::getComboBoxShowDropDownSprite() {
		return loadSprite(jsonObject_["window"]["comboBox"]["showDropDownSprite"].get<std::string>());
	}

	sdl::Sprite TetrisData::getHumanSprite() {
		return loadSprite(jsonObject_["window"]["sprites"]["human"].get<std::string>());
	}

	sdl::Sprite TetrisData::getComputerSprite() {
		return loadSprite(jsonObject_["window"]["sprites"]["computer"].get<std::string>());
	}

	sdl::Sprite TetrisData::getCrossSprite() {
		return loadSprite(jsonObject_["window"]["sprites"]["cross"].get<std::string>());
	}

	sdl::Sprite TetrisData::getZoomSprite() {
		return loadSprite(jsonObject_["window"]["sprites"]["zoom"].get<std::string>());
	}

	sdl::Color TetrisData::getMiddleTextColor() const {
		try {
			return jsonObject_.at("window").at("tetrisBoard").at("middleTextColor").get<sdl::Color>();
		} catch (const nlohmann::detail::out_of_range&) {
			return sdl::Color{0.2f, 0.2f, 0.2f, 0.5f};
		}
	}

	int TetrisData::getMiddleTextBoxSize() const {
		try {
			return jsonObject_.at("window").at("tetrisBoard").at("middleTextBoxSize").get<int>();
		} catch (const nlohmann::detail::out_of_range&) {
			return 7;
		}
	}
	/*
	void TetrisData::setActiveLocalGame(int columns, int rows, const std::vector<PlayerData>& playerDataVector) {
		jsonObject_["activeGames"]["localGame"]["columns"] = columns;
		jsonObject_["activeGames"]["localGame"]["rows"] = rows;

		nlohmann::json playerJson = nlohmann::json::array();
		for (const PlayerData& data : playerDataVector) {
			nlohmann::json test = nlohmann::json(data.board_);

			playerJson.push_back({
				{"name", data.name_},
				{"lastPosition", data.lastPosition_},
				{"nextBlockType", data.next_},
				{"levelUpCounter", data.levelUpCounter_},
				{"ai", data.device_->isAi()},
				{"level", data.level_},
				{"points", data.points_},
				{"clearedRows", data.clearedRows_},
				{"currentBlock", data.current_},
				{"board", convertBlockTypesToString(data.board_)},
				{"device",{
					{"name", data.device_->getName()},
					{"ai", data.device_->isAi()},
				}}
				});
		}
		jsonObject_["activeGames"]["localGame"]["players"] = playerJson;
	}
	*/

	/*
	std::vector<game::PlayerData> TetrisData::getActiveLocalGamePlayers() {
		nlohmann::json players = jsonObject_["activeGames"]["localGame"]["players"];

		std::vector<game::PlayerData> playerDataVector;
		for (nlohmann::json& player : players) {
			game::PlayerData playerData;
			playerData.name_ = player["name"].get<std::string>();
			playerData.lastPosition_ = player["lastPosition"].get<int>();
			playerData.next_ = player["nextBlockType"].get<BlockType>();
			playerData.levelUpCounter_ = player["levelUpCounter"].get<int>();
			playerData.level_ = player["level"].get<int>();
			playerData.points_ = player["points"].get<int>();
			playerData.clearedRows_ = player["clearedRows"].get<int>();
			playerData.current_ = player["currentBlock"].get<Block>();
			playerData.board_ = convertStringToBlockTypes(player["board"].get<std::string>());
			playerData.ai_ = player["ai"].get<bool>();
			playerData.deviceName_ = player["device"]["name"].get<std::string>();
			playerDataVector.push_back(playerData);
		}
		return playerDataVector;
	}
	*/

}
