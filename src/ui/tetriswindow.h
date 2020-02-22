#ifndef TETRISWINDOW_H
#define TETRISWINDOW_H

#include "../game/tetrisgame.h"
#include "../graphic/graphic.h"
#include "../graphic/drawboard.h"

#include "../game/device.h"
#include "../game/sdldevice.h"
#include <sdl/imguiwindow.h>
#include <sdl/sprite.h>
#include <sdl/textureatlas.h>

#include <imgui.h>

#include <array>

namespace tetris {

	class GameComponent;

	class TetrisWindow : public sdl::ImGuiWindow {
	public:
		enum class Page { MENU, PLAY, HIGHSCORE, CUSTOM, SETTINGS, NEW_HIGHSCORE, NETWORK, TEST };

		TetrisWindow();

		~TetrisWindow();

		void setStartPage(Page page) {
			currentPage_ = page;
		}
	
	private:
		void initPreLoop() override;
	
		DevicePtr findHumanDevice(std::string name) const;
		DevicePtr findAiDevice(std::string name) const;
		void resumeGame();

		ImFontAtlas fontAtlas_;
		ImFont* headerFont_;
		ImFont* defaultFont_;
		ImFont* buttonFont_;
		sdl::Sprite manTexture_;
		sdl::Sprite noManTexture_;
		sdl::Sprite aiTexture_;
		sdl::Sprite background_;
		ImColor labelColor_;
		ImColor buttonTextColor_;

		void changePage(Page page);
		
		int nbrHumans_;
		int nbrAis_;
		Page currentPage_;

	private:
		void pushButtonStyle();

		void popButtonStyle();

		void imGuiPreUpdate(const std::chrono::high_resolution_clock::duration& deltaTime) override;

		void imGuiUpdate(const std::chrono::high_resolution_clock::duration& deltaTime) override;

		void eventUpdate(const SDL_Event& windowEvent) override;

		void playPage();

		void menuPage();

		void highscorePage();

		void customGamePage();

		void settingsPage();

		void networkPage();
	
		float menuHeight_{};
		TetrisGame game_;
		std::array<DevicePtr, 4> activeAis_;
		std::unique_ptr<GameComponent> gameComponent_;
		Graphic graphic;
		DrawBoard drawBoard_;
		std::vector<SdlDevicePtr> devices_;
	};

} // Namespace tetris.

#endif // TETRISWINDOW_H
