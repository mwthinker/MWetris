#ifndef MWETRIS_UI_SCENE_PLAY_H
#define MWETRIS_UI_SCENE_PLAY_H

#include "scene.h"

#include "game/device.h"
#include "game/sdldevice.h"
#include "game/tetrisgame.h"
#include "types.h"
#include "tetrisdata.h"
#include "graphic/gamecomponent.h"
#include "graphic/graphic.h"
#include "ui/imguiextra.h"
#include "ai.h"

#include <sdl/sprite.h>

#include <array>
#include <vector>
#include <array>
#include <memory>

namespace mwetris::ui::scene {

	class Play : public Scene {
	public:
		Play();

		void eventUpdate(const SDL_Event& windowEvent) override;

		void draw(const sdl::Shader& shader, const std::chrono::high_resolution_clock::duration& deltaTime) override;

		void imGuiUpdate(const std::chrono::high_resolution_clock::duration& deltaTime) override;

	private:
		void resumeGame();

		std::vector<game::DevicePtr> getCurrentDevices() const;

		game::DevicePtr findHumanDevice(std::string name) const;
		game::DevicePtr findAiDevice(std::string name) const;

		std::unique_ptr<graphic::GameComponent> gameComponent_;
		std::array<game::DevicePtr, 4> activeAis_;
		game::TetrisGame game_;
		int nbrHumans_{1};
		int nbrAis_{0};
		std::vector<game::SdlDevicePtr> devices_;
		graphic::Graphic graphic_;
		sdl::Sprite crossSprite_;
		sdl::Sprite manSprite_;
		sdl::Sprite aiSprite_;
		Vec2 size_{};
	};

}

#endif
