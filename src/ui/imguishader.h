#ifndef MWETRIS_IMGUISHADER_H
#define MWETRIS_IMGUISHADER_H

#include "../types.h"

#include <sdl/shaderprogram.h>

namespace tetris {

	class ImGuiShader {
	public:
		ImGuiShader();

		ImGuiShader(const GLchar* vShade, const GLchar* fShader);

		void useProgram() const;

		void setVertexAttribPointer() const;

		void setMatrix(const tetris::Mat44& matrix) const;

		void setTexture(bool useTexture) const;

	private:
		sdl::ShaderProgram shader_;

		// Vertex buffer attributes.
		int aPosIndex_;
		int aTexIndex_;
		int aColorIndex_;

		// Vertex buffer uniform.
		int uMatrixIndex_;
		int uTextureIndex_;
	};

} // Namespace tetris.

#endif // MWETRIS_IMGUISHADER_H
