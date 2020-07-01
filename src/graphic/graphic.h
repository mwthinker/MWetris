#ifndef MWETRIS_GRAPHIC_GRAPHIC_H
#define MWETRIS_GRAPHIC_GRAPHIC_H

#include "types.h"

#include <sdl/shader.h>
#include <sdl/vertex.h>
#include <sdl/sprite.h>
#include <sdl/batch.h>
#include <sdl/vertexarrayobject.h>
#include <sdl/textureview.h>

#include <array>
#include <functional>

namespace tetris::graphic {

    class Graphic {
    public:
		Graphic();

		void setMatrix(const Mat4& matrix);

		const Mat4& currentMatrix() const {
			return matrixes_.back();
		}

		void pushMatrix();
		void pushMatrix(const Mat4& matrix);

		void multMatrix(const Mat4& matrix);

		void addFilledHexagon(const Vec2& center, float radius, Color color, float startAngle = 0);

		void addRectangle(const Vec2& pos, const Vec2& size, Color color);

		void addRectangleImage(const Vec2& pos, const Vec2& size, const sdl::TextureView& textureView);

		void addCircle(const Vec2& center, float radius, Color color, const int iterations = 40, float startAngle = 0);

		void addHexagonImage(const Vec2& center, float radius, const sdl::TextureView& textureView, float startAngle = 0);
		
		void addHexagon(const Vec2& center, float innerRadius, float outerRadius, Color color, float startAngle = 0);

		void draw(const sdl::Shader& shader);

		void clearDraw();

    private:
		using Batch = sdl::Batch<sdl::Vertex>;
		using BatchView = sdl::BatchView<sdl::Vertex>;

		struct BatchData {
			BatchData() = default;
			BatchData(BatchView&& batchView, int matrixIndex);
			BatchData(BatchView&& batchView, const sdl::TextureView& texture, int matrixIndex);

			BatchView batchView;
			GLuint texture{};
			int matrixIndex{};
		};

		void bind(const sdl::Shader& shader);

		void draw(const sdl::Shader& shader, const BatchData& batchData);
		
		Batch batch_{GL_DYNAMIC_DRAW};
		BatchView lastView_;
		std::vector<BatchData> batches_;
		std::vector<Mat4> matrixes_;
		sdl::VertexArrayObject vao_;
		int currentMatrix_{};
		bool initiated_{};
    };

}

#endif
