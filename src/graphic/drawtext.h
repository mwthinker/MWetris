#ifndef DRAWTEXT_H
#define DRAWTEXT_H

#include "boardbatch.h"

#include <sdl/text.h>

namespace tetris {

	class DrawText {
	public:
		DrawText() = default;

		DrawText(float lowX, float lowY);

		DrawText(std::string text, const sdl::Font& font, float lowX, float lowY,
			float scale = 1.f, bool center = false);

		void update(std::string text);

		void update(const sdl::Text& text);

		void update(std::string text, float x, float y);

		void update(float x, float y);

		void bindTexture();

		const std::vector<BoardShader::Vertex>& getVertexes() {
			return vertexes_;
		}

		bool isEmpty() const {
			return text_.getSprite().isValid();
		}

		float getLowX() const {
			if (center_) {
				return lowX_ - text_.getWidth() * 0.5f;
			} else {
				return lowX_;
			}
		}

		float getLowY() const {
			if (center_) {
				return lowY_ - text_.getHeight() * 0.5f;
			} else {
				return lowY_;
			}
		}

		float getWidth() const {
			return text_.getWidth();
		}

		float getHeight() const {
			return text_.getHeight();
		}

	private:
		void updateVertexes();

		sdl::Text text_;
		float lowX_{};
		float lowY_{};
		bool center_{};
		std::vector<BoardShader::Vertex> vertexes_;
	};

}

#endif // DRAWTEXT_H
