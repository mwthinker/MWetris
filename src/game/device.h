#ifndef MWETRIS_GAME_DEVICE_H
#define MWETRIS_GAME_DEVICE_H

#include "input.h"
#include "tetrisboardwrapper.h"

#include <SDL.h>

#include <string>
#include <memory>

namespace mwetris::game {

	class Device;
	using DevicePtr = std::shared_ptr<Device>;

	class Device {
	public:
		virtual ~Device() = default;

		virtual Input getInput() const = 0;

		virtual std::string getName() const = 0;

		virtual void update(const TetrisBoardWrapper& board) = 0;

		virtual bool isAi() const = 0;
	};

}

#endif
