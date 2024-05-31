#ifndef MWETRIS_GAME_DEFAULTGAMERULES_H
#define MWETRIS_GAME_DEFAULTGAMERULES_H

#include "player.h"
#include "gamerules.h"

#include <tetris/helper.h>
#include <tetris/tetrisboard.h>

#include <spdlog/spdlog.h>

#include <vector>
#include <span>

namespace mwetris::game {

	constexpr int LevelUpNbr = 1;

	inline double calculateGravity(int level) {
		return 1 + level * 0.5f;
	}

	class DefaultGameRules : public GameRules {
	public:
		DefaultGameRules() {
		}

		void update(double deltaTime) override {

		}
		
		void createGame(std::span<PlayerPtr> players) override {
			connections_.clear();
			players_.clear();
			for (auto& player : players) {
				auto& info = players_.emplace_back(player, DefaultPlayerData{
					.level = 1,
					.points = 0
				});
				info.player->updatePlayerData(info.data);
				info.player->setGravity(calculateGravity(info.data.level));

				connections_ += player->playerBoardUpdate.connect([this, index = players_.size()](const PlayerBoardEvent& playerBoardEvent) {
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
					.level = 1,
					.points = 0
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
			spdlog::info("[DefaultGameRules] Game event player: {}", static_cast<char>(info.player->getNextBlockType()));
			if (info.player->isLocal() && gameEvent == tetris::BoardEvent::RowsRemoved) {
				info.data.level = info.player->getClearedRows() / LevelUpNbr + 1;
				info.data.points += info.data.level * value * value;

				info.player->updatePlayerData(info.data);
				info.player->setGravity(calculateGravity(info.data.level));
			}
		}

		std::vector<Info> players_;
		mw::signals::ScopedConnections connections_;
	};

	class SurvivalGameRules : public GameRules {
	public:
		SurvivalGameRules() {
		}

		void update(double timeStep) override {
		}

		void createGame(std::span<PlayerPtr> players) override {
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
					.opponentRows = 0
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
					if (nbr == 1 && !info.player->isGameOver()) {
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

		struct Info {
			PlayerPtr player;
			SurvivalPlayerData data;
		};

		std::vector<Info> players_;
		mw::signals::ScopedConnections connections_;
	};
}

#endif
