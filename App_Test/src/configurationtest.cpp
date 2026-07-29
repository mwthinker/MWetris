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
			return {
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

		std::filesystem::current_path(std::filesystem::temp_directory_path());
		std::filesystem::remove_all(testDirectory);
	}

}
