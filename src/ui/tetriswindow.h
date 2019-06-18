#ifndef TETRISWINDOW_H
#define TETRISWINDOW_H

#include "imgui_impl_sdl.h"
#include "imgui_impl_opengl3.h"

#include "../game/tetrisgame.h"

#include <sdl/window.h>
#include <sdl/sprite.h>
#include <sdl/textureatlas.h>

#include "imguiwindow.h"

namespace tetris {

	class TetrisWindow : public ImGuiWindow {
	public:
		TetrisWindow();

		~TetrisWindow();
	private:
		TetrisGame tetrisGame_;
	};

} // Namespace tetris.

#endif // TETRISWINDOW_H
