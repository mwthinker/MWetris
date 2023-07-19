#ifndef MWETRIS_GAME_DEVICE_H
#define MWETRIS_GAME_DEVICE_H

#include "input.h"

#include <tetrisboard.h>

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

		virtual const char* getName() const = 0;

		virtual void onGameboardEvent(const tetris::TetrisBoard& board, tetris::BoardEvent, int value) {}
	};

}

#endif