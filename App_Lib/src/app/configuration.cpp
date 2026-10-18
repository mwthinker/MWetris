#include "configuration.h"

#include "util/uuid.h"

#include <sdl/imageatlas.h>
#include <sdl/util.h>
#include <sdl/sdlexception.h>
#include <sdl/gpuutil.h>
#include <SDL3/SDL_surface.h>
#include <SDL3/SDL_filesystem.h>

#include <IconsFontAwesome6.h>
#include <nlohmann/json.hpp>
#include <fmt/format.h>
#include <spdlog/spdlog.h>

#include <algorithm>
#include <charconv>
#include <cmath>
#include <filesystem>
#include <iterator>
#include <limits>
#include <map>
#include <optional>
#include <vector>
#include <fstream>
#include <sstream>
#include <unordered_set>

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
		std::string defaultsPath;
		nlohmann::json jsonObject;
		std::optional<EditableSettings> startupSettings;

		mutable ImFont* defaultFont{};
		mutable ImFont* regularSmallFont{};
		mutable ImFont* boardRegularFont{};
		mutable ImFont* mediumFont{};
		mutable ImFont* mediumValueFont{};
		mutable ImFont* semiBoldFont{};
		mutable ImFont* scoreFont{};
	};

	Configuration& Configuration::getInstance() {
		static Configuration instance;
		return instance;
	}

	void Configuration::resetRuntimeResources() {
		impl_->defaultFont = nullptr;
		impl_->regularSmallFont = nullptr;
		impl_->boardRegularFont = nullptr;
		impl_->mediumFont = nullptr;
		impl_->mediumValueFont = nullptr;
		impl_->semiBoldFont = nullptr;
		impl_->scoreFont = nullptr;
		impl_->startupSettings.reset();

		blockTypeI_ = {};
		blockTypeJ_ = {};
		blockTypeL_ = {};
		blockTypeO_ = {};
		blockTypeS_ = {};
		blockTypeT_ = {};
		blockTypeZ_ = {};
		background_ = {};

		// This resource must be destroyed before the window's GPU device.
		atlasTexture_.reset();
	}

	Configuration::Configuration()
		: impl_{std::make_unique<Configuration::Impl>()} {

		std::ifstream defaultStream{"USE_APPLICATION_JSON"};
		bool applicationJson;
		defaultStream >> applicationJson;
		const std::string APPLICATION_JSON{"tetris.json"};
		impl_->defaultsPath = "tetris.defaults.json";
		if (applicationJson) {
			impl_->jsonPath = APPLICATION_JSON;
		} else {
			// Find default path to save/load file from.
			auto preferencePath = std::unique_ptr<char, decltype(&SDL_free)>{
				SDL_GetPrefPath("mwthinker", "MWetris2"),
				SDL_free
			};
			if (!preferencePath) {
				throw sdl::SdlException{"Could not determine the configuration directory"};
			}
			impl_->jsonPath = std::string{preferencePath.get()} + APPLICATION_JSON;
		}
		std::ifstream stream(impl_->jsonPath);
		if (!stream.is_open()) {
			// Assume that the file does not exist, load file from application folder.
			stream = std::ifstream(APPLICATION_JSON);
		}
		stream >> impl_->jsonObject;
	}

	namespace {
		constexpr std::string_view ProfileNameWhitespace{" \t\r\n\f\v"};
		constexpr std::array<const char*, 7> BlockSpriteKeys{
			"squareI", "squareJ", "squareL", "squareO", "squareS", "squareT", "squareZ"
		};

		Configuration::EditableSettings editableSettingsFromJson(const json& document) {
			const auto& window = document.at("window");
			const auto& board = window.at("tetrisBoard");
			const auto& button = window.at("button");
			Configuration::EditableSettings settings{
				.window = {
					.width = window.at("width").get<int>(),
					.height = window.at("height").get<int>(),
					.minWidth = window.at("minWidth").get<int>(),
					.minHeight = window.at("minHeight").get<int>(),
					.resizable = window.at("resizeable").get<bool>(),
					.bordered = window.at("border").get<bool>(),
					.maximized = window.at("maximized").get<bool>(),
					.pauseOnLostFocus = window.at("pauseOnLostFocus").get<bool>(),
					.fullscreenOnDoubleClick = window.at("fullscreenOnDoubleClick").get<bool>(),
					.moveWindowByHoldingDownMouse = window.at("moveWindowByHoldingDownMouse").get<bool>(),
					.vsync = window.at("vsync").get<bool>(),
					.limitFps = window.at("limitFps").get<bool>(),
				},
				.board = {
					.borderSize = board.at("borderSize").get<float>(),
					.showDownBlock = board.at("showDownBlock").get<bool>(),
					.outerSquareColor = board.at("outerSquareColor").get<std::string>(),
					.innerSquareColor = board.at("innerSquareColor").get<std::string>(),
					.startAreaColor = board.at("startAreaColor").get<std::string>(),
					.borderColor = board.at("borderColor").get<std::string>(),
					.downBlockColor = board.at("downBlockColor").get<std::string>(),
				},
				.appearance = {
					.labelTextColor = window.at("label").at("textColor").get<std::string>(),
					.buttonFocusColor = button.at("focusColor").get<std::string>(),
					.buttonTextColor = button.at("textColor").get<std::string>(),
					.buttonHoverColor = button.at("hoverColor").get<std::string>(),
					.buttonBackgroundColor = button.at("backgroundColor").get<std::string>(),
					.buttonBorderColor = button.at("borderColor").get<std::string>(),
				},
				.assets = {
					.windowFont = window.at("font").get<std::string>(),
					.boardFont = board.at("font").get<std::string>(),
					.icon = window.at("icon").get<std::string>(),
					.background = window.at("sprites").at("background").get<std::string>(),
				},
				.network = document.at("network").get<Configuration::Network>(),
				.activeAiNames = {
					document.at("ai1").get<std::string>(),
					document.at("ai2").get<std::string>(),
					document.at("ai3").get<std::string>(),
					document.at("ai4").get<std::string>(),
				},
			};
			for (int index = 0; index < std::ssize(BlockSpriteKeys); ++index) {
				settings.assets.blockSprites[index] = board.at("sprites").at(BlockSpriteKeys[index]).get<std::string>();
			}
			for (const auto& definition : document.at("ais")) {
				settings.ais.push_back({
					.name = definition.at("name").get<std::string>(),
					.valueFunction = definition.at("valueFunction").get<std::string>(),
				});
			}
			return settings;
		}

		void writeEditableSettings(json& document, const Configuration::EditableSettings& settings) {
			auto& window = document["window"];
			auto& board = window["tetrisBoard"];
			window["width"] = settings.window.width;
			window["height"] = settings.window.height;
			window["minWidth"] = settings.window.minWidth;
			window["minHeight"] = settings.window.minHeight;
			window["resizeable"] = settings.window.resizable;
			window["border"] = settings.window.bordered;
			window["maximized"] = settings.window.maximized;
			window["pauseOnLostFocus"] = settings.window.pauseOnLostFocus;
			window["fullscreenOnDoubleClick"] = settings.window.fullscreenOnDoubleClick;
			window["moveWindowByHoldingDownMouse"] = settings.window.moveWindowByHoldingDownMouse;
			window["vsync"] = settings.window.vsync;
			window["limitFps"] = settings.window.limitFps;
			board["borderSize"] = settings.board.borderSize;
			board["showDownBlock"] = settings.board.showDownBlock;
			board["outerSquareColor"] = settings.board.outerSquareColor;
			board["innerSquareColor"] = settings.board.innerSquareColor;
			board["startAreaColor"] = settings.board.startAreaColor;
			board["borderColor"] = settings.board.borderColor;
			board["downBlockColor"] = settings.board.downBlockColor;
			window["label"]["textColor"] = settings.appearance.labelTextColor;
			auto& button = window["button"];
			button["focusColor"] = settings.appearance.buttonFocusColor;
			button["textColor"] = settings.appearance.buttonTextColor;
			button["hoverColor"] = settings.appearance.buttonHoverColor;
			button["backgroundColor"] = settings.appearance.buttonBackgroundColor;
			button["borderColor"] = settings.appearance.buttonBorderColor;
			window["font"] = settings.assets.windowFont;
			board["font"] = settings.assets.boardFont;
			window["icon"] = settings.assets.icon;
			window["sprites"]["background"] = settings.assets.background;
			for (int index = 0; index < std::ssize(BlockSpriteKeys); ++index) {
				board["sprites"][BlockSpriteKeys[index]] = settings.assets.blockSprites[index];
			}
			document["network"] = {
				{"globalServer", {{"ip", settings.network.global.ip}, {"port", settings.network.global.port}}},
				{"server", {{"ip", settings.network.server.ip}, {"port", settings.network.server.port}}},
			};
			for (int index = 0; index < std::ssize(settings.activeAiNames); ++index) {
				document["ai" + std::to_string(index + 1)] = settings.activeAiNames[index];
			}
			document["ais"] = json::array();
			for (const auto& definition : settings.ais) {
				document["ais"].push_back({
					{"name", definition.name},
					{"valueFunction", definition.valueFunction},
				});
			}
		}

		bool isValidIpv4(std::string_view address) {
			int parts = 0;
			while (!address.empty()) {
				auto separator = address.find('.');
				auto part = address.substr(0, separator);
				int value = -1;
				auto [end, error] = std::from_chars(part.data(), part.data() + part.size(), value);
				if (part.empty() || error != std::errc{} || end != part.data() + part.size() || value < 0 || value > 255) {
					return false;
				}
				++parts;
				if (separator == std::string_view::npos) {
					break;
				}
				address.remove_prefix(separator + 1);
			}
			return parts == 4;
		}

		std::string truncateUtf8(std::string_view value, int maximumLength) {
			int length = 0;
			for (auto position = value.begin(); position != value.end(); ++position) {
				if ((static_cast<unsigned char>(*position) & 0xc0) != 0x80) {
					if (length == maximumLength) {
						return std::string{value.begin(), position};
					}
					++length;
				}
			}
			return std::string{value};
		}

		bool isBuiltInProfileId(const ProfileId& profileId) {
			return profileId == ControlProfiles::BalancedProfileId
				|| profileId == ControlProfiles::FastProfileId
				|| profileId == ControlProfiles::ClassicProfileId;
		}

		bool containsProfileId(const ControlProfilesState& state, const ProfileId& profileId) {
			return std::ranges::find(state.profiles, profileId, &InputProfile::id) != state.profiles.end();
		}

		std::string trimProfileName(std::string name) {
			const auto first = name.find_first_not_of(ProfileNameWhitespace);
			if (first == std::string::npos) {
				return {};
			}
			const auto last = name.find_last_not_of(ProfileNameWhitespace);
			return name.substr(first, last - first + 1);
		}

		std::string makeUniqueProfileName(std::string name, const ControlProfilesState& state) {
			name = trimProfileName(std::move(name));
			if (name.empty()) {
				name = "Custom Profile";
			}
			name = truncateUtf8(name, ControlProfiles::MaxProfileNameLength);

			auto nameExists = [&state](std::string_view candidate) {
				return std::ranges::find(state.profiles, candidate, &InputProfile::name) != state.profiles.end();
			};
			if (!nameExists(name)) {
				return name;
			}

			for (int copyNumber = 1; copyNumber < std::numeric_limits<int>::max(); ++copyNumber) {
				auto suffix = copyNumber == 1
					? std::string{" Copy"}
					: " Copy " + std::to_string(copyNumber);
				auto candidate = truncateUtf8(
					name,
					ControlProfiles::MaxProfileNameLength - static_cast<int>(std::ssize(suffix))) + suffix;
				if (!nameExists(candidate)) {
					return candidate;
				}
			}
			return name;
		}

		ProfileId makeUniqueProfileId(
			std::string preferredId,
			const ControlProfilesState& state,
			int& migratedId) {
			ProfileId profileId{std::move(preferredId)};
			if (!profileId.value.empty() && !containsProfileId(state, profileId)) {
				return profileId;
			}

			do {
				profileId.value = "migrated-" + std::to_string(migratedId++);
			} while (containsProfileId(state, profileId));
			return profileId;
		}

		std::optional<double> getNumber(const json& object, std::string_view key) {
			auto value = object.find(key);
			if (value == object.end() || !value->is_number()) {
				return std::nullopt;
			}
			auto number = value->get<double>();
			if (!std::isfinite(number)) {
				return std::nullopt;
			}
			return number;
		}

		int convertTiming(double value, double scale, int minimum, int maximum) {
			if (value <= static_cast<double>(minimum) / scale) {
				return minimum;
			}
			if (value >= static_cast<double>(maximum) / scale) {
				return maximum;
			}
			return std::clamp(static_cast<int>(std::llround(value * scale)), minimum, maximum);
		}

		std::optional<InputTiming> getTiming(const json& object, double scale) {
			auto das = getNumber(object, "das");
			auto arr = getNumber(object, "arr");
			if (!das || !arr) {
				return std::nullopt;
			}
			return InputTiming{
				.dasMs = convertTiming(*das, scale, ControlProfiles::MinDasMs, ControlProfiles::MaxDasMs),
				.arrMs = convertTiming(*arr, scale, ControlProfiles::MinArrMs, ControlProfiles::MaxArrMs),
			};
		}

		std::string normalizeDeviceGuid(std::string guid) {
			if (guid == "keyboard1") {
				return "Keyboard 1";
			}
			if (guid == "keyboard2") {
				return "Keyboard 2";
			}
			return guid;
		}

		std::optional<ProfileId> getBuiltInProfileId(InputTiming timing) {
			if (timing == InputTiming{167, 33}) {
				return ControlProfiles::BalancedProfileId;
			}
			if (timing == InputTiming{100, 16}) {
				return ControlProfiles::FastProfileId;
			}
			if (timing == InputTiming{250, 50}) {
				return ControlProfiles::ClassicProfileId;
			}
			return std::nullopt;
		}

		json serializeInputProfiles(const ControlProfilesState& state) {
			auto profiles = json::array();
			for (const auto& profile : state.profiles) {
				if (profile.builtIn || isBuiltInProfileId(profile.id)) {
					continue;
				}
				profiles.push_back({
					{"id", profile.id.value},
					{"name", profile.name},
					{"das", profile.timing.dasMs},
					{"arr", profile.timing.arrMs},
				});
			}
			return profiles;
		}

		json serializeDeviceAssignments(const ControlProfilesState& state) {
			auto devices = json::array();
			for (const auto& assignment : state.assignments) {
				devices.push_back({
					{"guid", assignment.deviceGuid},
					{"inputProfileId", assignment.profileId.value},
				});
			}
			return devices;
		}

		ControlProfilesPersistenceResult replaceJsonFile(const std::string& path, const json& jsonObject) {
			std::string temporaryPath;
			std::ofstream stream;
			for (int attempt = 0; attempt < 10 && !stream.is_open(); ++attempt) {
				temporaryPath = path + "." + util::generateUuid() + ".tmp";
				stream.open(temporaryPath, std::ios::binary | std::ios::out | std::ios::noreplace);
				stream.clear();
			}
			if (!stream.is_open()) {
				return std::unexpected{"Could not create a temporary configuration file beside: " + path};
			}

			auto errorWithCleanup = [&temporaryPath](std::string message) -> ControlProfilesPersistenceResult {
				if (!SDL_RemovePath(temporaryPath.c_str())) {
					message += "; could not remove temporary file: ";
					message += SDL_GetError();
				}
				return std::unexpected{std::move(message)};
			};

			try {
				stream << jsonObject.dump(1);
				stream.flush();
				if (!stream) {
					stream.close();
					return errorWithCleanup("Could not write temporary configuration file: " + temporaryPath);
				}
				stream.close();
				if (stream.fail()) {
					return errorWithCleanup("Could not close temporary configuration file: " + temporaryPath);
				}
			} catch (const std::exception& exception) {
				stream.close();
				return errorWithCleanup(exception.what());
			}

			if (!SDL_RenamePath(temporaryPath.c_str(), path.c_str())) {
				auto message = std::string{"Could not replace configuration file: "} + SDL_GetError();
				return errorWithCleanup(std::move(message));
			}
			return {};
		}

		SDL_Surface* createSurface(int w, int h, sdl::Color color) {
			auto s = SDL_CreateSurface(w, h, SDL_PIXELFORMAT_RGBA32);
			SDL_FillSurfaceRect(s, nullptr, color.toImU32());
			return s;
		}

		app::TextureView addImageToSurface(sdl::ImageAtlas& atlas, SDL_Surface* surfaceAtlas, const std::string& filename, int border = 0) {
			auto rect = sdl::addImage(atlas, surfaceAtlas, filename, border);
			return app::TextureView{
				.pos = {static_cast<float>(rect.x) / surfaceAtlas->w, static_cast<float>(rect.y) / surfaceAtlas->h},
				.size = {static_cast<float>(rect.w) / surfaceAtlas->w, static_cast<float>(rect.h) / surfaceAtlas->h},
			};
		}

	}

	void Configuration::init(SDL_GPUDevice* gpuDevice) {
		std::vector<std::string> files;
		sdl::ImageAtlas atlas{2048, 2048};
		auto surfaceAtlas = sdl::createSdlSurface(createSurface(atlas.getWidth(), atlas.getHeight(), sdl::color::White));

		blockTypeI_ = addImageToSurface(atlas, surfaceAtlas.get(), impl_->jsonObject["window"]["tetrisBoard"]["sprites"]["squareI"].get<std::string>());
		blockTypeJ_ = addImageToSurface(atlas, surfaceAtlas.get(), impl_->jsonObject["window"]["tetrisBoard"]["sprites"]["squareJ"].get<std::string>());
		blockTypeL_ = addImageToSurface(atlas, surfaceAtlas.get(), impl_->jsonObject["window"]["tetrisBoard"]["sprites"]["squareL"].get<std::string>());
		blockTypeO_ = addImageToSurface(atlas, surfaceAtlas.get(), impl_->jsonObject["window"]["tetrisBoard"]["sprites"]["squareO"].get<std::string>());
		blockTypeS_ = addImageToSurface(atlas, surfaceAtlas.get(), impl_->jsonObject["window"]["tetrisBoard"]["sprites"]["squareS"].get<std::string>());
		blockTypeT_ = addImageToSurface(atlas, surfaceAtlas.get(), impl_->jsonObject["window"]["tetrisBoard"]["sprites"]["squareT"].get<std::string>());
		blockTypeZ_ = addImageToSurface(atlas, surfaceAtlas.get(), impl_->jsonObject["window"]["tetrisBoard"]["sprites"]["squareZ"].get<std::string>());
		background_ = addImageToSurface(atlas, surfaceAtlas.get(), impl_->jsonObject["window"]["sprites"]["background"].get<std::string>());
		
		atlasTexture_ = sdl::uploadSurface(gpuDevice, surfaceAtlas.get());
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

	void Configuration::initImGuiFonts() const {
		if (impl_->defaultFont) {
			return;
		}

		auto regularFont = impl_->jsonObject["window"]["font"].get<std::string>();
		auto boardRegularFont = impl_->jsonObject["window"]["tetrisBoard"]["font"].get<std::string>();
		constexpr auto mediumFont = "fonts/inter/Inter_18pt-Medium.ttf";
		constexpr auto semiBoldFont = "fonts/inter/Inter_18pt-SemiBold.ttf";
		auto& io = ImGui::GetIO();

		// Add the primary font first and select it explicitly so unscoped ImGui
		// content and newly-created windows consistently use Inter Regular.
		impl_->defaultFont = io.Fonts->AddFontFromFileTTF(regularFont.c_str(), 17.f);
		io.FontDefault = impl_->defaultFont;

		static const ImWchar iconRanges[] = {ICON_MIN_FA, ICON_MAX_16_FA, 0};
		ImFontConfig iconConfig;
		iconConfig.MergeMode = true;
		iconConfig.PixelSnapH = true;
		iconConfig.GlyphMinAdvanceX = 15.f;
		auto iconFile = fmt::format("fonts/{}", FONT_ICON_FILE_NAME_FAS);
		io.Fonts->AddFontFromFileTTF(iconFile.c_str(), 15.f, &iconConfig, iconRanges);

		impl_->regularSmallFont = io.Fonts->AddFontFromFileTTF(regularFont.c_str(), 14.f);
		impl_->boardRegularFont = boardRegularFont == regularFont
			? impl_->regularSmallFont
			: io.Fonts->AddFontFromFileTTF(boardRegularFont.c_str(), 14.f);
		impl_->mediumFont = io.Fonts->AddFontFromFileTTF(mediumFont, 15.f);
		impl_->mediumValueFont = io.Fonts->AddFontFromFileTTF(mediumFont, 17.f);
		impl_->semiBoldFont = io.Fonts->AddFontFromFileTTF(semiBoldFont, 15.f);
		impl_->scoreFont = io.Fonts->AddFontFromFileTTF(semiBoldFont, 24.f);
	}

	ImFont* Configuration::getImGuiDefaultFont() const {
		initImGuiFonts();
		return impl_->defaultFont;
	}

	ImFont* Configuration::getImGuiRegularSmallFont() const {
		initImGuiFonts();
		return impl_->regularSmallFont;
	}

	ImFont* Configuration::getImGuiBoardRegularFont() const {
		initImGuiFonts();
		return impl_->boardRegularFont;
	}

	ImFont* Configuration::getImGuiMediumFont() const {
		initImGuiFonts();
		return impl_->mediumFont;
	}

	ImFont* Configuration::getImGuiMediumValueFont() const {
		initImGuiFonts();
		return impl_->mediumValueFont;
	}

	ImFont* Configuration::getImGuiSemiBoldFont() const {
		initImGuiFonts();
		return impl_->semiBoldFont;
	}

	ImFont* Configuration::getImGuiScoreFont() const {
		initImGuiFonts();
		return impl_->scoreFont;
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

	Configuration::EditableSettings Configuration::getEditableSettings() const {
		return editableSettingsFromJson(impl_->jsonObject);
	}

	std::expected<Configuration::EditableSettings, std::string> Configuration::getDefaultEditableSettings() const {
		try {
			std::ifstream stream{impl_->defaultsPath};
			if (!stream.is_open()) {
				stream.open("tetris.json");
			}
			if (!stream.is_open()) {
				return std::unexpected{"Could not open tetris.defaults.json"};
			}
			json document;
			stream >> document;
			return editableSettingsFromJson(document);
		} catch (const std::exception& exception) {
			return std::unexpected{exception.what()};
		}
	}

	Configuration::SettingsValidationResult Configuration::validateEditableSettings(const EditableSettings& candidate) const {
		auto invalidRange = [](int value, int minimum, int maximum) {
			return value < minimum || value > maximum;
		};
		if (invalidRange(candidate.window.width, 320, 7680) || invalidRange(candidate.window.height, 320, 4320)) {
			return std::unexpected{"Window size must be between 320 x 320 and 7680 x 4320."};
		}
		if (invalidRange(candidate.window.minWidth, 200, 7680) || invalidRange(candidate.window.minHeight, 200, 4320)) {
			return std::unexpected{"Minimum window size must be between 200 x 200 and 7680 x 4320."};
		}
		if (candidate.window.minWidth > candidate.window.width || candidate.window.minHeight > candidate.window.height) {
			return std::unexpected{"Minimum window size cannot exceed the window size."};
		}
		if (!std::isfinite(candidate.board.borderSize) || candidate.board.borderSize < 0.f || candidate.board.borderSize > 20.f) {
			return std::unexpected{"Board border size must be between 0 and 20."};
		}
		if (!isValidIpv4(candidate.network.global.ip) || !isValidIpv4(candidate.network.server.ip)) {
			return std::unexpected{"Network addresses must be valid IPv4 addresses."};
		}
		if (invalidRange(candidate.network.global.port, 1, 65535) || invalidRange(candidate.network.server.port, 1, 65535)) {
			return std::unexpected{"Network ports must be between 1 and 65535."};
		}

		const std::array colors{
			candidate.board.outerSquareColor, candidate.board.innerSquareColor, candidate.board.startAreaColor,
			candidate.board.borderColor, candidate.board.downBlockColor,
			candidate.appearance.labelTextColor, candidate.appearance.buttonFocusColor,
			candidate.appearance.buttonTextColor, candidate.appearance.buttonHoverColor,
			candidate.appearance.buttonBackgroundColor,
			candidate.appearance.buttonBorderColor,
		};
		if (std::ranges::any_of(colors, [](const std::string& color) { return !sdl::color::isValidHexColor(color); })) {
			return std::unexpected{"Colors must use #RGB, #RGBA, #RRGGBB, or #RRGGBBAA syntax."};
		}

		const std::array paths{
			candidate.assets.windowFont, candidate.assets.boardFont, candidate.assets.icon, candidate.assets.background,
			candidate.assets.blockSprites[0], candidate.assets.blockSprites[1], candidate.assets.blockSprites[2],
			candidate.assets.blockSprites[3], candidate.assets.blockSprites[4], candidate.assets.blockSprites[5],
			candidate.assets.blockSprites[6],
		};
		if (std::ranges::any_of(paths, [](const std::string& path) { return path.empty() || path.contains('\0'); })) {
			return std::unexpected{"Asset paths cannot be empty."};
		}

		std::unordered_set<std::string> aiNames{"Default"};
		for (const auto& definition : candidate.ais) {
			if (definition.name.empty()) {
				return std::unexpected{"AI names cannot be empty."};
			}
			if (!aiNames.insert(definition.name).second) {
				return std::unexpected{"AI names must be unique and 'Default' is reserved."};
			}
			if (definition.valueFunction.empty()) {
				return std::unexpected{"AI value functions cannot be empty."};
			}
			try {
				tetris::Ai{definition.valueFunction, true};
			} catch (const std::exception& exception) {
				return std::unexpected{"Invalid AI '" + definition.name + "': " + exception.what()};
			}
		}
		for (const auto& activeName : candidate.activeAiNames) {
			if (!aiNames.contains(activeName)) {
				return std::unexpected{"Every active AI must reference Default or a custom AI."};
			}
		}
		return {};
	}

	Configuration::SettingsPersistenceResult Configuration::persistEditableSettings(const EditableSettings& candidate) {
		try {
			if (auto validation = validateEditableSettings(candidate); !validation) {
				return std::unexpected{validation.error()};
			}
			auto previous = getEditableSettings();
			if (!impl_->startupSettings) {
				impl_->startupSettings = previous;
			}
			auto document = impl_->jsonObject;
			writeEditableSettings(document, candidate);
			if (auto replacement = replaceJsonFile(impl_->jsonPath, document); !replacement) {
				return std::unexpected{replacement.error()};
			}
			impl_->jsonObject = std::move(document);

			SettingsApplyResult result;
			const auto& runningSettings = *impl_->startupSettings;
			if (runningSettings.network != candidate.network) {
				result.restartReasons.emplace_back("Network endpoints");
			}
			if (runningSettings.assets != candidate.assets) {
				result.restartReasons.emplace_back("Fonts and image assets");
			}
			if (runningSettings.window.vsync != candidate.window.vsync) {
				result.restartReasons.emplace_back("VSync");
			}
			result.restartRequired = !result.restartReasons.empty();
			settingsChanged(previous, candidate);
			return result;
		} catch (const std::exception& exception) {
			return std::unexpected{exception.what()};
		}
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

	ControlProfilesState Configuration::normalizeControlProfiles(json& jsonObject) {
		if (!jsonObject.is_object()) {
			jsonObject = json::object();
		}
		auto state = ControlProfiles::createDefaultState();
		int migratedId = 1;

		auto inputProfiles = jsonObject.find("inputProfiles");
		if (inputProfiles != jsonObject.end() && inputProfiles->is_array()) {
			for (const auto& value : *inputProfiles) {
				if (!value.is_object()) {
					continue;
				}
				auto nameValue = value.find("name");
				auto timing = getTiming(value, 1.0);
				if (nameValue == value.end() || !nameValue->is_string() || !timing) {
					continue;
				}

				auto idValue = value.find("id");
				auto id = idValue != value.end() && idValue->is_string()
					? idValue->get<std::string>()
					: std::string{};
				state.profiles.push_back({
					.id = makeUniqueProfileId(std::move(id), state, migratedId),
					.name = makeUniqueProfileName(nameValue->get<std::string>(), state),
					.timing = *timing,
					.builtIn = false,
				});
			}
		}

		std::vector<std::pair<InputTiming, ProfileId>> migratedTimings;
		std::vector<std::string> deviceGuids;
		auto devices = jsonObject.find("devices");
		if (devices != jsonObject.end() && devices->is_array()) {
			for (const auto& value : *devices) {
				if (!value.is_object()) {
					continue;
				}
				auto guidValue = value.find("guid");
				if (guidValue == value.end() || !guidValue->is_string()) {
					continue;
				}
				auto guid = normalizeDeviceGuid(guidValue->get<std::string>());
				if (guid.empty() || std::ranges::find(deviceGuids, guid) != deviceGuids.end()) {
					continue;
				}
				deviceGuids.push_back(guid);

				ProfileId profileId = ControlProfiles::BalancedProfileId;
				auto profileIdValue = value.find("inputProfileId");
				if (profileIdValue != value.end() && profileIdValue->is_string()) {
					ProfileId requestedId{profileIdValue->get<std::string>()};
					if (containsProfileId(state, requestedId)) {
						profileId = std::move(requestedId);
					}
				} else if (auto timing = getTiming(value, 1000.0)) {
					if (auto builtInId = getBuiltInProfileId(*timing)) {
						profileId = *builtInId;
					} else {
						auto migrated = std::ranges::find(migratedTimings, *timing, &std::pair<InputTiming, ProfileId>::first);
						if (migrated != migratedTimings.end()) {
							profileId = migrated->second;
						} else {
							profileId = makeUniqueProfileId({}, state, migratedId);
							auto name = makeUniqueProfileName(
								fmt::format("Legacy {}/{} ms", timing->dasMs, timing->arrMs),
								state);
							state.profiles.push_back({profileId, std::move(name), *timing, false});
							migratedTimings.push_back({*timing, profileId});
						}
					}
				}

				state.assignments.push_back({std::move(guid), std::move(profileId)});
			}
		}

		auto normalizedProfiles = serializeInputProfiles(state);
		auto normalizedDevices = serializeDeviceAssignments(state);
		jsonObject["inputProfiles"] = std::move(normalizedProfiles);
		jsonObject["devices"] = std::move(normalizedDevices);
		return state;
	}

	ControlProfilesLoadResult Configuration::loadControlProfiles() {
		try {
			auto candidate = impl_->jsonObject;
			auto state = normalizeControlProfiles(candidate);
			if (candidate != impl_->jsonObject) {
				auto result = replaceJsonFile(impl_->jsonPath, candidate);
				if (!result) {
					return std::unexpected{result.error()};
				}
				impl_->jsonObject = std::move(candidate);
			}
			return state;
		} catch (const std::exception& exception) {
			return std::unexpected{exception.what()};
		}
	}

	ControlProfilesPersistenceResult Configuration::persistControlProfiles(const ControlProfilesState& state) {
		try {
			auto candidate = impl_->jsonObject;
			candidate["inputProfiles"] = serializeInputProfiles(state);
			candidate["devices"] = serializeDeviceAssignments(state);
			auto result = replaceJsonFile(impl_->jsonPath, candidate);
			if (!result) {
				return result;
			}
			impl_->jsonObject = std::move(candidate);
			return {};
		} catch (const std::exception& exception) {
			return std::unexpected{exception.what()};
		}
	}

}
