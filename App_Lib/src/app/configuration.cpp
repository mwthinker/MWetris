#include "configuration.h"

#include <sdl/imageatlas.h>
#include <sdl/util.h>
#include <sdl/sdlexception.h>
#include <sdl/gpu/gpuutil.h>

#include <IconsFontAwesome6.h>
#include <nlohmann/json.hpp>
#include <fmt/format.h>
#include <spdlog/spdlog.h>

#include <filesystem>
#include <map>
#include <vector>
#include <fstream>
#include <sstream>

using nlohmann::json;
using namespace app;

namespace sdl {

	void from_json(const json& j, Color& color) {
		auto textColor = j.get<std::string>();
		if (sdl::color::isValidHexColor(textColor)) {
			color = Color{textColor};
		} else {
			throw std::runtime_error{fmt::format("Invalid hex color: ", color.toHexString())};
		}
	}

}

namespace app {

	void from_json(const json& j, Configuration::Ai& ai) {
		auto name = j.at("name").get<std::string>();
		auto valueFunction = j.at("valueFunction").get<std::string>();

		ai = Configuration::Ai{
			.name = name,
			.ai = valueFunction,
		};
	}

	void from_json(const json& j, Configuration::Device& deviceProperties) {
		deviceProperties = Configuration::Device{
			.guid = j.at("guid").get<std::string>(),
			.das = j.at("das").get<double>(),
			.arr = j.at("arr").get<double>(),
		};
	}

	void to_json(json& j, const Configuration::Device& deviceProperties) {
		j = json{
			{"guid", deviceProperties.guid},
			{"das", deviceProperties.das},
			{"arr", deviceProperties.arr}
		};
	}

}

namespace tetris {

	void from_json(const json& j, tetris::BlockType& blockType) {
		blockType = charToBlockType(j.get<std::string>()[0]);
	}

	void to_json(json& j, const tetris::BlockType& blockType) {
		j = json{blockTypeToString(blockType)};
	}

	void from_json(const json& j, tetris::Block& block) {
		block = tetris::Block{
			j.at("blockType").get<tetris::BlockType>(),
			j.at("leftColumn").get<int>(),
			j.at("bottomRow").get<int>(),
			j.at("currentRotation").get<int>()
		};
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

namespace app {

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

	void from_json(const json& j, Configuration::Network& network) {
		network = Configuration::Network{
			.global = {
				.ip = j.at("globalServer").at("ip").get<std::string>(),
				.port = j.at("globalServer").at("port").get<int>()
			},
			.server = {
				.ip = j.at("server").at("ip").get<std::string>(),
				.port = j.at("server").at("port").get<int>()
			}
		};
	}

	void from_json(const json& j, HighscoreRecord& highscoreRecord) {
		highscoreRecord.name = j.at("name").get<std::string>();
		highscoreRecord.date = j.at("date").get<std::string>();
		highscoreRecord.points = j.at("points").get<int>();

		try {
			highscoreRecord.level = j.at("level").get<int>();
		} catch (const nlohmann::detail::out_of_range&) {
			highscoreRecord.level = 0;
		}

		try {
			highscoreRecord.rows = j.at("rows").get<int>();
		} catch (const nlohmann::detail::out_of_range&) {
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

	struct Configuration::Impl {
		std::string jsonPath;
		nlohmann::json jsonObject;

		mutable ImFont* defaultFont{};
		mutable ImFont* headerFont{};
		mutable ImFont* buttonFont{};
	};

	Configuration& Configuration::getInstance() {
		static Configuration instance;
		return instance;
	}

	void Configuration::quit() {
		impl_ = nullptr;

		// A bit of a hack. Resource needs to be destructed before GpuContext is destroyed.
		sampler_ = {};
		atlasTexture_ = {};
	}

	Configuration::Configuration()
		: impl_{std::make_unique<Configuration::Impl>()} {

		std::ifstream defaultStream{"USE_APPLICATION_JSON"};
		bool applicationJson;
		defaultStream >> applicationJson;
		const std::string APPLICATION_JSON{"tetris.json"};
		if (applicationJson) {
			impl_->jsonPath = APPLICATION_JSON;
		} else {
			// Find default path to save/load file from.
			impl_->jsonPath = SDL_GetPrefPath("mwthinker", "MWetris2") + APPLICATION_JSON;
		}
		std::ifstream stream(impl_->jsonPath);
		if (!stream.is_open()) {
			// Assume that the file does not exist, load file from application folder.
			stream = std::ifstream(APPLICATION_JSON);
		}
		stream >> impl_->jsonObject;
	}

	void Configuration::save() {
		std::ofstream stream{impl_->jsonPath};
		stream << impl_->jsonObject.dump(1);
	}

	namespace {

		SDL_Surface* createSurface(int w, int h, sdl::Color color) {
			auto s = SDL_CreateSurface(w, h, SDL_PIXELFORMAT_RGBA32);
			SDL_FillSurfaceRect(s, nullptr, color.toImU32());
			return s;
		}

		app::TextureView addImage(sdl::ImageAtlas& atlas, SDL_Surface* surfaceAtlas, const std::string& filename, int border = 0) {
			auto surface = sdl::createSdlSurface(IMG_Load(filename.c_str()));
			if (!surface) {
				throw sdl::SdlException{"Failed to load surface from file: " + filename};
			}
			auto rectOptional = atlas.add(surface->w, surface->h, border);
			if (!rectOptional) {
				throw std::runtime_error{fmt::format("Failed to add surface to atlas: {}x{}, format: {}",
					surface->w, surface->h, SDL_GetPixelFormatName(surface->format))};
			}

			auto rectDst = *rectOptional;
			if (SDL_BlitSurface(surface.get(), nullptr, surfaceAtlas, &rectDst)) {
				spdlog::info("Added surface to atlas: {}x{}, format: {}, at position: {},{}",
					surface->w, surface->h, SDL_GetPixelFormatName(surface->format), rectDst.x, rectDst.y);
				return app::TextureView{
					.pos = {static_cast<float>(rectDst.x) / surfaceAtlas->w, static_cast<float>(rectDst.y) / surfaceAtlas->h},
					.size = {static_cast<float>(rectDst.w) / surfaceAtlas->w, static_cast<float>(rectDst.h) / surfaceAtlas->h},
				};
			} else {
				throw sdl::SdlException{"Failed to blit surface to atlas: " + filename + ", error: " + SDL_GetError()};
			}
		}

	}

	void Configuration::init(sdl::gpu::GpuContext& context) {
		std::vector<std::string> files;
		sdl::ImageAtlas atlas{2048, 2048};
		auto surfaceAtlas = sdl::createSdlSurface(createSurface(atlas.getWidth(), atlas.getHeight(), sdl::color::White));

		blockTypeI_ = addImage(atlas, surfaceAtlas.get(), impl_->jsonObject["window"]["tetrisBoard"]["sprites"]["squareI"].get<std::string>());
		blockTypeJ_ = addImage(atlas, surfaceAtlas.get(), impl_->jsonObject["window"]["tetrisBoard"]["sprites"]["squareJ"].get<std::string>());
		blockTypeL_ = addImage(atlas, surfaceAtlas.get(), impl_->jsonObject["window"]["tetrisBoard"]["sprites"]["squareL"].get<std::string>());
		blockTypeO_ = addImage(atlas, surfaceAtlas.get(), impl_->jsonObject["window"]["tetrisBoard"]["sprites"]["squareO"].get<std::string>());
		blockTypeS_ = addImage(atlas, surfaceAtlas.get(), impl_->jsonObject["window"]["tetrisBoard"]["sprites"]["squareS"].get<std::string>());
		blockTypeT_ = addImage(atlas, surfaceAtlas.get(), impl_->jsonObject["window"]["tetrisBoard"]["sprites"]["squareT"].get<std::string>());
		blockTypeZ_ = addImage(atlas, surfaceAtlas.get(), impl_->jsonObject["window"]["tetrisBoard"]["sprites"]["squareZ"].get<std::string>());
		background_ = addImage(atlas, surfaceAtlas.get(), impl_->jsonObject["window"]["sprites"]["background"].get<std::string>());
		
		atlasTexture_ = sdl::gpu::uploadSurface(context, surfaceAtlas.get());
		sampler_ = sdl::gpu::createSampler(context, SDL_GPUSamplerCreateInfo{
			.min_filter = SDL_GPU_FILTER_NEAREST,
			.mag_filter = SDL_GPU_FILTER_NEAREST,
			.mipmap_mode = SDL_GPU_SAMPLERMIPMAPMODE_NEAREST,
			.address_mode_u = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE,
			.address_mode_v = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE,
			.address_mode_w = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE
		});
		atlasBinding_ = SDL_GPUTextureSamplerBinding{
			.texture = atlasTexture_.get(),
			.sampler = sampler_.get()
		};
	}

	app::TextureView Configuration::getSprite(tetris::BlockType blockType) {
		switch (blockType) {
			case tetris::BlockType::I:
				return blockTypeI_;
			case tetris::BlockType::J:
				return blockTypeJ_;
			case tetris::BlockType::L:
				return blockTypeL_;
			case tetris::BlockType::O:
				return blockTypeO_;
			case tetris::BlockType::S:
				return blockTypeS_;
			case tetris::BlockType::T:
				return blockTypeT_;
			case tetris::BlockType::Z:
				return blockTypeZ_;
		}
		return {};
	}

	ImFont* Configuration::getImGuiDefaultFont() const {
		if (!impl_->defaultFont) {
			auto& io = ImGui::GetIO();

			impl_->defaultFont = io.Fonts->AddFontFromFileTTF("fonts/Ubuntu-B.ttf", 16);
			
			float baseFontSize = 13.0f; // 13.0f is the size of the default font. Change to the font size you use.
			float iconFontSize = baseFontSize * 3.0f / 3.0f;

			static const ImWchar icons_ranges[] = {ICON_MIN_FA, ICON_MAX_16_FA, 0};
			ImFontConfig icons_config;
			icons_config.MergeMode = true;
			icons_config.PixelSnapH = true;
			icons_config.GlyphMinAdvanceX = iconFontSize;

			auto file = fmt::format("fonts/{}", FONT_ICON_FILE_NAME_FAS);
			io.Fonts->AddFontFromFileTTF(file.c_str(), iconFontSize, &icons_config, icons_ranges);
		}

		return impl_->defaultFont;
	}

	ImFont* Configuration::getImGuiHeaderFont() const {
		if (!impl_->headerFont) {
			impl_->headerFont = ImGui::GetIO().Fonts->AddFontFromFileTTF("fonts/Ubuntu-B.ttf", 50);
		}

		return impl_->headerFont;
	}

	ImFont* Configuration::getImGuiButtonFont() const {
		if (!impl_->buttonFont) {
			impl_->buttonFont = ImGui::GetIO().Fonts->AddFontFromFileTTF("fonts/Ubuntu-B.ttf", 35);
		}

		return impl_->buttonFont;
	}

	sdl::Color Configuration::getOuterSquareColor() const {
		return impl_->jsonObject["window"]["tetrisBoard"]["outerSquareColor"].get<sdl::Color>();
	}

	sdl::Color Configuration::getInnerSquareColor() const {
		return impl_->jsonObject["window"]["tetrisBoard"]["innerSquareColor"].get<sdl::Color>();
	}

	sdl::Color Configuration::getStartAreaColor() const {
		return impl_->jsonObject["window"]["tetrisBoard"]["startAreaColor"].get<sdl::Color>();
	}

	sdl::Color Configuration::getPlayerAreaColor() const {
		return impl_->jsonObject["window"]["tetrisBoard"]["playerAreaColor"].get<sdl::Color>();
	}

	sdl::Color Configuration::getBorderColor() const {
		return impl_->jsonObject["window"]["tetrisBoard"]["borderColor"].get<sdl::Color>();
	}

	bool Configuration::isShowDownBlock() const {
		try {
			return impl_->jsonObject.at("window").at("tetrisBoard").at("showDownBlock").get<bool>();
		} catch (const nlohmann::detail::out_of_range&) {
			return true;
		}
	}

	void Configuration::setShowDownBlock(bool showDownColor) {
		impl_->jsonObject["window"]["tetrisBoard"]["showDownBlock"] = showDownColor;
	}

	sdl::Color Configuration::getDownBlockColor() const {
		try {
			return impl_->jsonObject.at("window").at("tetrisBoard").at("downBlockColor").get<sdl::Color>();
		} catch (const nlohmann::detail::out_of_range&) {
			return sdl::Color{1.f, 1.f, 1.f, 0.15f};
		}
	}

	float Configuration::getTetrisSquareSize() const {
		return impl_->jsonObject["window"]["tetrisBoard"]["squareSize"].get<float>();
	}

	float Configuration::getTetrisBorderSize() const {
		return impl_->jsonObject["window"]["tetrisBoard"]["borderSize"].get<float>();
	}

	bool Configuration::isLimitFps() const {
		try {
			return impl_->jsonObject.at("window").at("limitFps").get<bool>();
		} catch (const nlohmann::detail::out_of_range&) {
			return false;
		}
	}

	void Configuration::setLimitFps(bool limited) {
		impl_->jsonObject["window"]["limitFps"] = limited;
	}

	int Configuration::getWindowPositionX() const {
		return impl_->jsonObject["window"]["positionX"].get<int>();
	}

	int Configuration::getWindowPositionY() const {
		return impl_->jsonObject["window"]["positionY"].get<int>();
	}

	void Configuration::setWindowPositionX(int x) {
		impl_->jsonObject["window"]["positionX"] = x;
	}

	void Configuration::setWindowPositionY(int y) {
		impl_->jsonObject["window"]["positionY"] = y;
	}

	int Configuration::getWindowWidth() const {
		return impl_->jsonObject["window"]["width"].get<int>();
	}

	int Configuration::getWindowHeight() const {
		return impl_->jsonObject["window"]["height"].get<int>();
	}

	void Configuration::setWindowWidth(int width) {
		impl_->jsonObject["window"]["width"] = width;
	}

	void Configuration::setWindowHeight(int height) {
		impl_->jsonObject["window"]["height"] = height;
	}

	bool Configuration::isWindowResizable() const {
		return impl_->jsonObject["window"]["resizeable"].get<bool>();
	}

	void Configuration::setWindowResizable(bool resizeable) {
		impl_->jsonObject["window"]["resizeable"] = resizeable;
	}

	int Configuration::getWindowMinWidth() const {
		return impl_->jsonObject["window"]["minWidth"].get<int>();
	}

	int Configuration::getWindowMinHeight() const {
		return impl_->jsonObject["window"]["minHeight"].get<int>();
	}

	std::string Configuration::getWindowIcon() const {
		return impl_->jsonObject["window"]["icon"].get<std::string>();
	}

	bool Configuration::isWindowBordered() const {
		return impl_->jsonObject["window"]["border"].get<bool>();
	}

	void Configuration::setWindowBordered(bool border) {
		impl_->jsonObject["window"]["border"] = border;
	}

	bool Configuration::isWindowPauseOnLostFocus() const {
		try {
			return impl_->jsonObject.at("window").at("pauseOnLostFocus").get<bool>();
		} catch (const nlohmann::detail::out_of_range&) {
			return true;
		}
	}

	void Configuration::setWindowPauseOnLostFocus(bool pauseOnFocus) {
		impl_->jsonObject["window"]["pauseOnLostFocus"] = pauseOnFocus;
	}

	bool Configuration::isWindowMaximized() const {
		return impl_->jsonObject["window"]["maximized"].get<bool>();
	}

	void Configuration::setWindowMaximized(bool maximized) {
		impl_->jsonObject["window"]["maximized"] = maximized;
	}

	bool Configuration::isWindowVsync() const {
		return impl_->jsonObject["window"]["vsync"].get<bool>();
	}

	void Configuration::setWindowVsync(bool activate) {
		impl_->jsonObject["window"]["vsync"] = activate;
	}

	int Configuration::getMultiSampleBuffers() const {
		return impl_->jsonObject["window"]["multiSampleBuffers"].get<int>();
	}

	int Configuration::getMultiSampleSamples() const {
		return impl_->jsonObject["window"]["multiSampleSamples"].get<int>();
	}

	float Configuration::getRowFadingTime() const {
		return impl_->jsonObject["window"]["rowFadingTime"].get<float>();
	}

	void Configuration::setRowFadingTime(float time) {
		impl_->jsonObject["window"]["rowFadingTime"] = time;
	}

	float Configuration::getRowMovingTime() const {
		return impl_->jsonObject["window"]["rowMovingTime"].get<float>();
	}

	void Configuration::setRowMovingTime(float time) {
		impl_->jsonObject["window"]["rowMovingTime"] = time;
	}

	app::TextureView Configuration::getBackgroundSprite() {
		return background_;
	}

	std::string Configuration::getAi1Name() const {
		return impl_->jsonObject["ai1"].get<std::string>();
	}
	std::string Configuration::getAi2Name() const {
		return impl_->jsonObject["ai2"].get<std::string>();
	}
	std::string Configuration::getAi3Name() const {
		return impl_->jsonObject["ai3"].get<std::string>();
	}
	std::string Configuration::getAi4Name() const {
		return impl_->jsonObject["ai4"].get<std::string>();
	}

	void Configuration::setAi1Name(const std::string& name) {
		impl_->jsonObject["ai1"] = name;
	}

	void Configuration::setAi2Name(const std::string& name) {
		impl_->jsonObject["ai2"] = name;
	}

	void Configuration::setAi3Name(const std::string& name) {
		impl_->jsonObject["ai3"] = name;
	}

	void Configuration::setAi4Name(const std::string& name) {
		impl_->jsonObject["ai4"] = name;
	}

	std::vector<Configuration::Ai> Configuration::getAiVector() const {
		std::vector<Configuration::Ai> ais = impl_->jsonObject["ais"].get<std::vector<Configuration::Ai>>();
		ais.push_back(Configuration::Ai{
			.name = "Default",
			.ai = {}
		});
		return ais;
	}

	std::vector<HighscoreRecord> Configuration::getHighscoreRecordVector() const {
		return std::vector<HighscoreRecord>(impl_->jsonObject["highscore"].begin(), impl_->jsonObject["highscore"].end());
	}

	void Configuration::setHighscoreRecordVector(const std::vector<HighscoreRecord>& highscoreVector) {
		impl_->jsonObject["highscore"].clear();
		for (const auto& record : highscoreVector) {
			impl_->jsonObject["highscore"].push_back(record);
		}
	}

	int Configuration::getActiveLocalGameRows() const {
		return impl_->jsonObject["activeGames"]["localGame"]["rows"].get<int>();
	}

	int Configuration::getActiveLocalGameColumns() const {
		return impl_->jsonObject["activeGames"]["localGame"]["columns"].get<int>();
	}

	bool Configuration::isFullscreenOnDoubleClick() const {
		return impl_->jsonObject["window"]["fullscreenOnDoubleClick"].get<bool>();
	}

	void Configuration::setFullscreenOnDoubleClick(bool activate) {
		impl_->jsonObject["window"]["fullscreenOnDoubleClick"] = activate;
	}

	bool Configuration::isMoveWindowByHoldingDownMouse() const {
		return impl_->jsonObject["window"]["moveWindowByHoldingDownMouse"].get<bool>();
	}

	void Configuration::setMoveWindowByHoldingDownMouse(bool activate) {
		impl_->jsonObject["window"]["moveWindowByHoldingDownMouse"] = activate;
	}

	Configuration::Network Configuration::getNetwork() const {
		return impl_->jsonObject["network"].get<Configuration::Network>();
	}

	float Configuration::getWindowBarHeight() const {
		return impl_->jsonObject["window"]["bar"]["height"].get<float>();
	}

	sdl::Color Configuration::getWindowBarColor() const {
		return impl_->jsonObject["window"]["bar"]["color"].get<sdl::Color>();
	}

	sdl::Color Configuration::getCheckboxTextColor() const {
		return impl_->jsonObject["window"]["checkBox"]["textColor"].get<sdl::Color>();
	}

	sdl::Color Configuration::getCheckboxBackgroundColor() const {
		return impl_->jsonObject["window"]["checkBox"]["backgroundColor"].get<sdl::Color>();
	}

	sdl::Color Configuration::getCheckboxBoxColor() const {
		return impl_->jsonObject["window"]["checkBox"]["boxColor"].get<sdl::Color>();
	}

	sdl::Color Configuration::getChecboxCheckColor() const {
		return impl_->jsonObject["window"]["checkBox"]["checkColor"].get<sdl::Color>();
	}

	sdl::Color Configuration::getRadioButtonTextColor() const {
		return impl_->jsonObject["window"]["radioButton"]["textColor"].get<sdl::Color>();
	}

	sdl::Color Configuration::getRadioButtonBackgroundColor() const {
		return impl_->jsonObject["window"]["radioButton"]["backgroundColor"].get<sdl::Color>();
	}

	sdl::Color Configuration::getRadioButtonBoxColor() const {
		return impl_->jsonObject["window"]["radioButton"]["boxColor"].get<sdl::Color>();
	}

	sdl::Color Configuration::getRadioButtonCheckColor() const {
		return impl_->jsonObject["window"]["radioButton"]["checkColor"].get<sdl::Color>();
	}

	sdl::Color Configuration::getLabelTextColor() const {
		return impl_->jsonObject["window"]["label"]["textColor"].get<sdl::Color>();
	}

	sdl::Color Configuration::getLabelBackgroundColor() const {
		return impl_->jsonObject["window"]["label"]["backgroundColor"].get<sdl::Color>();
	}

	sdl::Color Configuration::getButtonFocusColor() const {
		return impl_->jsonObject["window"]["button"]["focusColor"].get<sdl::Color>();
	}

	sdl::Color Configuration::getButtonTextColor() const {
		return impl_->jsonObject["window"]["button"]["textColor"].get<sdl::Color>();
	}

	sdl::Color Configuration::getButtonHoverColor() const {
		return impl_->jsonObject["window"]["button"]["hoverColor"].get<sdl::Color>();
	}

	sdl::Color Configuration::getButtonPushColor() const {
		return impl_->jsonObject["window"]["button"]["pushColor"].get<sdl::Color>();
	}

	sdl::Color Configuration::getButtonBackgroundColor() const {
		return impl_->jsonObject["window"]["button"]["backgroundColor"].get<sdl::Color>();
	}

	sdl::Color Configuration::getButtonBorderColor() const {
		return impl_->jsonObject["window"]["button"]["borderColor"].get<sdl::Color>();
	}

	sdl::Color Configuration::getComboBoxFocusColor() const {
		return impl_->jsonObject["window"]["comboBox"]["focusColor"].get<sdl::Color>();
	}

	sdl::Color Configuration::getComboBoxTextColor() const {
		return impl_->jsonObject["window"]["comboBox"]["textColor"].get<sdl::Color>();
	}

	sdl::Color Configuration::getComboBoxSelectedTextColor() const {
		return impl_->jsonObject["window"]["comboBox"]["selectedTextColor"].get<sdl::Color>();
	}

	sdl::Color Configuration::getComboBoxSelectedBackgroundColor() const {
		return impl_->jsonObject["window"]["comboBox"]["selectedBackgroundColor"].get<sdl::Color>();
	}

	sdl::Color Configuration::getComboBoxBackgroundColor() const {
		return impl_->jsonObject["window"]["comboBox"]["backgroundColor"].get<sdl::Color>();
	}

	sdl::Color Configuration::getComboBoxBorderColor() const {
		return impl_->jsonObject["window"]["comboBox"]["borderColor"].get<sdl::Color>();
	}

	sdl::Color Configuration::getComboBoxShowDropDownColor() const {
		return impl_->jsonObject["window"]["comboBox"]["showDropDownColor"].get<sdl::Color>();
	}

	sdl::Color Configuration::getMiddleTextColor() const {
		try {
			return impl_->jsonObject.at("window").at("tetrisBoard").at("middleTextColor").get<sdl::Color>();
		} catch (const nlohmann::detail::out_of_range&) {
			return sdl::Color{0.2f, 0.2f, 0.2f, 0.5f};
		}
	}

	int Configuration::getMiddleTextBoxSize() const {
		try {
			return impl_->jsonObject.at("window").at("tetrisBoard").at("middleTextBoxSize").get<int>();
		} catch (const nlohmann::detail::out_of_range&) {
			return 7;
		}
	}

	std::optional<Configuration::Device> Configuration::getDevice(std::string_view guid) const {
		auto devices = impl_->jsonObject["devices"].get<std::vector<Configuration::Device>>();
		auto it = std::find_if(devices.begin(), devices.end(), [&guid](const Configuration::Device& device) {
			return device.guid == guid;
		});
		if (it != devices.end()) {
			return *it;
		}
		return std::nullopt;
	}
	
	void Configuration::setDevice(std::string_view guid, double das, double arr) {
		auto devices = impl_->jsonObject["devices"].get<std::vector<Configuration::Device>>();
		auto it = std::find_if(devices.begin(), devices.end(), [&guid](const Configuration::Device& device) {
			return device.guid == guid;
		});
		auto newDevice = Configuration::Device{
			.guid = guid.data(),
			.das = das,
			.arr = arr
		};
		if (it != devices.end()) {
			impl_->jsonObject[guid] = newDevice;
		} else {
			impl_->jsonObject.push_back(newDevice);
		}
	}

}
