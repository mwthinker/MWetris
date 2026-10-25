#ifndef APP_GAME_DEFAULTGAMERULES_H
#define APP_GAME_DEFAULTGAMERULES_H

#include "player.h"
#include "gamerules.h"

#include <tetris/helper.h>
#include <tetris/tetrisboard.h>

#include <spdlog/spdlog.h>

#include <algorithm>
#include <array>
#include <vector>
#include <variant>

namespace app::game {

	inline int calculateLineClearPoints(int rows, int level) {
		static constexpr std::array LineClearPoints{0, 40, 100, 300, 1200};
		return LineClearPoints.at(rows) * (level + 1);
	}

	inline double calculateGravity(int level) {
		static constexpr std::array GravityFrames{
			48, 43, 38, 33, 28, 23, 18, 13, 8, 6,
			5, 5, 5,
			4, 4, 4,
			3, 3, 3,
			2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
			1
		};
		constexpr double FramesPerSecond{60.0988};
		const int gravityLevel{std::clamp(level, 0, static_cast<int>(GravityFrames.size()) - 1)};
		return FramesPerSecond / GravityFrames.at(gravityLevel);
	}

	class DefaultGameRules : public GameRules {
	public:
		struct Config {};

		void createGame(const std::vector<PlayerPtr>& players) override {
			connections_.clear();
			players_.clear();
			for (auto& player : players) {
				auto& info = players_.emplace_back(player, DefaultPlayerData{
					.level = 0,
					.points = 0,
					.position = 0
				});
				info.player->updatePlayerData(info.data);
				info.player->setGravity(calculateGravity(info.data.level));

				connections_ += player->playerBoardUpdate.connect([this, index = players_.size() - 1](const PlayerBoardEvent& playerBoardEvent) {
					if (index < 0 || index >= players_.size()) {
						spdlog::error("[DefaultGameRules] Index {} is out of bounds.", index);
						return;
					}
					if (auto value = std::get_if<TetrisBoardEvent>(&playerBoardEvent)) {
						auto& playerInfo = players_[index];
						updateGameBoardEvent(value->event, value->value, playerInfo);
					}
				});
			}
		}

		void restart() override {
			for (auto& info : players_) {
				info.data = DefaultPlayerData{
					.level = 0,
					.points = 0,
					.position = 0
				};
				info.player->updatePlayerData(info.data);
				info.player->setGravity(calculateGravity(info.data.level));
			}
		}

	private:
		struct Info {
			PlayerPtr player;
			DefaultPlayerData data;
		};

		void updateGameBoardEvent(tetris::BoardEvent gameEvent, int value, Info& info) {
			switch (gameEvent) {
				case tetris::BoardEvent::RowsRemoved:
					rowsRemove(info.player, value, info);
					break;
				case tetris::BoardEvent::GameOver:
					gameOver(info.player);
					break;
			}
		}

		void rowsRemove(const PlayerPtr& fromPlayer, int nbr, Info& info) {
			if (info.player->isLocal()) {
				spdlog::info("[DefaultGameRules] Game event player: {}", static_cast<char>(info.player->getNextBlockType()));
				const int clearedRows = info.player->getClearedRows() + nbr;
				info.data.points += calculateLineClearPoints(nbr, info.data.level);
				info.data.level = levelForClearedRows(clearedRows);

				info.player->updatePlayerData(info.data);
				info.player->setGravity(calculateGravity(info.data.level));
			}

		}

		void gameOver(const PlayerPtr& player) {
			int nbr = static_cast<int>(players_.size());
			for (auto& info : players_) {
				if (info.player->isGameOver()) {
					--nbr;
				}
			}

			if (players_.size() > 1) {
				for (auto& info : players_) {
					if (nbr == 1 && !info.player->isGameOver() && info.player->isLocal()) {
						info.player->updateMove(tetris::Move::GameOver);
						info.data.position = 1;
						info.player->updatePlayerData(info.data);
					}
					if (info.player == player) {
						info.data.position = nbr + 1;
						info.player->updatePlayerData(info.data);
					}
				}
			}
		}

		std::vector<Info> players_;
		mw::signals::ScopedConnections connections_;
	};

	class SurvivalGameRules : public GameRules {
	public:
		struct Config {};

		void createGame(const std::vector<PlayerPtr>& players) override {
			connections_.clear();
			players_.clear();
			for (auto& player : players) {
				auto& info = players_.emplace_back(player, SurvivalPlayerData{
					.opponentRows = 0
				});
				info.player->updatePlayerData(info.data);
				connections_ += player->playerBoardUpdate.connect([this, index = players_.size() - 1](const PlayerBoardEvent& playerBoardEvent) {
					if (index < 0 || index >= players_.size()) {
						spdlog::error("[DefaultGameRules] Index {} is out of bounds", index);
						return;
					}
					auto& info = players_[index];
					if (auto value = std::get_if<TetrisBoardEvent>(&playerBoardEvent)) {
						updateGameBoardEvent(value->event, value->value, info.player);
					}
				});
			}
		}

		void restart() override {
			for (auto& info : players_) {
				info.data = SurvivalPlayerData{
					.opponentRows = 0,
					.position = 0
				};
				info.player->updatePlayerData(info.data);
			}
		}

	private:
		void updateGameBoardEvent(tetris::BoardEvent gameEvent, int value, const PlayerPtr& player) {
			switch (gameEvent) {
				case tetris::BoardEvent::RowsRemoved:
					rowsRemove(player, value);
					break;
				case tetris::BoardEvent::GameOver:
					gameOver(player);
					break;
			}
		}

		void rowsRemove(const PlayerPtr& fromPlayer, int nbr) {
			for (auto& info : players_) {
				if (info.player == fromPlayer) {
					if (fromPlayer->isLocal()) {
						fromPlayer->setGravity(calculateGravity(info.data.opponentRows));
					}
				} else if (info.player->isLocal()) {
					info.data.opponentRows += nbr;
					info.player->updatePlayerData(info.data);
					if (nbr == 4 && !info.player->isGameOver()) {
						addExternalRows(*info.player, 2);
					}
				}
			}
		}

		void addExternalRows(Player& player, int rows) {
			for (int i = 0; i < rows; ++i) {
				auto blockTypes = tetris::generateRow(player.getColumns(), 2);
				player.addExternalRows(blockTypes);
			}
		}

		void gameOver(const PlayerPtr& player) {
			int nbr = static_cast<int>(players_.size());
			for (auto& info : players_) {
				if (info.player->isGameOver()) {
					--nbr;
				}
			}

			if (players_.size() > 1) {
				for (auto& info : players_) {
					if (nbr == 1 && !info.player->isGameOver() && info.player->isLocal()) {
						info.player->updateMove(tetris::Move::GameOver);
						info.data.position = 1;
						info.player->updatePlayerData(info.data);
					}
					if (info.player == player) {
						info.data.position = nbr + 1;
						info.player->updatePlayerData(info.data);
					}
				}
			}
		}

		struct Info {
			PlayerPtr player;
			SurvivalPlayerData data;
		};

		std::vector<Info> players_;
		mw::signals::ScopedConnections connections_;
	};

	enum class GameRoomType {
		Default,
		Survival
	};

	using GameRulesConfig = std::variant<DefaultGameRules::Config, SurvivalGameRules::Config>;

}

#endif
