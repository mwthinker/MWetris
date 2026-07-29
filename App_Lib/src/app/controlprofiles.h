#ifndef APP_CONTROLPROFILES_H
#define APP_CONTROLPROFILES_H

#include <mw/signal.h>

#include <cstddef>
#include <expected>
#include <functional>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace app {

	struct ProfileId {
		std::string value;

		friend bool operator==(const ProfileId&, const ProfileId&) = default;
	};

	struct InputTiming {
		int dasMs = 0;
		int arrMs = 0;

		friend bool operator==(const InputTiming&, const InputTiming&) = default;
	};

	struct InputProfile {
		ProfileId id;
		std::string name;
		InputTiming timing;
		bool builtIn = false;

		friend bool operator==(const InputProfile&, const InputProfile&) = default;
	};

	struct DeviceProfileAssignment {
		std::string deviceGuid;
		ProfileId profileId;

		friend bool operator==(const DeviceProfileAssignment&, const DeviceProfileAssignment&) = default;
	};

	struct ControlProfilesChange {
		std::vector<std::string> affectedDeviceGuids;
	};

	struct ControlProfilesState {
		std::vector<InputProfile> profiles;
		std::vector<DeviceProfileAssignment> assignments;
	};

	enum class ControlProfilesErrorCode {
		ProfileNotFound,
		BuiltInProfileCannotBeModified,
		EmptyName,
		DuplicateName,
		NameTooLong,
		DasOutOfRange,
		ArrOutOfRange,
		PersistenceFailed,
	};

	struct ControlProfilesError {
		ControlProfilesErrorCode code;
		std::string message;
	};

	template <typename Type>
	using ControlProfilesResult = std::expected<Type, ControlProfilesError>;
	using ControlProfilesPersistenceResult = std::expected<void, std::string>;
	using ControlProfilesPersistence = std::function<ControlProfilesPersistenceResult(const ControlProfilesState&)>;

	class ControlProfiles {
	public:
		static constexpr std::size_t MaxProfileNameLength = 29;
		static constexpr int MinDasMs = 1;
		static constexpr int MaxDasMs = 1000;
		static constexpr int MinArrMs = 1;
		static constexpr int MaxArrMs = 250;

		static const ProfileId BalancedProfileId;
		static const ProfileId FastProfileId;
		static const ProfileId ClassicProfileId;

		mw::PublicSignal<ControlProfiles, const ControlProfilesChange&> changed;

		explicit ControlProfiles(ControlProfilesPersistence persistence = {});
		ControlProfiles(ControlProfilesState initialState, ControlProfilesPersistence persistence = {});

		static ControlProfilesState createDefaultState();

		const std::vector<InputProfile>& getProfiles() const;
		std::optional<InputProfile> getProfile(const ProfileId& profileId) const;

		const std::vector<DeviceProfileAssignment>& getAssignments() const;
		ProfileId getAssignedProfileId(std::string_view deviceGuid) const;
		InputTiming getAssignedTiming(std::string_view deviceGuid) const;
		std::size_t getAssignedDeviceCount(const ProfileId& profileId) const;
		ControlProfilesResult<std::string> getDuplicateProfileName(const ProfileId& profileId) const;

		ControlProfilesResult<void> assignProfile(
			std::string_view deviceGuid,
			const ProfileId& profileId);

		ControlProfilesResult<ProfileId> duplicateProfile(
			std::string_view deviceGuid,
			const ProfileId& sourceProfileId,
			std::string name,
			InputTiming timing);

		ControlProfilesResult<void> editProfile(
			const ProfileId& profileId,
			std::string name,
			InputTiming timing);

		ControlProfilesResult<std::size_t> deleteProfile(const ProfileId& profileId);

	private:
		ControlProfilesResult<std::string> validateProfile(
			std::string name,
			InputTiming timing,
			const std::optional<ProfileId>& ignoredProfileId = std::nullopt) const;
		ControlProfilesResult<void> commit(ControlProfilesState candidate, const ControlProfilesChange& change);
		bool isBuiltIn(const ProfileId& profileId) const;
		ProfileId generateProfileId(const ControlProfilesState& state) const;

		ControlProfilesState state_;
		ControlProfilesPersistence persistence_;
	};

}

#endif
