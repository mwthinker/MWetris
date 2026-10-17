#include <gtest/gtest.h>

#include <app/configuration.h>

#include <nlohmann/json.hpp>

#include <algorithm>
#include <chrono>
#include <filesystem>
#include <fstream>
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

	}

	TEST(ConfigurationTest, normalizesControlProfilesFromSuppliedJson) {
		auto document = legacyConfiguration();
		auto state = Configuration::normalizeControlProfiles(document);
		ASSERT_EQ(state.profiles.size(), 6);
		ASSERT_EQ(state.assignments.size(), 6);

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

		ControlProfiles profiles{state};
		EXPECT_EQ(profiles.getAssignedProfileId("Keyboard 1"), ControlProfiles::BalancedProfileId);
		EXPECT_EQ(profiles.getAssignedProfileId("Keyboard 2"), ControlProfiles::FastProfileId);
		EXPECT_EQ(std::ranges::count(state.assignments, "keyboard1", &DeviceProfileAssignment::deviceGuid), 0);
		EXPECT_EQ(std::ranges::count(state.assignments, "keyboard2", &DeviceProfileAssignment::deviceGuid), 0);
		EXPECT_EQ(profiles.getAssignedProfileId("pad"), ProfileId{"migrated-2"});
		EXPECT_EQ(profiles.getAssignedProfileId("pad-2"), ProfileId{"migrated-2"});
		EXPECT_EQ(profiles.getAssignedProfileId("missing"), ControlProfiles::BalancedProfileId);
		EXPECT_EQ(profiles.getAssignedProfileId("user-device"), ProfileId{"user-profile"});

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

		auto normalizedDocument = document;
		auto normalizedState = Configuration::normalizeControlProfiles(normalizedDocument);
		EXPECT_EQ(normalizedDocument, document);
		EXPECT_EQ(normalizedState.profiles, state.profiles);
		EXPECT_EQ(normalizedState.assignments, state.assignments);

		nlohmann::json invalidRoot = 42;
		auto defaultState = Configuration::normalizeControlProfiles(invalidRoot);
		EXPECT_TRUE(invalidRoot.is_object());
		EXPECT_EQ(defaultState.profiles, ControlProfiles::createDefaultState().profiles);
		EXPECT_TRUE(defaultState.assignments.empty());
	}

	TEST(ConfigurationTest, persistsCandidateOnlyAfterSuccessfulReplacement) {
		const auto unique = std::chrono::steady_clock::now().time_since_epoch().count();
		const auto testDirectory = std::filesystem::temp_directory_path()
			/ ("mwetris-configuration-test-" + std::to_string(unique));
		std::filesystem::create_directories(testDirectory);
		CurrentPathGuard currentPathGuard;
		std::filesystem::current_path(testDirectory);

		const auto initialDocument = legacyConfiguration();
		{
			std::ofstream selection{"USE_APPLICATION_JSON"};
			selection << true;
			std::ofstream configuration{"tetris.json"};
			configuration << initialDocument.dump();
			std::ofstream defaults{"tetris.defaults.json"};
			auto defaultDocument = initialDocument;
			defaultDocument["window"]["width"] = 1024;
			defaults << defaultDocument.dump();
		}

		auto& configuration = Configuration::getInstance();
		auto loaded = configuration.loadControlProfiles();
		ASSERT_TRUE(loaded);
		EXPECT_EQ(temporaryConfigurationFileCount(), 0);
		nlohmann::json migrated;
		{
			std::ifstream stream{"tetris.json"};
			stream >> migrated;
		}
		EXPECT_TRUE(migrated.at("devices").at(0).contains("inputProfileId"));
		EXPECT_FALSE(migrated.at("devices").at(0).contains("das"));

		ControlProfiles profiles{*loaded, [&configuration](const ControlProfilesState& candidate) {
			return configuration.persistControlProfiles(candidate);
		}};

		auto assignResult = profiles.assignProfile("user-device", ControlProfiles::ClassicProfileId);
		ASSERT_TRUE(assignResult);
		auto roundTripProfileId = profiles.duplicateProfile(
			"round-trip-device",
			ControlProfiles::BalancedProfileId,
			"Round Trip",
			{321, 45});
		ASSERT_TRUE(roundTripProfileId);
		EXPECT_EQ(temporaryConfigurationFileCount(), 0);

		nlohmann::json persisted;
		{
			std::ifstream stream{"tetris.json"};
			stream >> persisted;
		}
		EXPECT_EQ(persisted.at("sentinel"), 42);
		auto persistedUserAssignment = std::ranges::find(
			persisted.at("devices"),
			"user-device",
			[](const nlohmann::json& assignment) {
				return assignment.at("guid").get<std::string>();
			});
		ASSERT_NE(persistedUserAssignment, persisted.at("devices").end());
		EXPECT_EQ(persistedUserAssignment->at("inputProfileId"), "classic");
		auto persistedRoundTripProfile = std::ranges::find(
			persisted.at("inputProfiles"),
			roundTripProfileId->value,
			[](const nlohmann::json& profile) {
				return profile.at("id").get<std::string>();
			});
		ASSERT_NE(persistedRoundTripProfile, persisted.at("inputProfiles").end());
		EXPECT_EQ(persistedRoundTripProfile->at("name"), "Round Trip");
		EXPECT_EQ(persistedRoundTripProfile->at("das"), 321);
		EXPECT_EQ(persistedRoundTripProfile->at("arr"), 45);
		EXPECT_FALSE(persistedRoundTripProfile->contains("builtIn"));

		auto committed = configuration.loadControlProfiles();
		ASSERT_TRUE(committed);
		ControlProfiles roundTrippedProfiles{*committed};
		auto roundTrippedProfile = roundTrippedProfiles.getProfile(*roundTripProfileId);
		ASSERT_TRUE(roundTrippedProfile);
		EXPECT_EQ(roundTrippedProfile->name, "Round Trip");
		EXPECT_EQ(roundTrippedProfile->timing, (InputTiming{321, 45}));
		EXPECT_FALSE(roundTrippedProfile->builtIn);
		EXPECT_EQ(roundTrippedProfiles.getAssignedProfileId("round-trip-device"), *roundTripProfileId);
		auto failedCandidate = *committed;
		auto userAssignment = std::ranges::find(
			failedCandidate.assignments,
			"user-device",
			&DeviceProfileAssignment::deviceGuid);
		ASSERT_NE(userAssignment, failedCandidate.assignments.end());
		userAssignment->profileId = ControlProfiles::FastProfileId;
		std::filesystem::rename("tetris.json", "tetris.json.backup");
		std::filesystem::create_directory("tetris.json");
		auto failedSave = configuration.persistControlProfiles(failedCandidate);
		ASSERT_FALSE(failedSave);
		EXPECT_NE(failedSave.error().find("replace configuration file"), std::string::npos);
		EXPECT_EQ(temporaryConfigurationFileCount(), 0);
		std::filesystem::remove("tetris.json");
		std::filesystem::rename("tetris.json.backup", "tetris.json");

		{
			std::ifstream stream{"tetris.json"};
			stream >> persisted;
		}
		persistedUserAssignment = std::ranges::find(
			persisted.at("devices"),
			"user-device",
			[](const nlohmann::json& assignment) {
				return assignment.at("guid").get<std::string>();
			});
		ASSERT_NE(persistedUserAssignment, persisted.at("devices").end());
		EXPECT_EQ(persistedUserAssignment->at("inputProfileId"), "classic");
		auto afterFailure = configuration.loadControlProfiles();
		ASSERT_TRUE(afterFailure);
		auto committedUserAssignment = std::ranges::find(
			afterFailure->assignments,
			"user-device",
			&DeviceProfileAssignment::deviceGuid);
		ASSERT_NE(committedUserAssignment, afterFailure->assignments.end());
		EXPECT_EQ(committedUserAssignment->profileId, ControlProfiles::ClassicProfileId);

		auto settings = configuration.getEditableSettings();
		EXPECT_EQ(settings.window.width, 860);
		EXPECT_EQ(settings.ais.size(), 1);
		settings.window.width = 900;
		settings.board.showDownBlock = false;
		settings.activeAiNames[2] = "Careful";
		settings.ais.push_back({"Aggressive", "-0.5*holes - landingHeight"});
		auto settingsSave = configuration.persistEditableSettings(settings);
		ASSERT_TRUE(settingsSave);
		EXPECT_FALSE(settingsSave->restartRequired);

		{
			std::ifstream stream{"tetris.json"};
			stream >> persisted;
		}
		EXPECT_EQ(persisted.at("sentinel"), 42);
		EXPECT_EQ(persisted.at("window").at("width"), 900);
		EXPECT_FALSE(persisted.at("window").at("tetrisBoard").at("showDownBlock"));
		EXPECT_EQ(persisted.at("ai3"), "Careful");
		EXPECT_TRUE(persisted.contains("devices"));
		EXPECT_TRUE(persisted.contains("inputProfiles"));

		auto restartSettings = settings;
		restartSettings.network.server.ip = "127.0.0.2";
		auto restartSave = configuration.persistEditableSettings(restartSettings);
		ASSERT_TRUE(restartSave);
		EXPECT_TRUE(restartSave->restartRequired);
		EXPECT_EQ(restartSave->restartReasons, std::vector<std::string>{"Network endpoints"});
		auto revertRestartSetting = configuration.persistEditableSettings(settings);
		ASSERT_TRUE(revertRestartSetting);
		EXPECT_FALSE(revertRestartSetting->restartRequired);

		auto invalidSettings = settings;
		invalidSettings.network.server.port = 70000;
		auto invalidSave = configuration.persistEditableSettings(invalidSettings);
		ASSERT_FALSE(invalidSave);
		EXPECT_NE(invalidSave.error().find("ports"), std::string::npos);
		EXPECT_EQ(configuration.getEditableSettings(), settings);

		auto defaults = configuration.getDefaultEditableSettings();
		ASSERT_TRUE(defaults);
		EXPECT_EQ(defaults->window.width, 1024);
		EXPECT_EQ(configuration.getEditableSettings().window.width, 900);

		auto failedSettings = settings;
		failedSettings.window.width = 1000;
		std::filesystem::rename("tetris.json", "tetris.json.backup");
		std::filesystem::create_directory("tetris.json");
		auto failedSettingsSave = configuration.persistEditableSettings(failedSettings);
		ASSERT_FALSE(failedSettingsSave);
		EXPECT_EQ(configuration.getEditableSettings(), settings);
		EXPECT_EQ(temporaryConfigurationFileCount(), 0);
		std::filesystem::remove("tetris.json");
		std::filesystem::rename("tetris.json.backup", "tetris.json");

		std::filesystem::current_path(std::filesystem::temp_directory_path());
		std::filesystem::remove_all(testDirectory);
	}

}
