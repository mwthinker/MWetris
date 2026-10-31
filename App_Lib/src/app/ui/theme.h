#ifndef APP_UI_THEME_H
#define APP_UI_THEME_H

#include <sdl/color.h>

#include <string_view>

namespace app::ui::theme {

	inline constexpr auto Success = sdl::Color::createU32(54, 160, 126);
	inline constexpr auto SuccessHovered = sdl::Color::createU32(81, 193, 157);
	inline constexpr auto SuccessActive = sdl::Color::createU32(104, 216, 180);
	inline constexpr auto SuccessButtonText = sdl::Color::createU32(8, 26, 21);
	inline constexpr auto SuccessText = sdl::Color::createU32(72, 210, 120);

	inline constexpr auto Danger = sdl::Color::createU32(140, 41, 41);
	inline constexpr auto DangerHovered = sdl::Color::createU32(173, 51, 51);
	inline constexpr auto DangerActive = sdl::Color::createU32(115, 31, 31);
	inline constexpr auto DangerText = sdl::Color::createU32(255, 89, 89);

	inline constexpr auto ErrorText = sdl::Color::createU32(255, 89, 89);
	inline constexpr auto Link = sdl::Color::createU32(77, 166, 255);
	inline constexpr auto LinkHovered = sdl::Color::createU32(128, 199, 255);

	struct SliderPalette {
		sdl::Color grab;
		sdl::Color active;
	};

	inline constexpr SliderPalette sliderPalette(std::string_view colorScheme, sdl::Color customAccent) {
		if (colorScheme == "frostedBlue") {
			return {sdl::Color::createU32(120, 184, 212), sdl::Color::createU32(154, 213, 237)};
		}
		if (colorScheme == "classicCrimson") {
			return {sdl::Color::createU32(214, 107, 125), sdl::Color::createU32(239, 132, 149)};
		}
		if (colorScheme == "warmSand") {
			return {sdl::Color::createU32(198, 146, 62), sdl::Color::createU32(227, 173, 84)};
		}
		if (colorScheme == "highContrast") {
			return {sdl::Color::createU32(255, 230, 0), sdl::Color::createU32(255, 243, 106)};
		}
		return {customAccent, customAccent};
	}

}

#endif
