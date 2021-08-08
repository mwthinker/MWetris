#ifndef MWETRIS_UI_SCENE_PLAY_H
#define MWETRIS_UI_SCENE_PLAY_H

#include "scene.h"

#include "game/device.h"
#include "game/sdldevice.h"
#include "game/computer.h"
#include "game/tetrisgame.h"
#include "types.h"
#include "configuration.h"
#include "graphic/gamecomponent.h"
#include "ui/imguiextra.h"
#include "ai.h"

#include <sdl/graphic.h>
#include <sdl/sprite.h>

#include <array>
#include <vector>
#include <array>
#include <memory>

namespace mwetris::ui::scene {

	class Play : public Scene {
	public:
		explicit Play(sdl::Graphic& graphic);

		void eventUpdate(const SDL_Event& windowEvent) override;

		void draw(sdl::Shader& shader, const DeltaTime& deltaTime) override;

		void imGuiUpdate(const DeltaTime& deltaTime) override;

	private:
		void switchedFrom() override;

		void switchedTo() override;

		void resumeGame();

		std::vector<game::DevicePtr> getCurrentDevices() const;

		game::DevicePtr findHumanDevice(const std::string& name) const;

		std::unique_ptr<graphic::GameComponent> gameComponent_;

		std::unique_ptr<game::TetrisGame> game_;
		int nbrHumans_ = 1;
		int nbrAis_ = 0;
		std::vector<game::SdlDevicePtr> devices_;
		std::vector<game::ComputerPtr> computers_;

		sdl::Graphic& graphic_;
		sdl::TextureView crossSprite_;
		sdl::TextureView manSprite_;
		sdl::TextureView aiSprite_;
		Vec2 size_{};
		mw::signals::ScopedConnections connections_;
	};

}

#endif
