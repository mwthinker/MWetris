#ifndef CPPSDL2_SDL_TEXTUREVIEW_H
#define CPPSDL2_SDL_TEXTUREVIEW_H

#include <SDL3/SDL_rect.h>
#include <glm/vec2.hpp>

namespace app {

	/// @brief Normalized coordinates
	struct TextureView {
		glm::vec2 pos{0.f, 0.f};
		glm::vec2 size{1.f, 1.f};
	};

}

#endif
