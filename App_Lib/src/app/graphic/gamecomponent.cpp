#include "gamecomponent.h"
#include "configuration.h"

#include "game/tetrisgame.h"
#include "game/tetrisparameters.h"
#include "game/tetrisgameevent.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>

#include <queue>
#include <map>
#include <cassert>
#include <utility>
#include <algorithm>

namespace app::graphic {
	namespace {
		constexpr float PlayerCardSpacing = 12.f;
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
			imguiBoards_[i].draw(cardWidth, cardHeight, deltaTime, gamePause_);
			if (i + 1 < imguiBoards_.size()) {
				ImGui::SameLine(0.f, 2.f * PlayerCardSpacing);
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
