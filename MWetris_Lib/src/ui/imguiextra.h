#ifndef MWETRIS_IMGUIEXTRA_H
#define MWETRIS_IMGUIEXTRA_H

#include "types.h"
#include "configuration.h"

#include <sdl/imguiauxiliary.h>

#include <sdl/sprite.h>
#include <chrono>

namespace ImGui {

	void PushButtonStyle();

	void PopButtonStyle();

	void Text(const std::chrono::year_month_day& ymd);

	void TextWithBackgroundColor(const char* str, sdl::Color color, const ImVec2& padding = {});

	void TextWithBackgroundColor(int nbr, sdl::Color color, const ImVec2& padding = {});

	void IgnoreCursor(std::invocable auto&& t);

}

#endif
