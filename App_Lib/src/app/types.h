#ifndef APP_TYPES_H
#define APP_TYPES_H

#include <glm/glm.hpp>
#include <glm/mat4x2.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <sdl/color.h>

namespace app {

	using Mat4 = glm::mat4x4;
	using Vec2 = glm::vec2;
	using Vec3 = glm::vec3;
	using Vec4 = glm::vec4;
	using Color = sdl::Color;
	
	namespace color = sdl::color;

}

#endif
