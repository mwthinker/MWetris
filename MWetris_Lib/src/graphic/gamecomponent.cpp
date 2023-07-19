#include "gamecomponent.h"
#include "configuration.h"
#include "ui/imguiextra.h"

#include "game/tetrisgame.h"
#include "game/tetrisparameters.h"
#include "game/tetrisgameevent.h"

#include <sdl/opengl.h>

#include <fmt/format.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>

#include <queue>
#include <map>
#include <cassert>

namespace mwetris::graphic {

	namespace {

		std::string gamePosition(int position) {
			switch (position) {
				case 1:
					return fmt::format("{}:st place!", position);
				case 2:
					return fmt::format("{}:nd place!", position);
				case 3:
					return fmt::format("{}:rd place!", position);
				default:
					return fmt::format("{}:th place!", position);
			}
		}

	}

	GameComponent::GameComponent() {}

	GameComponent::~GameComponent() {}

	void GameComponent::draw(int windowWidth, int windowHeight, double deltaTime) {
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, Vec2{0.f, 0.f});
		ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, Vec2{0.f, 0.f});

		ImGui::PushStyleColor(ImGuiCol_Border, sdl::color::html::Red);

		float width = windowWidth;
		float height = windowHeight;
		if (imguiBoards_.size() > 1) {
			width = windowWidth / imguiBoards_.size();
		}

		//ImGui::SetNextWindowSize({width, height}); // FIX! Will effect main menu size.

		Vec2 pos = ImGui::GetCursorPos();
		for (auto& imguiBoard : imguiBoards_) {
			imguiBoard.draw(width, height, deltaTime);
			ImGui::SameLine();
		}

		for (auto& imguiBoard : imguiBoards_) {
			ImGui::SetCursorPos(pos + Vec2{width / 2.f, height / 2.f});
			pos.x += width;
			ImGui::PushFont(Configuration::getInstance().getImGuiHeaderFont());
			if (paused_) {
				ImGui::TextWithBackgroundColor("Paus", sdl::Color{0.f, 0.f, 0.f ,0.5f}, {5.f, 5.f});
			} else if (timeLeft_ > 0) {
				ImGui::TextWithBackgroundColor(timeLeft_, sdl::Color{0.f, 0.f, 0.f ,0.5f}, {5.f, 5.f});
			}
			ImGui::PopFont();
		}

		ImGui::PopStyleColor(1);
		ImGui::PopStyleVar(2);
	}

	void GameComponent::initGame(const game::InitGameEvent& event) {
		imguiBoards_.clear();
		for (auto& player : event.players) {
			imguiBoards_.emplace_back(player);
		}
	}

	void GameComponent::gamePause(const game::GamePause& event) {
		paused_ = event.pause;
	}

	void GameComponent::countDown(const game::CountDown& countDown) {
		timeLeft_ = countDown.timeLeft;
	}

}