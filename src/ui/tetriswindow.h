#ifndef TETRISWINDOW_H
#define TETRISWINDOW_H

#include "../ui/imguiwindow.h"
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
