#include "gamecomponent.h"
#include "configuration.h"

#include "game/tetrisgame.h"
#include "game/tetrisparameters.h"
#include "game/tetrisgameevent.h"

#include <fmt/format.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>

#include <queue>
#include <map>
#include <cassert>
#include <utility>
#include <algorithm>
#include <limits>

namespace app::graphic {
	namespace {
		constexpr float PlayerCardSpacing = 12.f;
		constexpr float PauseOverlayMinFontSize = 36.f;
		constexpr float PauseOverlayMaxFontSize = 64.f;

		void drawPauseOverlay(const ImGuiBoard& board, const game::GamePause& pause) {
			const auto text = pause.countDown > 0 ? fmt::format("{}", pause.countDown) : std::string{"PAUSED"};
			const auto boardSize = board.getBoardSize();
			float fontSize = std::clamp(boardSize.x * 0.16f,
				PauseOverlayMinFontSize, PauseOverlayMaxFontSize);
			auto font = Configuration::getInstance().getImGuiScoreFont();
			Vec2 textSize = font->CalcTextSizeA(fontSize, std::numeric_limits<float>::max(), 0.f, text.c_str());
			const float initialPanelWidth = textSize.x + fontSize * 0.9f;
			if (initialPanelWidth > boardSize.x * 0.9f) {
				fontSize *= boardSize.x * 0.9f / initialPanelWidth;
				textSize = font->CalcTextSizeA(fontSize, std::numeric_limits<float>::max(), 0.f, text.c_str());
			}
			const Vec2 padding{fontSize * 0.45f, fontSize * 0.3f};
			const Vec2 center = board.getBoardCenter();
			const Vec2 panelMin = center - textSize * 0.5f - padding;
			const Vec2 panelMax = center + textSize * 0.5f + padding;
			const Vec2 shadowOffset{4.f, 4.f};
			auto drawList = ImGui::GetForegroundDrawList();
			auto& configuration = Configuration::getInstance();

			drawList->AddRectFilled(panelMin + shadowOffset, panelMax + shadowOffset,
				sdl::Color{0.f, 0.f, 0.f, 0.4f}.toImU32(), 10.f);
			drawList->AddRectFilled(panelMin, panelMax,
				configuration.getPreviewBackgroundColor().toImU32(), 10.f);
			drawList->AddRect(panelMin, panelMax,
				configuration.getPlayerCardBorderColor().toImU32(), 10.f, 1.5f, ImDrawFlags_None);
			drawList->AddText(font, fontSize, center - textSize * 0.5f,
				(pause.countDown > 0
					? configuration.getPlayerCardBorderColor()
					: configuration.getPlayerCardTextColor()).toImU32(),
				text.c_str());
		}
	}

	GameComponent::GameComponent(BoardSidebarRenderer boardSidebarRenderer)
		: boardSidebarRenderer_{std::move(boardSidebarRenderer)} {
	}

	GameComponent::~GameComponent() {}

	void GameComponent::draw(int windowWidth, int windowHeight, double deltaTime) {
		const auto cursorPos = ImGui::GetCursorPos();

		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, Vec2{0.f, 0.f});
		ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, Vec2{0.f, 0.f});

		float width = windowWidth;
		if (imguiBoards_.size() > 1) {
			width = windowWidth / imguiBoards_.size();
		}
		const float cardWidth = std::max(1.f, width - 2.f * PlayerCardSpacing);
		const float cardHeight = std::max(1.f, windowHeight - 2.f * PlayerCardSpacing);

		const Vec2 pos = ImGui::GetCursorPos();
		ImGui::SetCursorPos(pos + Vec2{PlayerCardSpacing, PlayerCardSpacing});
		for (std::size_t i = 0; i < imguiBoards_.size(); ++i) {
			imguiBoards_[i].draw(cardWidth, cardHeight, deltaTime);
			if (i + 1 < imguiBoards_.size()) {
				ImGui::SameLine(0.f, 2.f * PlayerCardSpacing);
			}
		}

		if (gamePause_.pause) {
			for (const auto& imguiBoard : imguiBoards_) {
				drawPauseOverlay(imguiBoard, gamePause_);
			}
		}

		ImGui::PopStyleVar(2);

		ImGui::SetCursorPos(cursorPos);
		ImGui::Dummy({static_cast<float>(windowWidth), static_cast<float>(windowHeight)});
	}

	void GameComponent::initGame(const std::vector<game::PlayerPtr>& players) {
		imguiBoards_.clear();
		int playerNumber = 1;
		for (auto& player : players) {
			imguiBoards_.emplace_back(player, playerNumber++, boardSidebarRenderer_);
		}
	}

	void GameComponent::gamePause(const game::GamePause& gamePause) {
		gamePause_ = gamePause;
	}

}
