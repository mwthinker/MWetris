#ifndef MWETRIS_GAME_DEFAULTGAMERULES_H
#define MWETRIS_GAME_DEFAULTGAMERULES_H

#include "tetrisboard.h"
#include "localplayerboard.h"
#include "player.h"
#include "remoteplayer.h"

#include <vector>
#include <span>

namespace mwetris::game {

	constexpr int LevelUpNbr = 1;

	class GameRules {
	public:
		virtual ~GameRules() = default;

		virtual void update(double deltaTime) = 0;

		virtual void restart() = 0;

		virtual void createGame(std::span<PlayerPtr> players) = 0;
	};

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
				info.player->updateGravity(1 + info.data.level * 0.5f);
				connections_ += player->addEventCallback([&info, this](tetris::BoardEvent gameEvent, int value) {
					updateGameBoardEvent(gameEvent, value, info);
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
				info.data.level = info.player->getPlayerBoard()->getClearedRows() / LevelUpNbr + 1;
				info.data.points += info.data.level * value * value;

				info.player->updatePlayerData(info.data);
				info.player->updateGravity(1 + info.data.level * 0.5f);
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
				connections_ += player->addEventCallback([board = info.player->getPlayerBoard(), this](tetris::BoardEvent gameEvent, int value) {
					updateGameBoardEvent(gameEvent, value, board);
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
		void updateGameBoardEvent(tetris::BoardEvent gameEvent, int value, const PlayerBoardPtr& playerBoard) {
			switch (gameEvent) {
				case tetris::BoardEvent::RowsRemoved:
					rowsRemove(playerBoard, value);
					break;
			}
		}

		void rowsRemove(const PlayerBoardPtr& fromPlayerBoard, int nbr) {
			for (auto& info : localPlayers_) {
				if (info.player->getPlayerBoard() != fromPlayerBoard) {
					info.data.opponentRows += nbr;

					if (nbr == 1 && !info.player->getPlayerBoard()->isGameOver()) {
						info.player->addRowWithHoles(2);
					}
				}
			}
		}

		struct Info {
			PlayerPtr player;
			SurvivalPlayerData data;
		};

		std::vector<Info> localPlayers_;
		std::vector<RemotePlayer> remotePlayers_;
		mw::signals::ScopedConnections connections_;
	};
}

#endif
