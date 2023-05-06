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

	GameComponent::GameComponent() {
	}

	GameComponent::~GameComponent() {
	}

	void GameComponent::draw(int windowWidth, int windowHeight, double deltaTime) {
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, Vec2{0.f, 0.f});
		ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, Vec2{0.f, 0.f});

		ImGui::PushStyleColor(ImGuiCol_Border, sdl::color::html::Red);

		float width = windowWidth;
		float height = windowHeight;
		if (players_.size() > 1) {
			width = windowWidth / players_.size();
		}

		ImGui::SetNextWindowSize({width, height});

		Vec2 pos = ImGui::GetCursorPos();
		for (auto& [player, imguiBoard] : players_) {
			imguiBoard.draw(width, height);
			ImGui::SameLine();
		}

		for (auto& [player, imguiBoard] : players_) {
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
		players_.clear();
		for (auto& player : event.players) {
			players_.insert({player, ImGuiBoard{player}});
		}
	}

	void GameComponent::gamePause(const game::GamePause& event) {
		paused_ = event.pause;
	}

	void GameComponent::countDown(const game::CountDown& countDown) {
		timeLeft_ = countDown.timeLeft;
	}

	//void GameComponent::eventHandler(Event& tetrisEvent) {
		/*
		try {
			auto& gameBoardEvent = dynamic_cast<game::GameBoardEvent&>(tetrisEvent);
			return;
		} catch (std::bad_cast&) {}


		// Handle CountDown event.
		try {
			auto& countDown = dynamic_cast<game::CountDown&>(tetrisEvent);

			if (countDown.timeLeft > 0) {
				//middleText_.setText("Start in " + std::to_string(countDown.timeLeft_));
			} else {
				//middleText_.setText("");
			}

			// Update the text for the active players.
			for (auto& graphic : drawPlayers_) {
				if (!graphic.first->getTetrisBoard().isGameOver()) {
					//graphic.second.setMiddleMessage(middleText_);
				}
			}
			return;
		} catch (std::bad_cast&) {}

		// Handle GamePause event.
		try {
			auto& gamePause = dynamic_cast<game::GamePause&>(tetrisEvent);

			if (gamePause.printPause) {
				if (!gamePause.pause) {
					//middleText_.setText("");
				} else {
					//middleText_.setText("Paused");
				}

				// Update the text for the active players.
				for (auto& graphic : drawPlayers_) {
					if (!graphic.first->isGameOver()) {
						//graphic.second.setMiddleMessage(middleText_);
					}
				}
			}
			return;
		} catch (std::bad_cast&) {}

		// Handle InitGame event.
		try {
			//middleText_ = sdl::Text("", Configuration::getInstance().getDefaultFont(50), 20);
			auto& initGameVar = dynamic_cast<game::InitGame&>(tetrisEvent);
			initGame(initGameVar.players);

			for (const auto& player : initGameVar.players) {
				if (player->isGameOver()) {
					handleMiddleText(player, player->getGameOverPosition());
				}
			}

			return;
		} catch (std::bad_cast&) {}
		*/
		// Handle RestartPlayer event.
	//	try {
			/*
			middleText_ = mw::Text("", Configuration::getInstance().getDefaultFont(50), 20);
			auto& restartPlayer = dynamic_cast<RestartPlayer&>(tetrisEvent);

			for (auto& player : *restartPlayer.connection_) {

				if (player->isGameOver()) {
					//handleMiddleText(player, player->getLastPositon());
				}

				//GameGraphic& graphic = graphicPlayers_[player];
				//graphic.restart(*player);
			}

			return;
			*/
	//	} catch (std::bad_cast&) {}

		/*
		// Handle LevelChange event.
		try {
			auto& levelChange = dynamic_cast<game::LevelChange&>(tetrisEvent);
			//GameGraphic& gg = drawPlayers_[levelChange.player_];
			//gg.update(levelChange.player_->getClearedRows(), levelChange.player_->getPoints(), levelChange.newLevel_);
			return;
		} catch (std::bad_cast&) {}

		// Handle PointsChange event.
		try {
			auto& pointsChange = dynamic_cast<game::PointsChange&>(tetrisEvent);
			//GameGraphic& gg = graphicPlayers_[pointsChange.player_];
			//gg.update(pointsChange.player_->getClearedRows(), pointsChange.player_->getPoints(), pointsChange.player_->getLevel());
			return;
		} catch (std::bad_cast&) {}

		// Handle GameOver event.
		try {
			auto& gameOver = dynamic_cast<game::GameOver&>(tetrisEvent);
			handleMiddleText(gameOver.player, gameOver.player->getGameOverPosition());

			return;
		} catch (std::bad_cast&) {}
		*/
	//}

	//void GameComponent::handleMiddleText(const game::PlayerPtr& player, int lastPostion) {
		//sdl::Text middleText("", Configuration::getInstance().getDefaultFont(50), 20);

		// Test if the player is a local player, exception otherwise.
		//if (tetrisGame_.getNbrOfPlayers() == 1) {
			//middleText.setText("Game over");
		//} else {
			//middleText.setText(gamePosition(lastPostion));
		//}

		//graphicPlayers_[player].setMiddleMessage(middleText);
	//}

}
