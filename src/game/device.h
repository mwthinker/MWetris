#ifndef DEVICE_H
#define DEVICE_H

#include "input.h"

#include <SDL.h>

#include <string>
#include <memory>

namespace tetris {

	class TetrisBoard;

	class Device;
	using DevicePtr = std::shared_ptr<Device>;

	class Device {
	public:
		virtual ~Device() = default;

		virtual Input getInput() const = 0;

		virtual std::string getName() const = 0;

		virtual void update(const TetrisBoard& board) = 0;

		virtual bool isAi() const = 0;
	};

}

#endif // DEVICE_H
