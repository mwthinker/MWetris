#include "gamecomponent.h"
#include "../game/tetrisgame.h"
#include "gamegraphic.h"
#include "../game/tetrisparameters.h"
#include "../game/tetrisgameevent.h"
#include "../tetrisdata.h"
#include <fmt/format.h>

#include <sdl/opengl.h>

#include <queue>
#include <map>
#include <cassert>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>

namespace tetris {

	namespace {

		std::string gamePosition(int position) {
			switch (position) {
			case 1:
				return fmt::format("{}:st place!", position);
			case 2:
				return fmt::format("{}:nd place!", position);
			case 3:
				return fmt::format("{}:rd place!", position);
			}
			return fmt::format("{}:th place!", position);
		}

	}

	GameComponent::GameComponent(TetrisGame& tetrisGame)
		: tetrisGame_{tetrisGame} {

		eventConnection_ = tetrisGame_.addGameEventHandler([&](TetrisGameEvent& tetrisEvent) {
			eventHandler(tetrisEvent);
		});
	}

	GameComponent::~GameComponent() {
		eventConnection_.disconnect();
	}

	Mat4 GameComponent::calculateBoardMatrix(int windowWidth, int windowHeight) const {
		float width = 0;
		float height = 0;

		for (const auto& [player, drawBoardPtr] : drawPlayers_) {
			auto size = drawBoardPtr->getSize();
			width += size.x;
			height = size.y;
		}
		
		float dx{}, dy{};
		float scale{};
		if (width / windowWidth > height / windowHeight) {
			// Blank sides, up and down.
			scale = windowWidth / width;
			dx = 0;
			dy = (windowHeight - scale * height) * 0.5f;
		} else {
			// Blank sides, left and right.
			scale = windowHeight / height;
			dx = (windowWidth - scale * width) * 0.5f;
			dy = 0;
		}

		return glm::translate(Vec3{dx, dy, 0.f}) * glm::scale(Vec3{scale, scale, 1.f});
	}

	void GameComponent::draw(Graphic& graphic, int windowWidth, int windowHeight, double deltaTime) {
		graphic.pushMatrix();
		graphic.multMatrix(calculateBoardMatrix(windowWidth, windowHeight));

		if (!drawPlayers_.empty()) {
			TetrisData::getInstance().bindTextureFromAtlas();
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			
			auto model = graphic.currentMatrix();
			float delta = 0;
			for (auto& [player, drawBoardPtr] : drawPlayers_) {
				model = glm::translate(model, Vec3{delta, 0.f, 0.f});
				graphic.pushMatrix(model);
				drawBoardPtr->draw(graphic);
				delta += drawBoardPtr->getSize().x;
			}

			sdl::assertGlError();
		}
	}

	void GameComponent::initGame(const std::vector<PlayerPtr>& players) {
		bool showPoints = false;
		if (players.size() == 1) {
			showPoints = true;
		}

		//staticBoardBatch_ = std::make_shared<BoardBatch>(boardShader_);
		drawPlayers_.clear();

		float w = 0;
		for (auto& player : players) {
			auto& drawBoardPtr = drawPlayers_[player];
			if (drawBoardPtr == nullptr) {
				drawBoardPtr = std::make_unique<DrawBoard>(*player);
			}
			//graphic.restart(*staticBoardBatch_, *player, w, 0, tetrisGame_.isDefaultGame());
			w += drawBoardPtr->getSize().x;
		}
		//staticBoardBatch_->uploadToGraphicCard();
	}

	void GameComponent::eventHandler(TetrisGameEvent& tetrisEvent) {
		// Handle CountDown event.
		try {
			auto& countDown = dynamic_cast<CountDown&>(tetrisEvent);

			if (countDown.timeLeft_ > 0) {
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
			auto& gamePause = dynamic_cast<GamePause&>(tetrisEvent);

			if (gamePause.printPause_) {
				if (!gamePause.pause_) {
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
			//middleText_ = sdl::Text("", TetrisData::getInstance().getDefaultFont(50), 20);
			auto& initGameVar = dynamic_cast<InitGame&>(tetrisEvent);
			initGame(initGameVar.players_);

			for (const PlayerPtr& player : initGameVar.players_) {
				if (player->isGameOver()) {
					handleMiddleText(player, player->getGameOverPosition());
				}
			}

			return;
		} catch (std::bad_cast&) {}

		// Handle RestartPlayer event.
		try {
			/*
			middleText_ = mw::Text("", TetrisData::getInstance().getDefaultFont(50), 20);
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
		} catch (std::bad_cast&) {}

		// Handle LevelChange event.
		try {
			auto& levelChange = dynamic_cast<LevelChange&>(tetrisEvent);
			//GameGraphic& gg = drawPlayers_[levelChange.player_];
			//gg.update(levelChange.player_->getClearedRows(), levelChange.player_->getPoints(), levelChange.newLevel_);
			return;
		} catch (std::bad_cast&) {}

		// Handle PointsChange event.
		try {
			auto& pointsChange = dynamic_cast<PointsChange&>(tetrisEvent);
			//GameGraphic& gg = graphicPlayers_[pointsChange.player_];
			//gg.update(pointsChange.player_->getClearedRows(), pointsChange.player_->getPoints(), pointsChange.player_->getLevel());
			return;
		} catch (std::bad_cast&) {}

		// Handle GameOver event.
		try {
			auto& gameOver = dynamic_cast<GameOver&>(tetrisEvent);
			handleMiddleText(gameOver.player_, gameOver.player_->getGameOverPosition());

			return;
		} catch (std::bad_cast&) {}
	}

	void GameComponent::handleMiddleText(const PlayerPtr& player, int lastPostion) {
		//sdl::Text middleText("", TetrisData::getInstance().getDefaultFont(50), 20);

		// Test if the player is a local player, exception otherwise.
		if (tetrisGame_.getNbrOfPlayers() == 1) {
			//middleText.setText("Game over");
		} else {
			//middleText.setText(gamePosition(lastPostion));
		}

		//graphicPlayers_[player].setMiddleMessage(middleText);
	}

} // Namespace tetris.
