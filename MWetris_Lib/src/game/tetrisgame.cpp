#include "tetrisgame.h"

#include "tetrisboard.h"
#include "helper.h"
#include "tetrisgameevent.h"
#include "localplayerboard.h"
#include "tetrisparameters.h"
#include "localplayerboardbuilder.h"
#include "serialize.h"
#include "defaultgamerules.h"

#include <vector>
#include <algorithm>

namespace mwetris::game {

	namespace {

		LocalPlayerBoardPtr createLocalPlayerBoard(int width, int height, const std::string& name) {
			return LocalPlayerBoardBuilder{}
				.withClearedRows(0)
				.withGameOverPosition(0)
				.withPlayerData(DefaultPlayerData{
					.level = 1,
					.points = 0
				})
				.withName(name)
				.withMovingBlockType(tetris::randomBlockType())
				.withNextBlockType(tetris::randomBlockType())
				.withWidth(width)
				.withHeight(height)
				.build();
		}

	}

	PlayerFactory::PlayerFactory() {}

	std::vector<PlayerPtr> PlayerFactory::createPlayers(int width, int height,
		const std::vector<game::Human>& humans,
		const std::vector<game::Ai>& ais) {

		std::vector<game::PlayerPtr> players;

		for (auto& human : humans) {
			auto board = createLocalPlayerBoard(width, height, human.name);
			auto player = std::make_shared<HumanPlayer>(human.device, createLocalPlayerBoard(TetrisWidth, TetrisHeight, "Player"));
			players.push_back(player);
		}
		for (auto& ai : ais) {
			auto localPlayer = createLocalPlayerBoard(width, height, ai.name);
			auto player = std::make_shared<AiPlayer>(ai.ai, createLocalPlayerBoard(TetrisWidth, TetrisHeight, "Player"));
			players.push_back(player);
		}

		return players;
	}

	TetrisGame::TetrisGame() {
	}

	TetrisGame::~TetrisGame() {
	}

	bool TetrisGame::isDefaultGame() const {
		if (players_.size() != 1) {
			return false;
		}

		const auto& player = *players_.front();

		if (player.getPlayerBoard()->getRows() != TetrisHeight) {
			return false;
		}

		if (player.getPlayerBoard()->getColumns() != TetrisWidth) {
			return false;
		}

		if (!dynamic_cast<DefaultGameRules*>(rules_.get())) {
			return false;
		}

		return dynamic_cast<const HumanPlayer*>(&player);
	}

	void TetrisGame::saveDefaultGame() {
		if (!isDefaultGame()) {
			return;
		}

		const auto& player = static_cast<HumanPlayer&>(*players_.front());
		if (!player.getPlayerBoard()->isGameOver()) {
			saveGame(*player.getPlayerBoard());
		}
	}

	void TetrisGame::createDefaultGame(DevicePtr device) {
		rules_ = std::make_unique<DefaultGameRules>();
		remotePlayers_.clear();
		saveDefaultGame();

		if (isDefaultGame() && players_.size() == 1 && players_.front()->getPlayerBoard()->isGameOver()) {
			restartGame();
		} else {
			LocalPlayerBoardPtr localPlayerBoard = loadGame();
			if (!localPlayerBoard) {
				localPlayerBoard = createLocalPlayerBoard(TetrisWidth, TetrisHeight, "Player");
			}
			auto player = std::make_shared<HumanPlayer>(device, localPlayerBoard);
			connections_.clear();
			players_ = {player};
			connections_ += player->addEventCallback([](tetris::BoardEvent gameEvent, int value) {
				game::clearSavedGame();
			});
			setPause(true);
			initGame();
		}
	}

	void TetrisGame::createGame(std::unique_ptr<GameRules> gameRules, int width, int height,
		const std::vector<PlayerPtr>& players,
		const std::vector<RemotePlayerPtr>& remotePlayers) {
		
		rules_ = std::move(gameRules);

		saveDefaultGame();
		remotePlayers_.clear();

		players_ = players;
		remotePlayers_ = remotePlayers;

		rules_->createGame(players_);

		initGame();
	}

	void TetrisGame::initGame() {
		std::vector<PlayerBoardPtr> playerBoards;
		for (auto& player : players_) {
			playerBoards.push_back(player->getPlayerBoard());
		}
		for (auto& player : remotePlayers_) {
			playerBoards.push_back(player->getPlayerBoard());
		}

		initGameEvent.invoke(InitGameEvent{playerBoards.begin(), playerBoards.end()});
	}

	void TetrisGame::restartGame() {
		accumulator_ = 0.0;
		setPause(false);
		
		auto current = tetris::randomBlockType();
		auto next = tetris::randomBlockType();

		for (auto& player : players_) {
			player->updateRestart(current, next);
		}
		initGame();
		rules_->restart();
	}

	bool TetrisGame::isPaused() const {
		return pause_;
	}

	void TetrisGame::setPause(bool pause) {
		timeHandler_.removeCallback(pauseKey_);
		pause_ = pause;
		gamePauseEvent(GamePause{
			.countDown = 0,
			.pause = pause
		});
	}

	void TetrisGame::pause() {
		if (isDefaultGame() && players_.size() == 1 && !players_.front()->getPlayerBoard()->isGameOver()) {
			saveGame(*players_.front()->getPlayerBoard());
		}
		if (timeHandler_.removeCallback(pauseKey_) || !pause_) {
			gamePauseEvent(GamePause{
				.countDown = 0,
				.pause = true
			});
			pause_ = true;
		} else {
			gamePauseEvent(GamePause{
				.countDown = 3,
				.pause = true
			});
			pauseKey_ = timeHandler_.scheduleRepeat([&, nbr = 2]() mutable {
				if (nbr == 0) {
					pause_ = false;
					gamePauseEvent(GamePause{
						.countDown = 0,
						.pause = false
					});
				} else {
					gamePauseEvent(GamePause{
						.countDown = nbr--,
						.pause = true
					});
				}
			}, 1.0, 3);
		}
	}

	int TetrisGame::getNbrOfPlayers() const {
		return static_cast<int>(players_.size());
	}

	void TetrisGame::update(double deltaTime) {
		rules_->update(deltaTime);
		timeHandler_.update(deltaTime);

		if (!pause_) {
			updateGame(deltaTime);
		}
	}

	void TetrisGame::updateGame(double deltaTime) {
		if (deltaTime > 0.250) {
			// To avoid spiral of death.
			deltaTime = 0.250;
		}

		accumulator_ += deltaTime;
		while (accumulator_ >= fixedTimestep) {
			accumulator_ -= fixedTimestep;
			for (auto& player : players_) {
				player->update(fixedTimestep);
			}
		}
	}

}
