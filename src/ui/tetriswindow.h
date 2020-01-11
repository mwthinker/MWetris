#ifndef TETRISWINDOW_H
#define TETRISWINDOW_H

#include "../game/tetrisgame.h"

#include <sdl/imguiwindow.h>
#include <sdl/sprite.h>
#include <sdl/textureatlas.h>

#include <imgui.h>

namespace tetris {

	class TetrisWindow : public sdl::ImGuiWindow {
	public:
		TetrisWindow();

		virtual ~TetrisWindow();

		enum class Page { MENU, PLAY, HIGHSCORE, CUSTOM, SETTINGS, NEW_HIGHSCORE, NETWORK };
	
	protected:
		void initPreLoop() override;

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

		void beginBar();

		void endBar();	

		void beginMain();

		void endMain();

		void imGuiUpdate(const std::chrono::high_resolution_clock::duration& deltaTime) override;

		void eventUpdate(const SDL_Event& windowEvent) override;

		void playPage();

		void menuPage();

		void highscorePage();

		void customGamePage();

		void settingsPage();

		void networkPage();
		
		float menuHeight_;
		TetrisGame game_;
	};

} // Namespace tetris.

#endif // TETRISWINDOW_H
