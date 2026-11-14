#ifndef APP_CONFIGURATIONNORMALIZATION_H
#define APP_CONFIGURATIONNORMALIZATION_H

#include "controlprofiles.h"
#include "game/inputbindings.h"

#include <nlohmann/json_fwd.hpp>

#include <vector>

namespace app {

	// Kept in a separate internal header so normalization and repair logic used by
	// Configuration can be tested directly without exposing Configuration internals.
	ControlProfilesState normalizeControlProfiles(nlohmann::json& jsonObject);
	std::vector<game::DeviceBinding> normalizeDeviceBindings(nlohmann::json& jsonObject);

}

#endif
