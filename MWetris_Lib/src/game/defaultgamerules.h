#ifndef MWETRIS_GAME_DEFAULTGAMERULES_H
#define MWETRIS_GAME_DEFAULTGAMERULES_H

#include "tetrisboard.h"
#include "player.h"
#include "gamerules.h"

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
			restart();
		}

		void update(double deltaTime) override {

		}
		
		void createGame(std::span<PlayerPtr> players) override {
			connections_.clear();
			localPlayers_.clear();
			for (auto& player : players) {
				auto& info = localPlayers_.emplace_back(player, DefaultPlayerData{});
				info.player->updatePlayerData(info.data);
				info.player->setGravity(1 + info.data.level * 0.5f);
				connections_ += player->playerBoardUpdate.connect([&info, this](const PlayerBoardEvent& playerBoardEvent) {
					if (auto value = std::get_if<TetrisBoardEvent>(&playerBoardEvent)) {
						updateGameBoardEvent(value->event, value->value, info);
					}
				});
			}
		}

		void restart() override {
			localPlayers_.clear();
		}

	private:
		struct Info {
			PlayerPtr player;
			DefaultPlayerData data;
		};

		void updateGameBoardEvent(tetris::BoardEvent gameEvent, int value, Info& info) {
			if (gameEvent == tetris::BoardEvent::RowsRemoved) {
				info.data.level = info.player->getClearedRows() / LevelUpNbr + 1;
				info.data.points += info.data.level * value * value;

				info.player->updatePlayerData(info.data);
				info.player->setGravity(1 + info.data.level * 0.5f);
			}
		}

		std::vector<Info> localPlayers_;
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
			localPlayers_.clear();
			for (auto& player : players) {
				auto& info = localPlayers_.emplace_back(player, SurvivalPlayerData{});
				info.player->updatePlayerData(info.data);
				connections_ += player->playerBoardUpdate.connect([&info, player, this](const PlayerBoardEvent& playerBoardEvent) {
					if (auto value = std::get_if<TetrisBoardEvent>(&playerBoardEvent)) {
						updateGameBoardEvent(value->event, value->value, player);
					}
				});
			}
		}

		void restart() override {
			for (auto& info : localPlayers_) {
				info.data.opponentRows = 0;
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
			for (auto& info : localPlayers_) {
				if (info.player != fromPlayer) {
					info.data.opponentRows += nbr;

					if (nbr == 1 && !info.player->isGameOver()) {
						//info.player->addRowWithHoles(2);
					}
				}
			}
		}

		struct Info {
			PlayerPtr player;
			SurvivalPlayerData data;
		};

		std::vector<Info> localPlayers_;
		std::vector<PlayerPtr> remotePlayers_;
		mw::signals::ScopedConnections connections_;
	};
}

#endif
