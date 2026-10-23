#include <gtest/gtest.h>

#include <app/configuration.h>

#include <nlohmann/json.hpp>

#include <algorithm>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <memory>
#include <string>

namespace app {

	namespace {

		class CurrentPathGuard {
		public:
			CurrentPathGuard()
				: path_{std::filesystem::current_path()} {
			}

			~CurrentPathGuard() {
				std::filesystem::current_path(path_);
			}

		private:
			std::filesystem::path path_;
		};

		nlohmann::json legacyConfiguration() {
			auto document = nlohmann::json{
				{"sentinel", 42},
				{"inputProfiles", {
					{
						{"id", "user-profile"},
						{"name", "User"},
						{"das", 140},
						{"arr", 25},
						{"builtIn", true},
					},
					{
						{"id", "balanced"},
						{"name", "Balanced"},
						{"das", 130},
						{"arr", 20},
					},
				}},
				{"devices", {
					{{"guid", "keyboard1"}, {"das", 0.167}, {"arr", 0.033}},
					{{"guid", "Keyboard 1"}, {"inputProfileId", "fast"}},
					{{"guid", "keyboard2"}, {"das", 0.100}, {"arr", 0.016}},
					{{"guid", "pad"}, {"das", 0.140}, {"arr", 0.025}},
					{{"guid", "pad-2"}, {"das", 0.140}, {"arr", 0.025}},
					{{"guid", "missing"}, {"inputProfileId", "does-not-exist"}},
					{{"guid", "user-device"}, {"inputProfileId", "user-profile"}},
				}},
			};
			document["window"] = {
				{"width", 860}, {"height", 1120}, {"minWidth", 480}, {"minHeight", 480},
				{"resizeable", true}, {"border", true}, {"maximized", false},
				{"pauseOnLostFocus", true}, {"fullscreenOnDoubleClick", false},
				{"moveWindowByHoldingDownMouse", false}, {"vsync", true}, {"limitFps", false},
				{"font", "fonts/ui.ttf"}, {"icon", "images/icon.png"},
				{"label", {{"textColor", "#ffffff"}}},
				{"button", {
					{"focusColor", "#111111"}, {"textColor", "#ffffff"}, {"hoverColor", "#222222"},
					{"pushColor", "#333333"}, {"backgroundColor", "#000000"}, {"borderColor", "#ffffff"},
				}},
				{"sprites", {{"background", "images/background.png"}}},
				{"tetrisBoard", {
					{"font", "fonts/board.ttf"}, {"borderSize", 3.0}, {"showDownBlock", true},
					{"outerSquareColor", "#111111"}, {"innerSquareColor", "#222222"},
					{"startAreaColor", "#333333"}, {"playerAreaColor", "#444444"},
					{"borderColor", "#555555"}, {"downBlockColor", "#ffffff26"},
					{"sprites", {
						{"squareI", "images/I.png"}, {"squareJ", "images/J.png"},
						{"squareL", "images/L.png"}, {"squareO", "images/O.png"},
						{"squareS", "images/S.png"}, {"squareT", "images/T.png"},
						{"squareZ", "images/Z.png"},
					}},
				}},
			};
			document["network"] = {
				{"globalServer", {{"ip", "127.0.0.1"}, {"port", 11155}}},
				{"server", {{"ip", "127.0.0.1"}, {"port", 11175}}},
			};
			document["ai1"] = "Default";
			document["ai2"] = "Default";
			document["ai3"] = "Default";
			document["ai4"] = "Default";
			document["ais"] = nlohmann::json::array({{
				{"name", "Careful"},
				{"valueFunction", "-1*holes"},
			}});
			return document;
		}

		int temporaryConfigurationFileCount() {
			int count = 0;
			for (const auto& entry : std::filesystem::directory_iterator{"."}) {
				auto filename = entry.path().filename().string();
				if (filename.starts_with("tetris.json.") && filename.ends_with(".tmp")) {
					++count;
				}
			}
			return count;
		}

		nlohmann::json readConfigurationFile() {
			nlohmann::json document;
			std::ifstream stream{"tetris.json"};
			stream >> document;
			return document;
		}

	}

	class ConfigurationPersistenceTest : public ::testing::Test {
	protected:
		static void SetUpTestSuite() {
			const auto unique = std::chrono::steady_clock::now().time_since_epoch().count();
			testDirectory_ = std::filesystem::temp_directory_path()
				/ ("mwetris-configuration-test-" + std::to_string(unique));
			std::filesystem::create_directories(testDirectory_);
			currentPathGuard_ = std::make_unique<CurrentPathGuard>();
			std::filesystem::current_path(testDirectory_);

			auto document = legacyConfiguration();
			std::ofstream{"USE_APPLICATION_JSON"} << true;
			std::ofstream{"tetris.json"} << document.dump();
			document["window"]["width"] = 1024;
			std::ofstream{"tetris.defaults.json"} << document.dump();

			configuration_ = &Configuration::getInstance();
			baselineSettings_ = configuration_->getEditableSettings();
			auto profilesDocument = legacyConfiguration();
			baselineProfiles_ = Configuration::normalizeControlProfiles(profilesDocument);
			auto bindingsDocument = legacyConfiguration();
			baselineBindings_ = Configuration::normalizeDeviceBindings(bindingsDocument);
		}

		static void TearDownTestSuite() {
			configuration_->resetRuntimeResources();
			currentPathGuard_.reset();
			std::filesystem::remove_all(testDirectory_);
		}

		void SetUp() override {
			restoreConfigurationFile();
			configuration_->resetRuntimeResources();
			ASSERT_TRUE(configuration_->persistEditableSettings(baselineSettings_));
			ASSERT_TRUE(configuration_->persistControlProfiles(baselineProfiles_));
			ASSERT_TRUE(configuration_->persistDeviceBindings(baselineBindings_));
			configuration_->resetRuntimeResources();
		}

		void TearDown() override {
			restoreConfigurationFile();
			configuration_->resetRuntimeResources();
		}

		void blockConfigurationFileReplacement() {
			std::filesystem::rename("tetris.json", "tetris.json.backup");
			std::filesystem::create_directory("tetris.json");
		}

		void restoreConfigurationFile() {
			if (!std::filesystem::exists("tetris.json.backup")) {
				return;
			}
			std::filesystem::remove_all("tetris.json");
			std::filesystem::rename("tetris.json.backup", "tetris.json");
		}

		static inline std::filesystem::path testDirectory_;
		static inline std::unique_ptr<CurrentPathGuard> currentPathGuard_;
		static inline Configuration* configuration_ = nullptr;
		static inline Configuration::EditableSettings baselineSettings_;
		static inline ControlProfilesState baselineProfiles_;
		static inline std::vector<game::DeviceBinding> baselineBindings_;
	};

	TEST(ConfigurationTest, normalizesLegacyControlProfiles) {
		// Given
		auto document = legacyConfiguration();

		// When
		auto state = Configuration::normalizeControlProfiles(document);

		// Then
		ASSERT_EQ(state.profiles.size(), 6);
		auto userProfile = std::ranges::find(state.profiles, ProfileId{"user-profile"}, &InputProfile::id);
		ASSERT_NE(userProfile, state.profiles.end());
		EXPECT_EQ(userProfile->name, "User");
		EXPECT_EQ(userProfile->timing, (InputTiming{140, 25}));
		EXPECT_FALSE(userProfile->builtIn);
		auto repairedProfile = std::ranges::find(state.profiles, ProfileId{"migrated-1"}, &InputProfile::id);
		ASSERT_NE(repairedProfile, state.profiles.end());
		EXPECT_EQ(repairedProfile->name, "Balanced Copy");
		auto legacyProfile = std::ranges::find(state.profiles, ProfileId{"migrated-2"}, &InputProfile::id);
		ASSERT_NE(legacyProfile, state.profiles.end());
		EXPECT_EQ(legacyProfile->timing, (InputTiming{140, 25}));
	}

	TEST(ConfigurationTest, normalizesLegacyProfileAssignments) {
		// Given
		auto document = legacyConfiguration();

		// When
		auto state = Configuration::normalizeControlProfiles(document);

		// Then
		ASSERT_EQ(state.assignments.size(), 6);
		ControlProfiles profiles{state};
		EXPECT_EQ(profiles.getAssignedProfileId("Keyboard 1"), ControlProfiles::BalancedProfileId);
		EXPECT_EQ(profiles.getAssignedProfileId("Keyboard 2"), ControlProfiles::FastProfileId);
		EXPECT_EQ(std::ranges::count(state.assignments, "keyboard1", &DeviceProfileAssignment::deviceGuid), 0);
		EXPECT_EQ(std::ranges::count(state.assignments, "keyboard2", &DeviceProfileAssignment::deviceGuid), 0);
		EXPECT_EQ(profiles.getAssignedProfileId("pad"), ProfileId{"migrated-2"});
		EXPECT_EQ(profiles.getAssignedProfileId("pad-2"), ProfileId{"migrated-2"});
		EXPECT_EQ(profiles.getAssignedProfileId("missing"), ControlProfiles::BalancedProfileId);
		EXPECT_EQ(profiles.getAssignedProfileId("user-device"), ProfileId{"user-profile"});
	}

	TEST(ConfigurationTest, rewritesLegacyControlProfileJson) {
		// Given
		auto document = legacyConfiguration();

		// When
		Configuration::normalizeControlProfiles(document);

		// Then
		EXPECT_EQ(document.at("sentinel"), 42);
		ASSERT_EQ(document.at("inputProfiles").size(), 3);
		for (const auto& profile : document.at("inputProfiles")) {
			EXPECT_FALSE(profile.contains("builtIn"));
		}
		for (const auto& device : document.at("devices")) {
			EXPECT_TRUE(device.contains("inputProfileId"));
			EXPECT_FALSE(device.contains("das"));
			EXPECT_FALSE(device.contains("arr"));
		}
	}

	TEST(ConfigurationTest, controlProfileNormalizationIsIdempotent) {
		// Given
		auto document = legacyConfiguration();
		auto state = Configuration::normalizeControlProfiles(document);
		auto normalizedDocument = document;

		// When
		auto normalizedState = Configuration::normalizeControlProfiles(normalizedDocument);

		// Then
		EXPECT_EQ(normalizedDocument, document);
		EXPECT_EQ(normalizedState.profiles, state.profiles);
		EXPECT_EQ(normalizedState.assignments, state.assignments);
	}

	TEST(ConfigurationTest, invalidControlProfileRootProducesDefaults) {
		// Given
		nlohmann::json document = 42;

		// When
		auto state = Configuration::normalizeControlProfiles(document);

		// Then
		EXPECT_TRUE(document.is_object());
		EXPECT_EQ(state.profiles, ControlProfiles::createDefaultState().profiles);
		EXPECT_TRUE(state.assignments.empty());
	}

	TEST(ConfigurationTest, normalizesDeviceBindings) {
		// Given
		auto document = nlohmann::json{{"devices", {
			{{"guid", "keyboard1"}, {"inputProfileId", "fast"}},
			{{"guid", "pad"}, {"inputProfileId", "balanced"}, {"bindings", {
				{"type", "gamepad"},
				{"left", SDL_GetGamepadStringForButton(SDL_GAMEPAD_BUTTON_WEST)},
				{"right", SDL_GetGamepadStringForButton(SDL_GAMEPAD_BUTTON_EAST)},
				{"softDrop", SDL_GetGamepadStringForButton(SDL_GAMEPAD_BUTTON_DPAD_DOWN)},
				{"rotate", SDL_GetGamepadStringForButton(SDL_GAMEPAD_BUTTON_NORTH)},
				{"hardDrop", SDL_GetGamepadStringForButton(SDL_GAMEPAD_BUTTON_SOUTH)},
			}}},
			{{"guid", "broken"}, {"inputProfileId", "balanced"}, {"bindings", {
				{"type", "keyboard"},
				{"left", "A"}, {"right", "A"}, {"softDrop", "S"}, {"rotate", "W"}, {"hardDrop", "Left Ctrl"},
			}}},
		}}};

		// When
		auto state = Configuration::normalizeDeviceBindings(document);

		// Then
		ASSERT_EQ(state.size(), 3);
		EXPECT_EQ(state[0].deviceGuid, "Keyboard 1");
		ASSERT_TRUE(std::holds_alternative<game::KeyboardBindings>(state[0].bindings));
		EXPECT_EQ(std::get<game::KeyboardBindings>(state[0].bindings), game::getDefaultKeyboardBindings(1));
		ASSERT_TRUE(std::holds_alternative<game::GamepadBindings>(state[1].bindings));
		EXPECT_TRUE(game::validateBindings(std::get<game::GamepadBindings>(state[1].bindings)));
		EXPECT_EQ(std::get<game::GamepadBindings>(state[1].bindings).left, SDL_GAMEPAD_BUTTON_WEST);
		ASSERT_TRUE(std::holds_alternative<game::KeyboardBindings>(state[2].bindings));
		EXPECT_EQ(std::get<game::KeyboardBindings>(state[2].bindings), game::getDefaultKeyboardBindings(1));
		EXPECT_EQ(document["devices"][0]["inputProfileId"], "fast");
		EXPECT_EQ(document["devices"][0]["bindings"]["type"], "keyboard");
	}

	TEST(ConfigurationTest, deviceBindingNormalizationIsIdempotent) {
		// Given
		auto document = legacyConfiguration();
		auto state = Configuration::normalizeDeviceBindings(document);
		auto normalized = document;

		// When
		auto roundTrip = Configuration::normalizeDeviceBindings(normalized);

		// Then
		EXPECT_EQ(normalized, document);
		EXPECT_EQ(roundTrip, state);
	}

	TEST_F(ConfigurationPersistenceTest, persistsDeviceBindings) {
		// Given
		auto bindings = configuration_->loadDeviceBindings();
		ASSERT_TRUE(bindings);
		auto keyboard = std::ranges::find(*bindings, "Keyboard 1", &game::DeviceBinding::deviceGuid);
		ASSERT_NE(keyboard, bindings->end());
		auto customized = std::get<game::KeyboardBindings>(keyboard->bindings);
		customized.hardDrop = SDLK_SPACE;
		keyboard->bindings = customized;

		// When
		auto result = configuration_->persistDeviceBindings(*bindings);

		// Then
		ASSERT_TRUE(result);
		auto roundTrip = configuration_->loadDeviceBindings();
		ASSERT_TRUE(roundTrip);
		EXPECT_EQ(*roundTrip, *bindings);
		EXPECT_EQ(temporaryConfigurationFileCount(), 0);
		auto persisted = readConfigurationFile();
		auto persistedKeyboard = std::ranges::find(
			persisted.at("devices"), "Keyboard 1", [](const nlohmann::json& value) {
				return value.at("guid").get<std::string>();
			});
		ASSERT_NE(persistedKeyboard, persisted.at("devices").end());
		EXPECT_EQ(persistedKeyboard->at("bindings").at("hardDrop"), "Space");
	}

	TEST_F(ConfigurationPersistenceTest, persistsControlProfileAssignment) {
		// Given
		auto state = configuration_->loadControlProfiles();
		ASSERT_TRUE(state);
		ControlProfiles profiles{*state, [this](const ControlProfilesState& candidate) {
			return configuration_->persistControlProfiles(candidate);
		}};

		// When
		auto result = profiles.assignProfile("user-device", ControlProfiles::ClassicProfileId);

		// Then
		ASSERT_TRUE(result);
		auto persisted = readConfigurationFile();
		auto assignment = std::ranges::find(
			persisted.at("devices"), "user-device", [](const nlohmann::json& value) {
				return value.at("guid").get<std::string>();
			});
		ASSERT_NE(assignment, persisted.at("devices").end());
		EXPECT_EQ(assignment->at("inputProfileId"), "classic");
	}

	TEST_F(ConfigurationPersistenceTest, persistsDuplicatedControlProfile) {
		// Given
		auto state = configuration_->loadControlProfiles();
		ASSERT_TRUE(state);
		ControlProfiles profiles{*state, [this](const ControlProfilesState& candidate) {
			return configuration_->persistControlProfiles(candidate);
		}};

		// When
		auto profileId = profiles.duplicateProfile(
			"round-trip-device", ControlProfiles::BalancedProfileId, "Round Trip", {321, 45});

		// Then
		ASSERT_TRUE(profileId);
		auto roundTrip = configuration_->loadControlProfiles();
		ASSERT_TRUE(roundTrip);
		ControlProfiles roundTrippedProfiles{*roundTrip};
		auto profile = roundTrippedProfiles.getProfile(*profileId);
		ASSERT_TRUE(profile);
		EXPECT_EQ(profile->name, "Round Trip");
		EXPECT_EQ(profile->timing, (InputTiming{321, 45}));
		EXPECT_FALSE(profile->builtIn);
		EXPECT_EQ(roundTrippedProfiles.getAssignedProfileId("round-trip-device"), *profileId);
	}

	TEST_F(ConfigurationPersistenceTest, failedControlProfilePersistenceKeepsCommittedState) {
		// Given
		auto state = configuration_->loadControlProfiles();
		ASSERT_TRUE(state);
		auto userAssignment = std::ranges::find(
			state->assignments, "user-device", &DeviceProfileAssignment::deviceGuid);
		ASSERT_NE(userAssignment, state->assignments.end());
		userAssignment->profileId = ControlProfiles::ClassicProfileId;
		ASSERT_TRUE(configuration_->persistControlProfiles(*state));
		userAssignment->profileId = ControlProfiles::FastProfileId;
		blockConfigurationFileReplacement();

		// When
		auto result = configuration_->persistControlProfiles(*state);

		// Then
		ASSERT_FALSE(result);
		EXPECT_NE(result.error().find("replace configuration file"), std::string::npos);
		EXPECT_EQ(temporaryConfigurationFileCount(), 0);
		restoreConfigurationFile();
		auto persisted = readConfigurationFile();
		auto persistedAssignment = std::ranges::find(
			persisted.at("devices"), "user-device", [](const nlohmann::json& value) {
				return value.at("guid").get<std::string>();
			});
		ASSERT_NE(persistedAssignment, persisted.at("devices").end());
		EXPECT_EQ(persistedAssignment->at("inputProfileId"), "classic");
		auto committed = configuration_->loadControlProfiles();
		ASSERT_TRUE(committed);
		auto committedAssignment = std::ranges::find(
			committed->assignments, "user-device", &DeviceProfileAssignment::deviceGuid);
		ASSERT_NE(committedAssignment, committed->assignments.end());
		EXPECT_EQ(committedAssignment->profileId, ControlProfiles::ClassicProfileId);
	}

	TEST_F(ConfigurationPersistenceTest, persistsEditableSettingsWithoutRestart) {
		// Given
		auto settings = configuration_->getEditableSettings();
		settings.window.width = 900;
		settings.window.uiScale = 1.25f;
		settings.board.showDownBlock = false;
		settings.activeAiNames[2] = "Careful";
		settings.ais.push_back({"Aggressive", "-0.5*holes - landingHeight"});

		// When
		auto result = configuration_->persistEditableSettings(settings);

		// Then
		ASSERT_TRUE(result);
		EXPECT_FALSE(result->restartRequired);
		EXPECT_EQ(configuration_->getEditableSettings(), settings);
		auto persisted = readConfigurationFile();
		EXPECT_EQ(persisted.at("sentinel"), 42);
		EXPECT_EQ(persisted.at("window").at("width"), 900);
		EXPECT_FLOAT_EQ(persisted.at("window").at("uiScale").get<float>(), 1.25f);
		EXPECT_FALSE(persisted.at("window").at("tetrisBoard").at("showDownBlock"));
		EXPECT_EQ(persisted.at("ai3"), "Careful");
		EXPECT_TRUE(persisted.contains("devices"));
		EXPECT_TRUE(persisted.contains("inputProfiles"));
	}

	TEST_F(ConfigurationPersistenceTest, networkChangeRequiresRestart) {
		// Given
		auto settings = configuration_->getEditableSettings();
		settings.network.server.ip = "127.0.0.2";

		// When
		auto result = configuration_->persistEditableSettings(settings);

		// Then
		ASSERT_TRUE(result);
		EXPECT_TRUE(result->restartRequired);
		EXPECT_EQ(result->restartReasons, std::vector<std::string>{"Network endpoints"});
	}

	TEST_F(ConfigurationPersistenceTest, runtimeResetAcceptsCurrentSettingsAsRestartBaseline) {
		// Given
		auto settings = configuration_->getEditableSettings();
		settings.network.server.ip = "127.0.0.2";
		ASSERT_TRUE(configuration_->persistEditableSettings(settings));

		// When
		configuration_->resetRuntimeResources();
		auto result = configuration_->persistEditableSettings(settings);

		// Then
		ASSERT_TRUE(result);
		EXPECT_FALSE(result->restartRequired);
		EXPECT_TRUE(result->restartReasons.empty());
	}

	TEST_F(ConfigurationPersistenceTest, revertingNetworkChangeStillRequiresRestart) {
		// Given
		auto original = configuration_->getEditableSettings();
		auto changed = original;
		changed.network.server.ip = "127.0.0.2";
		ASSERT_TRUE(configuration_->persistEditableSettings(changed));
		configuration_->resetRuntimeResources();
		ASSERT_TRUE(configuration_->persistEditableSettings(changed));

		// When
		auto result = configuration_->persistEditableSettings(original);

		// Then
		ASSERT_TRUE(result);
		EXPECT_TRUE(result->restartRequired);
		EXPECT_EQ(result->restartReasons, std::vector<std::string>{"Network endpoints"});
	}

	TEST_F(ConfigurationPersistenceTest, rejectsInvalidNetworkPortWithoutChangingSettings) {
		// Given
		auto original = configuration_->getEditableSettings();
		auto invalid = original;
		invalid.network.server.port = 70000;

		// When
		auto result = configuration_->persistEditableSettings(invalid);

		// Then
		ASSERT_FALSE(result);
		EXPECT_NE(result.error().find("ports"), std::string::npos);
		EXPECT_EQ(configuration_->getEditableSettings(), original);
	}

	TEST_F(ConfigurationPersistenceTest, rejectsInvalidUiScaleWithoutChangingSettings) {
		// Given
		auto original = configuration_->getEditableSettings();
		auto invalid = original;
		invalid.window.uiScale = 0.25f;

		// When
		auto result = configuration_->persistEditableSettings(invalid);

		// Then
		ASSERT_FALSE(result);
		EXPECT_NE(result.error().find("UI scale"), std::string::npos);
		EXPECT_EQ(configuration_->getEditableSettings(), original);
	}

	TEST_F(ConfigurationPersistenceTest, loadsEditableDefaultsFromDefaultsFile) {
		// Given
		ASSERT_EQ(configuration_->getEditableSettings().window.width, 860);

		// When
		auto defaults = configuration_->getDefaultEditableSettings();

		// Then
		ASSERT_TRUE(defaults);
		EXPECT_EQ(defaults->window.width, 1024);
	}

	TEST_F(ConfigurationPersistenceTest, failedSettingsPersistenceKeepsCurrentSettings) {
		// Given
		auto original = configuration_->getEditableSettings();
		auto candidate = original;
		candidate.window.width = 1000;
		blockConfigurationFileReplacement();

		// When
		auto result = configuration_->persistEditableSettings(candidate);

		// Then
		ASSERT_FALSE(result);
		EXPECT_EQ(configuration_->getEditableSettings(), original);
		EXPECT_EQ(temporaryConfigurationFileCount(), 0);
	}

	TEST(ConfigurationTest, appliesColorScheme) {
		// Given
		Configuration::BoardSettings settings;

		// When
		Configuration::applyColorScheme(settings, "warmSand");

		// Then
		EXPECT_EQ(settings.colorScheme, "warmSand");
		EXPECT_EQ(Configuration::detectColorScheme(settings), "warmSand");
		EXPECT_EQ(settings.playerCardTextColor, "#2A241B");
	}

	TEST(ConfigurationTest, detectsModifiedColorSchemeAsCustom) {
		// Given
		Configuration::BoardSettings settings;
		Configuration::applyColorScheme(settings, "warmSand");
		settings.sidebarBackgroundColor = "#000000";

		// When
		auto scheme = Configuration::detectColorScheme(settings);

		// Then
		EXPECT_EQ(scheme, "custom");
	}

}
