#ifndef BOARDBATCH_H
#define BOARDBATCH_H

#include "boardshader.h"
#include "../logger.h"

#include <sdl/batch.h>
#include <sdl/sprite.h>
#include <sdl/window.h>

#include <vector>

namespace tetris {

	inline void addRectangle(std::vector<BoardShader::Vertex>& vertexes,
		float x, float y, float w, float h, const Color& color) {

		vertexes.push_back(BoardShader::Vertex{x, y, color});
		vertexes.push_back(BoardShader::Vertex{x + w, y, color});
		vertexes.push_back(BoardShader::Vertex{x, y + h, color});
		vertexes.push_back(BoardShader::Vertex{x, y + h, color});
		vertexes.push_back(BoardShader::Vertex{x + w, y, color});
		vertexes.push_back(BoardShader::Vertex{x + w, y + h, color});
	}

	inline void addRectangle(std::vector<BoardShader::Vertex>& vertexes,
		float x, float y, float w, float h, const sdl::Sprite& sprite, const Color& color = Color{1, 1, 1, 1}) {

		float textureW = sprite.getWidth();
		float textureH = sprite.getHeight();

		vertexes.push_back(BoardShader::Vertex{x, y, sprite.getX() / textureW, sprite.getY() / textureH, color});
		vertexes.push_back(BoardShader::Vertex{x + w, y, (sprite.getX() + sprite.getWidth()) / textureW, sprite.getY() / textureH, color});
		vertexes.push_back(BoardShader::Vertex{x, y + h, sprite.getX() / textureW, (sprite.getY() + sprite.getHeight()) / textureH, color});

		vertexes.push_back(BoardShader::Vertex{x, y + h, sprite.getX() / textureW, (sprite.getY() + sprite.getHeight()) / textureH, color});
		vertexes.push_back(BoardShader::Vertex{x + w, y, (sprite.getX() + sprite.getWidth()) / textureW, sprite.getY() / textureH, color});
		vertexes.push_back(BoardShader::Vertex{x + w, y + h, (sprite.getX() + sprite.getWidth()) / textureW, (sprite.getY() + sprite.getHeight()) / textureH, color});
	}

	class BoardBatch {
	public:
		BoardBatch(const std::shared_ptr<BoardShader>& shader, int maxVertexes)
			: batch_{GL_DYNAMIC_DRAW}
			, shader_(shader) {

			logger()->info("[BoardBatch] {} Mib\n", batch_.getVboSizeInMiB());
		}

		BoardBatch(const std::shared_ptr<BoardShader>& shader)
			: batch_{GL_TRIANGLES} {
		}

		virtual ~BoardBatch() = default;

		void clear() {
			batch_.clear();
		}

		void uploadToGraphicCard() {
			batch_.uploadToGraphicCard();
		}

		void add(const std::vector<BoardShader::Vertex>& data) {
			batch_.insert(data.begin(), data.end());
		}

		void addTriangle(const BoardShader::Vertex& v1, const BoardShader::Vertex& v2, const BoardShader::Vertex& v3) {
			batch_.pushBack(v1);
			batch_.pushBack(v2);
			batch_.pushBack(v3);
		}

		void addRectangle(float x, float y, float w, float h, const Color& color) {
			addTriangle(BoardShader::Vertex{x, y, color}, BoardShader::Vertex{x + w, y, color}, BoardShader::Vertex{x, y + h, color});
			addTriangle(BoardShader::Vertex{x, y + h, color}, BoardShader::Vertex{x + w, y, color}, BoardShader::Vertex{x + w, y + h, color});
		}

		void addSquare(float x, float y, float size, const Color& color) {
			addRectangle(x, y, size, size, color);
		}

		void addRectangle(float x, float y, float w, float h, const sdl::Sprite& sprite) {
			int textureW = sprite.getTextureWidth();
			int textureH = sprite.getTextureHeight();

			addTriangle(BoardShader::Vertex(x, y, sprite.getX() / textureW, sprite.getY() / textureH),
				BoardShader::Vertex(x + w, y, (sprite.getX() + sprite.getWidth()) / textureW, sprite.getY() / textureH),
				BoardShader::Vertex(x, y + h, sprite.getX() / textureW, (sprite.getY() + sprite.getHeight()) / textureH));

			addTriangle(BoardShader::Vertex(x, y + h, sprite.getX() / textureW, (sprite.getY() + sprite.getHeight()) / textureH),
				BoardShader::Vertex(x + w, y, (sprite.getX() + sprite.getWidth()) / textureW, sprite.getY() / textureH),
				BoardShader::Vertex(x + w, y + h, (sprite.getX() + sprite.getWidth()) / textureW, (sprite.getY() + sprite.getHeight()) / textureH));
		}

		void addSquare(float x, float y, float size, const sdl::Sprite& sprite, const Color& color) {
			addRectangle(x, y, size, size, sprite, color);
		}

		void addRectangle(float x, float y, float w, float h, const sdl::Sprite& sprite, const Color& color) {
			int textureW = sprite.getTextureWidth();
			int textureH = sprite.getTextureHeight();

			addTriangle(BoardShader::Vertex{x, y, sprite.getX() / textureW, sprite.getY() / textureH, color},
				BoardShader::Vertex{x + w, y, (sprite.getX() + sprite.getWidth()) / textureW, sprite.getY() / textureH, color},
				BoardShader::Vertex{x, y + h, sprite.getX() / textureW, (sprite.getY() + sprite.getHeight()) / textureH, color});

			addTriangle(BoardShader::Vertex{x, y + h, sprite.getX() / textureW, (sprite.getY() + sprite.getHeight()) / textureH, color},
				BoardShader::Vertex{x + w, y, (sprite.getX() + sprite.getWidth()) / textureW, sprite.getY() / textureH, color},
				BoardShader::Vertex{x + w, y + h, (sprite.getX() + sprite.getWidth()) / textureW, (sprite.getY() + sprite.getHeight()) / textureH, color});
		}

		void addRectangle(const BoardShader::Vertex& v1, const BoardShader::Vertex& v2, const BoardShader::Vertex& v3, const BoardShader::Vertex& v4) {
			addTriangle(v1, v2, v3);
			addTriangle(v3, v4, v1);
		}

		void addCircle(float x, float y, float radius, const int iterations = 40) {
			const float PI = 3.14159265359f;

			const BoardShader::Vertex v1{x, y};
			BoardShader::Vertex v2{x + radius, y};
			for (int i = 1; i <= iterations; ++i) {
				BoardShader::Vertex v3{x + radius * std::cos(2 * PI * i / iterations), y + radius * std::sin(2 * PI * i / iterations)};
				addTriangle(v1, v2, v3);
				v2 = v3;
			}
		}

		void addLine(float x1, float y1, float x2, float y2, float w) {
			float angle = atan2(x2 - x1, y2 - y1);
			float dx = 0.5f * w * std::cos(angle);
			float dy = 0.5f * w * std::sin(angle);
			addRectangle(BoardShader::Vertex{x1 - dx, y1 + dy}, BoardShader::Vertex{x1 + dx, y1 - dy}, BoardShader::Vertex{x2 + dx, y2 - dy}, BoardShader::Vertex{x2 - dx, y2 + dy});
		}

	private:
		std::shared_ptr<BoardShader> shader_;
		sdl::Batch<BoardShader::Vertex> batch_;
	};

}

#endif // BOARDBATCH_H
