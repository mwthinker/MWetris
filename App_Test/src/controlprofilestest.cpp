#include <gtest/gtest.h>

#include <app/controlprofiles.h>

#include <array>
#include <stdexcept>
#include <string>
#include <vector>

namespace app {

	TEST(ControlProfilesTest, suppliesBuiltInCatalog) {
		ControlProfiles profiles;

		EXPECT_EQ(profiles.getProfiles(), (std::vector<InputProfile>{
			{ControlProfiles::BalancedProfileId, "Balanced", {167, 33}, true},
			{ControlProfiles::FastProfileId, "Fast", {100, 16}, true},
			{ControlProfiles::ClassicProfileId, "Classic", {250, 50}, true},
		}));
	}

	TEST(ControlProfilesTest, validatesAndNormalizesProfiles) {
		int saveCount = 0;
		ControlProfiles profiles{[&](const ControlProfilesState&) -> ControlProfilesPersistenceResult {
			++saveCount;
			return {};
		}};

		struct InvalidProfile {
			std::string name;
			InputTiming timing;
			ControlProfilesErrorCode error;
		};
		const std::array invalidProfiles{
			InvalidProfile{" \t\r\n", {167, 33}, ControlProfilesErrorCode::EmptyName},
			InvalidProfile{"\f\v", {167, 33}, ControlProfilesErrorCode::EmptyName},
			InvalidProfile{"Balanced", {167, 33}, ControlProfilesErrorCode::DuplicateName},
			InvalidProfile{std::string(30, 'a'), {167, 33}, ControlProfilesErrorCode::NameTooLong},
			InvalidProfile{"Zero DAS", {0, 33}, ControlProfilesErrorCode::DasOutOfRange},
			InvalidProfile{"Large DAS", {1001, 33}, ControlProfilesErrorCode::DasOutOfRange},
			InvalidProfile{"Zero ARR", {167, 0}, ControlProfilesErrorCode::ArrOutOfRange},
			InvalidProfile{"Large ARR", {167, 251}, ControlProfilesErrorCode::ArrOutOfRange},
		};

		for (const auto& invalidProfile : invalidProfiles) {
			auto result = profiles.duplicateProfile(
				"Keyboard 1",
				ControlProfiles::BalancedProfileId,
				invalidProfile.name,
				invalidProfile.timing);
			ASSERT_FALSE(result);
			EXPECT_EQ(result.error().code, invalidProfile.error);
		}
		EXPECT_EQ(saveCount, 0);

		auto result = profiles.duplicateProfile(
			"Keyboard 1",
			ControlProfiles::BalancedProfileId,
			"  Custom  ",
			{ControlProfiles::MinDasMs, ControlProfiles::MinArrMs});
		ASSERT_TRUE(result);
		EXPECT_EQ(profiles.getProfile(*result)->name, "Custom");

		auto editResult = profiles.editProfile(
			*result,
			"\tCustom Renamed\n",
			{ControlProfiles::MaxDasMs, ControlProfiles::MaxArrMs});
		ASSERT_TRUE(editResult);
		const auto edited = profiles.getProfile(*result);
		ASSERT_TRUE(edited);
		EXPECT_EQ(edited->name, "Custom Renamed");
		EXPECT_EQ(edited->timing, (InputTiming{1000, 250}));
		EXPECT_EQ(saveCount, 2);

		const std::string unicodeCharacter{"\xC3\xA9"};
		std::string unicodeName;
		for (int index = 0; index < ControlProfiles::MaxProfileNameLength; ++index) {
			unicodeName += unicodeCharacter;
		}
		auto unicodeResult = profiles.duplicateProfile(
			"Keyboard 2",
			ControlProfiles::BalancedProfileId,
			unicodeName,
			{167, 33});
		EXPECT_TRUE(unicodeResult);
		EXPECT_EQ(saveCount, 3);
	}

	TEST(ControlProfilesTest, rejectsBuiltInChangesAndTrimmedDuplicateNames) {
		ControlProfiles profiles;

		auto editBuiltIn = profiles.editProfile(ControlProfiles::FastProfileId, "Faster", {100, 16});
		ASSERT_FALSE(editBuiltIn);
		EXPECT_EQ(editBuiltIn.error().code, ControlProfilesErrorCode::BuiltInProfileCannotBeModified);

		auto deleteBuiltIn = profiles.deleteProfile(ControlProfiles::ClassicProfileId);
		ASSERT_FALSE(deleteBuiltIn);
		EXPECT_EQ(deleteBuiltIn.error().code, ControlProfilesErrorCode::BuiltInProfileCannotBeModified);

		auto customProfile = profiles.duplicateProfile(
			"Keyboard 1",
			ControlProfiles::BalancedProfileId,
			"Custom",
			{167, 33});
		ASSERT_TRUE(customProfile);
		auto duplicateEdit = profiles.editProfile(*customProfile, "  Fast\t", {167, 33});
		ASSERT_FALSE(duplicateEdit);
		EXPECT_EQ(duplicateEdit.error().code, ControlProfilesErrorCode::DuplicateName);
		EXPECT_EQ(profiles.getProfile(*customProfile)->name, "Custom");
	}

	TEST(ControlProfilesTest, generatesUniqueCopyNamesWithinLimit) {
		ControlProfiles profiles;

		auto firstName = profiles.getDuplicateProfileName(ControlProfiles::BalancedProfileId);
		ASSERT_TRUE(firstName);
		EXPECT_EQ(*firstName, "Balanced Copy");

		auto firstCopy = profiles.duplicateProfile(
			"Keyboard 1",
			ControlProfiles::BalancedProfileId,
			*firstName,
			{167, 33});
		ASSERT_TRUE(firstCopy);

		auto secondName = profiles.getDuplicateProfileName(ControlProfiles::BalancedProfileId);
		ASSERT_TRUE(secondName);
		EXPECT_EQ(*secondName, "Balanced Copy 2");

		auto longProfile = profiles.duplicateProfile(
			"Keyboard 1",
			ControlProfiles::BalancedProfileId,
			std::string(ControlProfiles::MaxProfileNameLength, 'a'),
			{167, 33});
		ASSERT_TRUE(longProfile);
		auto longCopyName = profiles.getDuplicateProfileName(*longProfile);
		ASSERT_TRUE(longCopyName);
		EXPECT_EQ(longCopyName->size(), ControlProfiles::MaxProfileNameLength);
		EXPECT_EQ(longCopyName->substr(longCopyName->size() - 5), " Copy");
	}

	TEST(ControlProfilesTest, keepsAssignmentsStableAcrossRenameAndFallsBackOnDelete) {
		int saveCount = 0;
		ControlProfiles profiles{[&](const ControlProfilesState&) -> ControlProfilesPersistenceResult {
			++saveCount;
			return {};
		}};

		auto customProfile = profiles.duplicateProfile(
			"Keyboard 1",
			ControlProfiles::BalancedProfileId,
			"Custom",
			{140, 25});
		ASSERT_TRUE(customProfile);
		ASSERT_TRUE(profiles.assignProfile("Keyboard 2", *customProfile));

		ASSERT_TRUE(profiles.editProfile(*customProfile, "Renamed", {140, 25}));
		EXPECT_EQ(profiles.getAssignedProfileId("Keyboard 1"), *customProfile);
		EXPECT_EQ(profiles.getAssignedProfileId("Keyboard 2"), *customProfile);

		auto deleteResult = profiles.deleteProfile(*customProfile);
		ASSERT_TRUE(deleteResult);
		EXPECT_EQ(*deleteResult, 2);
		EXPECT_FALSE(profiles.getProfile(*customProfile));
		EXPECT_EQ(profiles.getAssignedProfileId("Keyboard 1"), ControlProfiles::BalancedProfileId);
		EXPECT_EQ(profiles.getAssignedProfileId("Keyboard 2"), ControlProfiles::BalancedProfileId);
		EXPECT_EQ(saveCount, 4);
	}

	TEST(ControlProfilesTest, savesOncePerCommittedOperationAndNeverForDraftsOrNoOps) {
		int saveCount = 0;
		ControlProfiles profiles{[&](const ControlProfilesState&) -> ControlProfilesPersistenceResult {
			++saveCount;
			return {};
		}};

		auto draft = *profiles.getProfile(ControlProfiles::BalancedProfileId);
		draft.name = "Canceled draft";
		draft.timing = {300, 60};
		EXPECT_TRUE(profiles.getDuplicateProfileName(ControlProfiles::BalancedProfileId));
		EXPECT_EQ(saveCount, 0);

		ASSERT_TRUE(profiles.assignProfile("Keyboard 1", ControlProfiles::FastProfileId));
		EXPECT_EQ(saveCount, 1);
		ASSERT_TRUE(profiles.assignProfile("Keyboard 1", ControlProfiles::FastProfileId));
		EXPECT_EQ(saveCount, 1);

		auto customProfile = profiles.duplicateProfile(
			"Keyboard 1",
			ControlProfiles::FastProfileId,
			"Custom",
			{120, 20});
		ASSERT_TRUE(customProfile);
		EXPECT_EQ(saveCount, 2);

		ASSERT_TRUE(profiles.editProfile(*customProfile, "Custom", {120, 20}));
		EXPECT_EQ(saveCount, 2);
		ASSERT_TRUE(profiles.editProfile(*customProfile, "Edited", {130, 21}));
		EXPECT_EQ(saveCount, 3);

		auto invalidEdit = profiles.editProfile(*customProfile, "", {130, 21});
		EXPECT_FALSE(invalidEdit);
		EXPECT_EQ(saveCount, 3);

		ASSERT_TRUE(profiles.deleteProfile(*customProfile));
		EXPECT_EQ(saveCount, 4);
	}

	TEST(ControlProfilesTest, persistsCandidateBeforePublishingState) {
		ControlProfiles* committedProfiles = nullptr;
		bool candidatePersisted = false;
		ControlProfiles profiles{[&](const ControlProfilesState& candidate) -> ControlProfilesPersistenceResult {
			EXPECT_EQ(committedProfiles->getAssignedProfileId("Keyboard 1"), ControlProfiles::BalancedProfileId);
			auto assignment = candidate.assignments.front();
			EXPECT_EQ(assignment.deviceGuid, "Keyboard 1");
			EXPECT_EQ(assignment.profileId, ControlProfiles::FastProfileId);
			candidatePersisted = true;
			return {};
		}};
		committedProfiles = &profiles;

		auto result = profiles.assignProfile("Keyboard 1", ControlProfiles::FastProfileId);
		EXPECT_TRUE(result);
		EXPECT_TRUE(candidatePersisted);
		EXPECT_EQ(profiles.getAssignedProfileId("Keyboard 1"), ControlProfiles::FastProfileId);
	}

	TEST(ControlProfilesTest, failedPersistenceRollsBackEveryMutation) {
		bool failPersistence = false;
		bool throwPersistence = false;
		int saveCount = 0;
		ControlProfiles profiles{[&](const ControlProfilesState&) -> ControlProfilesPersistenceResult {
			++saveCount;
			if (throwPersistence) {
				throw std::runtime_error{"write exception"};
			}
			if (failPersistence) {
				return std::unexpected{"disk full"};
			}
			return {};
		}};

		auto customProfile = profiles.duplicateProfile(
			"Keyboard 1",
			ControlProfiles::BalancedProfileId,
			"Custom",
			{167, 33});
		ASSERT_TRUE(customProfile);
		failPersistence = true;
		saveCount = 0;

		auto assignResult = profiles.assignProfile("Keyboard 1", ControlProfiles::FastProfileId);
		ASSERT_FALSE(assignResult);
		EXPECT_EQ(assignResult.error().code, ControlProfilesErrorCode::PersistenceFailed);
		EXPECT_NE(assignResult.error().message.find("disk full"), std::string::npos);
		EXPECT_EQ(profiles.getAssignedProfileId("Keyboard 1"), *customProfile);

		auto duplicateResult = profiles.duplicateProfile(
			"Keyboard 2",
			ControlProfiles::BalancedProfileId,
			"Another",
			{100, 16});
		EXPECT_FALSE(duplicateResult);
		EXPECT_EQ(profiles.getProfiles().size(), 4);
		EXPECT_EQ(profiles.getAssignedProfileId("Keyboard 2"), ControlProfiles::BalancedProfileId);

		auto editResult = profiles.editProfile(*customProfile, "Edited", {100, 16});
		EXPECT_FALSE(editResult);
		EXPECT_EQ(profiles.getProfile(*customProfile)->name, "Custom");

		auto deleteResult = profiles.deleteProfile(*customProfile);
		EXPECT_FALSE(deleteResult);
		EXPECT_TRUE(profiles.getProfile(*customProfile));
		EXPECT_EQ(profiles.getAssignedProfileId("Keyboard 1"), *customProfile);

		failPersistence = false;
		throwPersistence = true;
		auto exceptionResult = profiles.assignProfile("Keyboard 1", ControlProfiles::FastProfileId);
		ASSERT_FALSE(exceptionResult);
		EXPECT_EQ(exceptionResult.error().code, ControlProfilesErrorCode::PersistenceFailed);
		EXPECT_NE(exceptionResult.error().message.find("write exception"), std::string::npos);
		EXPECT_EQ(profiles.getAssignedProfileId("Keyboard 1"), *customProfile);

		EXPECT_EQ(saveCount, 5);
	}

}
