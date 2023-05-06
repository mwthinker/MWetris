#include "localgame.h"
#include "tetrisgameevent.h"
#include "localplayerbuilder.h"
#include "helper.h"

#include <memory>

namespace {

	constexpr int LevelUpNbr = 10;

}

namespace mwetris::game {

	namespace {

		std::vector<LocalPlayerPtr> createLocalPlayers(int columns, int rows, const std::vector<DevicePtr>& devices) {
			std::vector<LocalPlayerPtr> players;
			
			LocalPlayerBuilder builder;
			for (const auto& device : devices) {
				builder.withDevice(device);
				builder.withClearedRows(0);
				builder.withGameOverPosition(0);
				builder.withLevel(1);
				builder.withLevelUpCounter(0);
				//builder.widthMovingBlock(data.current_);
				//builder.widthName(data.name_);
				builder.withMovingBlockType(tetris::randomBlockType());
				builder.withNextBlockType(tetris::randomBlockType());
				builder.withPoints(0);
				builder.withHeight(rows);
				builder.withWidth(columns);

				auto player = builder.build();
				players.push_back(player);
			}
			return players;
		}
	}

	void LocalGame::resume(const std::vector<LocalPlayerPtr>& players) {
		players_ = players;
		connections_.clear();

		for (auto& player : players_) {
			connections_ += player->gameboardEventUpdate.connect([this, p = player](tetris::BoardEvent gameEvent, int value) {
				applyRules(gameEvent, value, p);
			});
		}
	}

	std::vector<LocalPlayerPtr> LocalGame::create(int columns, int rows, const std::vector<DevicePtr>& devices) {
		connections_.clear();
		players_ = createLocalPlayers(columns, rows, devices);
		
		for (auto& player : players_) {
			connections_ += player->gameboardEventUpdate.connect([this, p = player](tetris::BoardEvent gameEvent, int value) {
				applyRules(gameEvent, value, p);
			});
		}

		return players_;
	}

	void LocalGame::restart() {
		connections_.clear();
		for (auto& player : players_) {
			connections_ += player->gameboardEventUpdate.connect([this, p = player](tetris::BoardEvent gameEvent, int value) {
				applyRules(gameEvent, value, p);
			});
			player->updateRestart();
		}
	}

	LocalGame::LocalGame() {
	}

	void LocalGame::applyRules(tetris::BoardEvent gameEvent, int value, const LocalPlayerPtr& player) {
		if (tetris::BoardEvent::RowsRemoved == gameEvent) {
			rows_ += value;
			level_ = rows_ / LevelUpNbr + 1;
			points_ += level_ * value * value;

			player->updateLevel(level_);
			player->updatePoints(points_);
		}
	}

}
