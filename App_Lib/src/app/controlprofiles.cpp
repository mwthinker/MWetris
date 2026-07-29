#include "controlprofiles.h"

#include "util/uuid.h"

#include <algorithm>
#include <exception>
#include <limits>
#include <utility>

namespace app {
	namespace {

		std::size_t utf8Length(std::string_view value) {
			return static_cast<std::size_t>(std::ranges::count_if(value, [](char character) {
				return (static_cast<unsigned char>(character) & 0xc0) != 0x80;
			}));
		}

		std::string truncateUtf8(std::string_view value, std::size_t maximumLength) {
			std::size_t length = 0;
			for (std::size_t index = 0; index < value.size(); ++index) {
				if ((static_cast<unsigned char>(value[index]) & 0xc0) != 0x80) {
					if (length == maximumLength) {
						return std::string{value.substr(0, index)};
					}
					++length;
				}
			}
			return std::string{value};
		}

	}

	const ProfileId ControlProfiles::BalancedProfileId{"balanced"};
	const ProfileId ControlProfiles::FastProfileId{"fast"};
	const ProfileId ControlProfiles::ClassicProfileId{"classic"};

	ControlProfiles::ControlProfiles(ControlProfilesPersistence persistence)
		: ControlProfiles{createDefaultState(), std::move(persistence)} {
	}

	ControlProfiles::ControlProfiles(ControlProfilesState initialState, ControlProfilesPersistence persistence)
		: state_{createDefaultState()},
		persistence_{std::move(persistence)} {
		for (auto& profile : initialState.profiles) {
			if (!profile.builtIn && !isBuiltIn(profile.id)) {
				state_.profiles.push_back(std::move(profile));
			}
		}
		state_.assignments = std::move(initialState.assignments);
	}

	ControlProfilesState ControlProfiles::createDefaultState() {
		return {
			.profiles = {
				{BalancedProfileId, "Balanced", {167, 33}, true},
				{FastProfileId, "Fast", {100, 16}, true},
				{ClassicProfileId, "Classic", {250, 50}, true},
			},
		};
	}

	const std::vector<InputProfile>& ControlProfiles::getProfiles() const {
		return state_.profiles;
	}

	std::optional<InputProfile> ControlProfiles::getProfile(const ProfileId& profileId) const {
		auto profile = std::ranges::find(state_.profiles, profileId, &InputProfile::id);
		if (profile == state_.profiles.end()) {
			return std::nullopt;
		}
		return *profile;
	}

	const std::vector<DeviceProfileAssignment>& ControlProfiles::getAssignments() const {
		return state_.assignments;
	}

	ProfileId ControlProfiles::getAssignedProfileId(std::string_view deviceGuid) const {
		auto assignment = std::ranges::find(state_.assignments, deviceGuid, &DeviceProfileAssignment::deviceGuid);
		if (assignment == state_.assignments.end() || !getProfile(assignment->profileId)) {
			return BalancedProfileId;
		}
		return assignment->profileId;
	}

	InputTiming ControlProfiles::getAssignedTiming(std::string_view deviceGuid) const {
		return getProfile(getAssignedProfileId(deviceGuid))->timing;
	}

	std::size_t ControlProfiles::getAssignedDeviceCount(const ProfileId& profileId) const {
		return static_cast<std::size_t>(std::ranges::count(state_.assignments, profileId, &DeviceProfileAssignment::profileId));
	}

	ControlProfilesResult<std::string> ControlProfiles::getDuplicateProfileName(const ProfileId& profileId) const {
		auto profile = std::ranges::find(state_.profiles, profileId, &InputProfile::id);
		if (profile == state_.profiles.end()) {
			return std::unexpected{ControlProfilesError{
				.code = ControlProfilesErrorCode::ProfileNotFound,
				.message = "Cannot duplicate an unknown control profile.",
			}};
		}

		for (std::size_t copyNumber = 1; copyNumber < std::numeric_limits<std::size_t>::max(); ++copyNumber) {
			auto suffix = copyNumber == 1
				? std::string{" Copy"}
				: " Copy " + std::to_string(copyNumber);
			auto name = truncateUtf8(profile->name, MaxProfileNameLength - suffix.size()) + suffix;
			auto duplicate = std::ranges::find(state_.profiles, name, &InputProfile::name);
			if (duplicate == state_.profiles.end()) {
				return name;
			}
		}

		return std::unexpected{ControlProfilesError{
			.code = ControlProfilesErrorCode::DuplicateName,
			.message = "Could not generate a unique control profile name.",
		}};
	}

	ControlProfilesResult<void> ControlProfiles::assignProfile(
		std::string_view deviceGuid,
		const ProfileId& profileId) {
		if (!getProfile(profileId)) {
			return std::unexpected{ControlProfilesError{
				.code = ControlProfilesErrorCode::ProfileNotFound,
				.message = "Cannot assign an unknown control profile.",
			}};
		}

		auto candidate = state_;
		auto assignment = std::ranges::find(candidate.assignments, deviceGuid, &DeviceProfileAssignment::deviceGuid);
		if (assignment != candidate.assignments.end()) {
			if (assignment->profileId == profileId) {
				return {};
			}
			assignment->profileId = profileId;
		} else {
			candidate.assignments.push_back({std::string{deviceGuid}, profileId});
		}

		return commit(std::move(candidate), {{std::string{deviceGuid}}});
	}

	ControlProfilesResult<ProfileId> ControlProfiles::duplicateProfile(
		std::string_view deviceGuid,
		const ProfileId& sourceProfileId,
		std::string name,
		InputTiming timing) {
		if (!getProfile(sourceProfileId)) {
			return std::unexpected{ControlProfilesError{
				.code = ControlProfilesErrorCode::ProfileNotFound,
				.message = "Cannot duplicate an unknown control profile.",
			}};
		}

		auto validatedName = validateProfile(std::move(name), timing);
		if (!validatedName) {
			return std::unexpected{validatedName.error()};
		}

		auto candidate = state_;
		auto profileId = generateProfileId(candidate);
		candidate.profiles.push_back({profileId, std::move(*validatedName), timing, false});

		auto assignment = std::ranges::find(candidate.assignments, deviceGuid, &DeviceProfileAssignment::deviceGuid);
		if (assignment != candidate.assignments.end()) {
			assignment->profileId = profileId;
		} else {
			candidate.assignments.push_back({std::string{deviceGuid}, profileId});
		}

		auto commitResult = commit(std::move(candidate), {{std::string{deviceGuid}}});
		if (!commitResult) {
			return std::unexpected{commitResult.error()};
		}
		return profileId;
	}

	ControlProfilesResult<void> ControlProfiles::editProfile(
		const ProfileId& profileId,
		std::string name,
		InputTiming timing) {
		auto profile = std::ranges::find(state_.profiles, profileId, &InputProfile::id);
		if (profile == state_.profiles.end()) {
			return std::unexpected{ControlProfilesError{
				.code = ControlProfilesErrorCode::ProfileNotFound,
				.message = "Cannot edit an unknown control profile.",
			}};
		}
		if (isBuiltIn(profileId)) {
			return std::unexpected{ControlProfilesError{
				.code = ControlProfilesErrorCode::BuiltInProfileCannotBeModified,
				.message = "Built-in control profiles cannot be edited.",
			}};
		}

		auto validatedName = validateProfile(std::move(name), timing, profileId);
		if (!validatedName) {
			return std::unexpected{validatedName.error()};
		}
		if (profile->name == *validatedName && profile->timing == timing) {
			return {};
		}

		ControlProfilesChange change;
		for (const auto& assignment : state_.assignments) {
			if (assignment.profileId == profileId) {
				change.affectedDeviceGuids.push_back(assignment.deviceGuid);
			}
		}

		auto candidate = state_;
		auto candidateProfile = std::ranges::find(candidate.profiles, profileId, &InputProfile::id);
		candidateProfile->name = std::move(*validatedName);
		candidateProfile->timing = timing;
		return commit(std::move(candidate), change);
	}

	ControlProfilesResult<std::size_t> ControlProfiles::deleteProfile(const ProfileId& profileId) {
		auto profile = std::ranges::find(state_.profiles, profileId, &InputProfile::id);
		if (profile == state_.profiles.end()) {
			return std::unexpected{ControlProfilesError{
				.code = ControlProfilesErrorCode::ProfileNotFound,
				.message = "Cannot delete an unknown control profile.",
			}};
		}
		if (isBuiltIn(profileId)) {
			return std::unexpected{ControlProfilesError{
				.code = ControlProfilesErrorCode::BuiltInProfileCannotBeModified,
				.message = "Built-in control profiles cannot be deleted.",
			}};
		}

		auto candidate = state_;
		ControlProfilesChange change;
		for (auto& assignment : candidate.assignments) {
			if (assignment.profileId == profileId) {
				assignment.profileId = BalancedProfileId;
				change.affectedDeviceGuids.push_back(assignment.deviceGuid);
			}
		}
		std::erase_if(candidate.profiles, [&profileId](const InputProfile& candidateProfile) {
			return candidateProfile.id == profileId;
		});

		const auto affectedDeviceCount = change.affectedDeviceGuids.size();
		auto commitResult = commit(std::move(candidate), change);
		if (!commitResult) {
			return std::unexpected{commitResult.error()};
		}
		return affectedDeviceCount;
	}

	ControlProfilesResult<std::string> ControlProfiles::validateProfile(
		std::string name,
		InputTiming timing,
		const std::optional<ProfileId>& ignoredProfileId) const {
		const auto first = name.find_first_not_of(" \t\r\n\f\v");
		if (first == std::string::npos) {
			return std::unexpected{ControlProfilesError{
				.code = ControlProfilesErrorCode::EmptyName,
				.message = "Enter a control profile name.",
			}};
		}
		const auto last = name.find_last_not_of(" \t\r\n\f\v");
		name = name.substr(first, last - first + 1);

		if (utf8Length(name) > MaxProfileNameLength) {
			return std::unexpected{ControlProfilesError{
				.code = ControlProfilesErrorCode::NameTooLong,
				.message = "Control profile names cannot exceed 29 characters.",
			}};
		}

		for (const auto& profile : state_.profiles) {
			if (profile.name == name && (!ignoredProfileId || profile.id != *ignoredProfileId)) {
				return std::unexpected{ControlProfilesError{
					.code = ControlProfilesErrorCode::DuplicateName,
					.message = "A control profile with this name already exists.",
				}};
			}
		}

		if (timing.dasMs < MinDasMs || timing.dasMs > MaxDasMs) {
			return std::unexpected{ControlProfilesError{
				.code = ControlProfilesErrorCode::DasOutOfRange,
				.message = "DAS must be between 1 and 1000 ms.",
			}};
		}
		if (timing.arrMs < MinArrMs || timing.arrMs > MaxArrMs) {
			return std::unexpected{ControlProfilesError{
				.code = ControlProfilesErrorCode::ArrOutOfRange,
				.message = "ARR must be between 1 and 250 ms.",
			}};
		}

		return name;
	}

	ControlProfilesResult<void> ControlProfiles::commit(
		ControlProfilesState candidate,
		const ControlProfilesChange& change) {
		if (persistence_) {
			ControlProfilesPersistenceResult result;
			try {
				result = persistence_(candidate);
			} catch (const std::exception& exception) {
				return std::unexpected{ControlProfilesError{
					.code = ControlProfilesErrorCode::PersistenceFailed,
					.message = "Could not save control profiles: " + std::string{exception.what()},
				}};
			} catch (...) {
				return std::unexpected{ControlProfilesError{
					.code = ControlProfilesErrorCode::PersistenceFailed,
					.message = "Could not save control profiles due to an unexpected error.",
				}};
			}
			if (!result) {
				auto message = result.error().empty()
					? std::string{"Could not save control profiles."}
					: "Could not save control profiles: " + result.error();
				return std::unexpected{ControlProfilesError{
					.code = ControlProfilesErrorCode::PersistenceFailed,
					.message = std::move(message),
				}};
			}
		}

		state_ = std::move(candidate);
		changed(change);
		return {};
	}

	bool ControlProfiles::isBuiltIn(const ProfileId& profileId) const {
		return profileId == BalancedProfileId
			|| profileId == FastProfileId
			|| profileId == ClassicProfileId;
	}

	ProfileId ControlProfiles::generateProfileId(const ControlProfilesState& state) const {
		ProfileId profileId;
		do {
			profileId.value = util::generateUuid();
		} while (std::ranges::find(state.profiles, profileId, &InputProfile::id) != state.profiles.end());
		return profileId;
	}

}
