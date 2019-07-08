#ifndef IMGUIWINDOW_H
#define IMGUIWINDOW_H

#include "imgui_impl_sdl.h"
#include "imgui_impl_opengl3.h"

#include "../game/tetrisgame.h"

#include <sdl/window.h>
#include <sdl/sprite.h>
#include <sdl/textureatlas.h>

#include <imgui.h>

namespace tetris {

	class ImGuiWindow : public sdl::Window {
	public:
		ImGuiWindow();

		virtual ~ImGuiWindow();

		enum class Page { MENU, PLAY, HIGHSCORE, CUSTOM, SETTINGS, NEW_HIGHSCORE, NETWORK };
	protected:		

		void initOpenGl();

		void initPreLoop();

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

		void update(double deltaTime);

		void eventUpdate(const SDL_Event& windowEvent);

		void resize(int width, int height);

		void playPage();

		void menuPage();

		void highscorePage();

		void customGamePage();

		void settingsPage();

		void networkPage();

		bool show_demo_window;
		bool show_another_window;
		float menuHeight_;

		bool initiatedOpenGl_;
		bool initiatedSdl_;
	};

} // Namespace tetris.

#endif // IMGUIWINDOW_H
