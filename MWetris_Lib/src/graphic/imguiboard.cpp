#include "imguiboard.h"
#include "configuration.h"
#include "tetrisboard.h"
#include "game/playerboard.h"

#include "configuration.h"
#include "util.h"

#include <sdl/imguiauxiliary.h>

#include <spdlog/spdlog.h>

#include <variant>
#include <concepts>

namespace mwetris::graphic {

	namespace {

		Vec2 calculateCenterOfMass(const tetris::Block& block) {
			Vec2 pos{};
			for (const auto& sq : block) {
				pos += Vec2{sq.column, sq.row};
			}
			return 1.f / block.getSize() * pos;
		}

		constexpr float middleDistance = 5.f;
		constexpr float rightDistance = 5.f;

		Color setAlpha(Color color, float alpha) {
			return color * Color{1.f, 1.f, 1.f, alpha};
		}

		constexpr float NormalizedPreviewSize = 5.f;

	}

	ImGuiBoard::ImGuiBoard(game::PlayerBoardPtr playerBoard)
		: playerBoard_{playerBoard} {

		spriteZ_ = Configuration::getInstance().getSprite(tetris::BlockType::Z);
		spriteS_ = Configuration::getInstance().getSprite(tetris::BlockType::S);
		spriteJ_ = Configuration::getInstance().getSprite(tetris::BlockType::J);
		spriteI_ = Configuration::getInstance().getSprite(tetris::BlockType::I);
		spriteL_ = Configuration::getInstance().getSprite(tetris::BlockType::L);
		spriteT_ = Configuration::getInstance().getSprite(tetris::BlockType::T);
		spriteO_ = Configuration::getInstance().getSprite(tetris::BlockType::O);

		squareSize_ = Configuration::getInstance().getTetrisSquareSize();
		borderSize_ = Configuration::getInstance().getTetrisBorderSize();

		connections_ += playerBoard_->gameboardEventUpdate.connect(this, &ImGuiBoard::gameBoardEvent);
	}

	ImGuiBoard::ImGuiBoard(const ImGuiBoard& imGuiBoard)
		: squareSize_{imGuiBoard.squareSize_}
		, borderSize_{imGuiBoard.borderSize_}
		, playerBoard_{imGuiBoard.playerBoard_}
		, rows_{imGuiBoard.rows_}
		, height_{imGuiBoard.height_}
		, spriteI_{imGuiBoard.spriteI_}
		, spriteJ_{imGuiBoard.spriteJ_}
		, spriteL_{imGuiBoard.spriteL_}
		, spriteO_{imGuiBoard.spriteO_}
		, spriteS_{imGuiBoard.spriteS_}
		, spriteT_{imGuiBoard.spriteT_}
		, spriteZ_{imGuiBoard.spriteZ_} {

		connections_ += playerBoard_->gameboardEventUpdate.connect(this, &ImGuiBoard::gameBoardEvent);
	}

	ImGuiBoard& ImGuiBoard::operator=(const ImGuiBoard& imGuiBoard) {
		squareSize_ = imGuiBoard.squareSize_;
		borderSize_ = imGuiBoard.borderSize_;
		playerBoard_ = imGuiBoard.playerBoard_;
		rows_ = imGuiBoard.rows_;
		height_ = imGuiBoard.height_;
		spriteI_ = imGuiBoard.spriteI_;
		spriteJ_ = imGuiBoard.spriteJ_;
		spriteL_ = imGuiBoard.spriteL_;
		spriteO_ = imGuiBoard.spriteO_;
		spriteS_ = imGuiBoard.spriteS_;
		spriteT_ = imGuiBoard.spriteT_;
		spriteZ_ = imGuiBoard.spriteZ_;

		connections_ += playerBoard_->gameboardEventUpdate.connect(this, &ImGuiBoard::gameBoardEvent);
		return *this;
	}

	void ImGuiBoard::gameBoardEvent(tetris::BoardEvent boardEvent, int nbr) {
		switch (boardEvent) {
			case tetris::BoardEvent::RowToBeRemoved:
				for (int y = nbr + 1; y < rows_.size(); ++y) {
					rows_[y] += 1;
				}
				break;
		}
	}

	void ImGuiBoard::drawBlock(const tetris::Block& block, Vec2 pos, bool center, Color color) {
		Vec2 delta{};
		if (center) {
			delta = (Vec2{1.f, 3.f} - calculateCenterOfMass(block)) * squareSize_;
			delta.y = NormalizedPreviewSize * squareSize_ - delta.y;
		} else {
			delta = Vec2{0.f, (playerBoard_->getRows() - 2) * squareSize_};
		}
		
		auto drawList = ImGui::GetWindowDrawList();

		auto texture = getSprite(block.getBlockType());
		drawList->PushTextureID((ImTextureID)(intptr_t) texture);

		for (const auto& sq : block) {
			auto x = pos.x + sq.column * squareSize_ + delta.x;
			auto y = pos.y - (sq.row + 1) * squareSize_ + delta.y;
			
			if (sq.row < playerBoard_->getRows() - 2) {
				drawList->PrimReserve(6, 4);
				ImGui::Helper::AddImageQuad(texture, {x, y}, Vec2{squareSize_, squareSize_}, color);
			}
		}
		drawList->PopTextureID();
	}

	void ImGuiBoard::drawGrid(int columns, int rows) {
		auto drawList = ImGui::GetWindowDrawList();
		Vec2 cursorPos = ImGui::GetCursorScreenPos();
		Vec2 sqSize{squareSize_ , squareSize_};

		auto startAreaColor = Configuration::getInstance().getStartAreaColor().toImU32();
		drawList->AddRectFilled(cursorPos, cursorPos + sqSize * Vec2{columns, 2}, startAreaColor);

		auto outerColor = Configuration::getInstance().getOuterSquareColor().toImU32();
		drawList->AddRectFilled(cursorPos, cursorPos + sqSize * Vec2{columns, rows}, outerColor);
		
		auto innerColor = Configuration::getInstance().getInnerSquareColor().toImU32();
		for (int i = 0; i < rows; ++i) {
			for (int j = 0; j < columns; ++j) {
				Vec2 pos = cursorPos + Vec2{j + 0.1f, i + 0.1f} * sqSize;
				drawList->AddRectFilled(pos, pos + sqSize * 0.8f, innerColor);
			}
		}
	}

	void ImGuiBoard::drawPreviewBlock(tetris::BlockType type, Color color) {
		auto drawList = ImGui::GetWindowDrawList();
		Vec2 pos = ImGui::GetCursorScreenPos();

		Vec2 sqSize{squareSize_ * NormalizedPreviewSize , squareSize_ * NormalizedPreviewSize};
		drawList->AddRectFilled(pos + sqSize * 0.05f, pos + sqSize * 0.95f, color.toImU32());
		drawBlock(tetris::Block{playerBoard_->getNextBlockType(), 0, 0}, pos + squareSize_, true);

		ImGui::Dummy(sqSize);
	}

	void ImGuiBoard::drawBoard(double deltaTime) {
		const int columns = playerBoard_->getColumns();
		const int rows = playerBoard_->getRows();
		
		drawGrid(columns, rows - 2);
		drawBoardSquares(deltaTime);
		
		Vec2 cursorPos = ImGui::GetCursorScreenPos();
		drawBlock(playerBoard_->getBlockDown(), cursorPos, false, Color(1.f, 1.f, 1, 0.3f));
		drawBlock(playerBoard_->getBlock(), cursorPos);

		ImGui::Dummy({squareSize_ * columns, squareSize_ * (rows - 2)});
	}

	void ImGuiBoard::drawBorder(float width, float height, Color color) {
		auto drawList = ImGui::GetWindowDrawList();
		Vec2 pos = ImGui::GetCursorScreenPos();
		
		// Left side.
		drawList->AddRectFilled(pos, pos + Vec2{borderSize_, height}, color.toImU32());

		// Right side.
		drawList->AddRectFilled(pos + Vec2{width - borderSize_, 0.f}, pos + Vec2{width, height}, color.toImU32());

		// Up side.
		drawList->AddRectFilled(pos, pos + Vec2{width, borderSize_}, color.toImU32());

		// Down side.
		drawList->AddRectFilled(pos + Vec2{0.f, height - borderSize_}, pos + Vec2{width, height}, color.toImU32());
	}

	void ImGuiBoard::drawBoardSquares(double deltaTime) {
		Vec2 cursorPos = ImGui::GetCursorScreenPos();

		auto drawList = ImGui::GetWindowDrawList();

		rows_.resize(playerBoard_->getRows() - 2);

		for (int i = 0; i < playerBoard_->getRows() - 2; ++i) {
			rows_[i] -= 5.0 * deltaTime;
			if (rows_[i] < 0) {
				rows_[i] = 0.0;
			}
			for (int j = 0; j < playerBoard_->getColumns(); ++j) {
				float x = squareSize_ * j + cursorPos.x;
				float y = height_ - squareSize_ * (i + 1) + cursorPos.y - rows_[i] * squareSize_;

				auto blockType = playerBoard_->getBlockType(j, i);
				if (blockType != tetris::BlockType::Empty
					&& blockType != tetris::BlockType::Wall) {

					auto texture = getSprite(blockType);
					drawList->PushTextureID((ImTextureID)(intptr_t) texture);
					
					drawList->PrimReserve(6, 4);
					ImGui::Helper::AddImageQuad(texture, {x, y}, Vec2{squareSize_, squareSize_});
					drawList->PopTextureID();
				}
			}
		}
	}

	void ImGuiBoard::draw(float width, float height, double deltaTime) {
		ImGui::Group([&]() {
			const int columns = playerBoard_->getColumns();
			const int rows = playerBoard_->getRows() - 2;

			float normalizedWidth = columns + NormalizedPreviewSize;
			float normalizedHeight = rows;

			bool blankSideUpAndDown = (width / normalizedWidth) < (height / normalizedHeight);

			const Vec2 startPosition = ImGui::GetCursorPos();

			if (blankSideUpAndDown) {
				squareSize_ = width / normalizedWidth;
				height_ = rows * squareSize_;

				ImGui::Dummy({width, (height - height_) * 0.5f});
			} else {
				height_ = height;
				squareSize_ = height_ / normalizedHeight;

				ImGui::Dummy({(width - normalizedWidth * squareSize_) * 0.5f, height});
				ImGui::SameLine();
			}

			drawBorder(normalizedWidth * squareSize_, normalizedHeight * squareSize_, color::Red);
			drawBoard(deltaTime);

			ImGui::SameLine();

			ImGui::Group([&]() {
				drawPreviewBlock(playerBoard_->getNextBlockType(), Color(1.f, 1.f, 1, 0.3f));

				ImGui::Indent(10.f);

				ImGui::Text("%s:", playerBoard_->getName().c_str());
				
				std::visit([&](auto&& playerData) mutable {
					using T = std::decay_t<decltype(playerData)>;
					if constexpr (std::is_same_v<T, game::DefaultPlayerData>) {
						ImGui::Text("%s: %d", "Level", playerData.level);
						ImGui::Text("%s: %d", "Points", playerData.points);
					} else if constexpr (std::is_same_v<T, game::SurvivalPlayerData>) {
						ImGui::Text("%s: %d", "OpponentRows", playerData.opponentRows);
					} else {
						static_assert(always_false_v<T>, "non-exhaustive visitor!");
					}
				}, playerBoard_->getPlayerData());
				ImGui::Text("%s: %d", "Rows", playerBoard_->getClearedRows());
			});


			if (blankSideUpAndDown) {
				ImGui::Dummy({width, (height - height_) * 0.5f});
			} else {
				ImGui::SameLine();
				ImGui::Dummy({(width - normalizedWidth * squareSize_) * 0.5f, height});
			}

			if (playerBoard_->isGameOver()) {
				auto pos = ImGui::GetCursorPos();

				const char* text = "Game Over";
				Vec2 delta = ImGui::CalcTextSize(text);

				ImGui::SetCursorPos(startPosition + Vec2{width * 0.5f, height * 0.5f} - delta * 0.5f);
				ImGui::Text("%s", text);
				
				ImGui::SetCursorPos(pos);
			}
		});
	}

	const game::PlayerBoard& ImGuiBoard::getPlayerBoard() const {
		return *playerBoard_;
	}

	sdl::TextureView ImGuiBoard::getSprite(tetris::BlockType blockType) const {
		switch (blockType) {
		case tetris::BlockType::I:
			return spriteI_;
		case tetris::BlockType::J:
			return spriteJ_;
		case tetris::BlockType::L:
			return spriteL_;
		case tetris::BlockType::O:
			return spriteO_;
		case tetris::BlockType::S:
			return spriteS_;
		case tetris::BlockType::T:
			return spriteT_;
		case tetris::BlockType::Z:
			return spriteZ_;
		case tetris::BlockType::Empty:
			[[fallthrough]];
		case tetris::BlockType::Wall:
			break;
		}
		return spriteI_;
	}

}
