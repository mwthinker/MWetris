#ifndef BOARDSHADER_H
#define BOARDSHADER_H

#include "../types.h"
#include <sdl/shaderprogram.h>
#include <sdl/sprite.h>

#include <memory>

namespace tetris {

	class BoardShader;
	using BoardShaderPtr = std::shared_ptr<BoardShader>;

	class BoardShader {
	public:
		BoardShader();
		BoardShader(std::string vShaderFile, std::string fShaderFile);

		void useProgram() const;

		void setVertexAttribPointer() const;

		// Uniforms. -------------------------------------------
		void setMatrix(const Mat44& matrix) const;

		class Vertex {
		public:
			Vertex() = default;
			Vertex(const Vertex&) = default;
			Vertex& operator=(const Vertex&) = default;
			Vertex(Vertex&&) = default;
			Vertex& operator=(Vertex&&) = default;

			Vertex(GLfloat x, GLfloat y)
				: pos_{x, y}
				, color_{1, 1, 1, 1}
				, texture_{0} {
			}

			Vertex(GLfloat x, GLfloat y, const Color& color)
				: pos_{x, y}
				, color_{color}
				, texture_{0} {
			}

			Vertex(GLfloat x, GLfloat y, GLfloat xTex, GLfloat yTex)
				: pos_{x, y}
				, tex_{xTex, yTex}
				, color_{1, 1, 1, 1}
				, texture_{1} {
			}

			Vertex(GLfloat x, GLfloat y, GLfloat xTex, GLfloat yTex, const Color& color)
				: pos_{x, y}
				, tex_{xTex, yTex}
				, color_{color}
				, texture_{1} {
			}

			bool isTexture() const {
				return texture_;
			}

			// The order is important for setVertexAttribPointer()
			Vec2 pos_;
			Vec2 tex_;
			GLfloat texture_;
			Color color_;
		};

	private:
		sdl::ShaderProgram shader_;

		// Vertex buffer attributes.
		int aPosIndex_;
		int aTexIndex_;
		int aTextureIndex_;
		int aColorIndex_;

		// Vertex buffer uniform.
		int uMatrixIndex_;
	};

}

#endif // BOARDSHADER_H
