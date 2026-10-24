#ifndef APP_UI_THEME_H
#define APP_UI_THEME_H

#include <sdl/color.h>

namespace app::ui::theme {

	inline constexpr auto Success = sdl::Color::createU32(41, 107, 69);
	inline constexpr auto SuccessHovered = sdl::Color::createU32(51, 133, 84);
	inline constexpr auto SuccessActive = sdl::Color::createU32(33, 89, 56);

	inline constexpr auto Danger = sdl::Color::createU32(140, 41, 41);
	inline constexpr auto DangerHovered = sdl::Color::createU32(173, 51, 51);
	inline constexpr auto DangerActive = sdl::Color::createU32(115, 31, 31);

	inline constexpr auto ErrorText = sdl::Color::createU32(255, 89, 89);
	inline constexpr auto Link = sdl::Color::createU32(77, 166, 255);
	inline constexpr auto LinkHovered = sdl::Color::createU32(128, 199, 255);

}

#endif
