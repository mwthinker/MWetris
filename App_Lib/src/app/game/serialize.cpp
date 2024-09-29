#include "serialize.h"
#include "tetrisgame.h"
#include "player.h"

#include "util/protofile.h"

#include <protocol/shared.pb.h>

#include <spdlog/spdlog.h>

#include <concepts>
#include <fstream>
#include <filesystem>
#include <google/protobuf/util/time_util.h>
#ifdef GetCurrentTime
#undef GetCurrentTime
#endif

namespace app::game {

	namespace {

		static bool gameLoaded = false;
		static tp::Game cachedGame; // cachedGame.last_played().seconds() == 0 means that no game is currently saved

		constexpr int NbrHighScoreResults = 10;
		const std::string SavedGameFile{"savedGame.mw"};
		const std::string SavedHighScoreFile{"highScore.mw"};

		template <tetris::BlockType type, tp::BlockType tpType, char chr>
		consteval void staticAssertBlockType() {
			static_assert(static_cast<char>(type) == static_cast<char>(tpType));
			static_assert(static_cast<int>(type) == chr);
		}

		consteval void staticTestBlockType() {
			staticAssertBlockType<tetris::BlockType::I, tp::BlockType::I, 'I'>();
			staticAssertBlockType<tetris::BlockType::J, tp::BlockType::J, 'J'>();
			staticAssertBlockType<tetris::BlockType::L, tp::BlockType::L, 'L'>();
			staticAssertBlockType<tetris::BlockType::O, tp::BlockType::O, 'O'>();
			staticAssertBlockType<tetris::BlockType::S, tp::BlockType::S, 'S'>();
			staticAssertBlockType<tetris::BlockType::T, tp::BlockType::T, 'T'>();
			staticAssertBlockType<tetris::BlockType::Z, tp::BlockType::Z, 'Z'>();
			staticAssertBlockType<tetris::BlockType::Empty, tp::BlockType::EMPTY, 'E'>();
			staticAssertBlockType<tetris::BlockType::Wall, tp::BlockType::WALL, 'W'>();
		}

		tetris::Block toBlock(const tp::Block& block) {
			return tetris::Block{static_cast<tetris::BlockType>(block.type()), block.start_column(), block.lowest_start_row(), block.rotations()};
		}

		std::vector<tetris::BlockType> toBoard(const tp::PlayerBoard& player) {
			std::vector<tetris::BlockType> board;
			for (auto type : player.board()) {
				board.push_back(static_cast<tetris::BlockType>(type));
			}
			return board;
		}

		tetris::TetrisBoard toTetrisBoard(const tp::PlayerBoard& player) {
			std::vector<tetris::BlockType> board;
			for (auto type : player.board()) {
				board.push_back(static_cast<tetris::BlockType>(type));
			}

			auto next = static_cast<tetris::BlockType>(player.next());
			tetris::Block current = toBlock(player.current());

			return tetris::TetrisBoard{board, player.width(), player.height(), current, next};
		}

		void setTpPlayer(tp::PlayerBoard& tpPlayerBoard, const Player& player) {
			auto playerData = std::get<DefaultPlayerData>(player.getPlayerData()); // Do first in case throwing bad_variant
			const auto& blockTypes = player.getBoardVector();
			tpPlayerBoard.clear_board();
			for (const auto type : blockTypes) {
				tpPlayerBoard.add_board(static_cast<tp::BlockType>(type));
			}
			tpPlayerBoard.set_ai(false);
			tpPlayerBoard.set_level(playerData.level);
			tpPlayerBoard.set_points(playerData.points);

			tpPlayerBoard.set_next(static_cast<tp::BlockType>(player.getNextBlockType()));
			tpPlayerBoard.set_cleared_rows(player.getClearedRows());
			tpPlayerBoard.set_width(player.getColumns());
			tpPlayerBoard.set_height(player.getRows());

			tpPlayerBoard.mutable_current()->set_lowest_start_row(player.getBlock().getLowestStartRow());
			tpPlayerBoard.mutable_current()->set_start_column(player.getBlock().getStartColumn());
			tpPlayerBoard.mutable_current()->set_rotations(player.getBlock().getCurrentRotation());
			tpPlayerBoard.mutable_current()->set_type(static_cast<tp::BlockType>(player.getBlock().getBlockType()));
		}
		
		PlayerPtr createPlayer(DevicePtr device, const tp::PlayerBoard& tpPlayer) {
			tetris::TetrisBoard tetrisBoard = toTetrisBoard(tpPlayer);
			DefaultPlayerData playerData{
				.level = tpPlayer.level(),
				.points = tpPlayer.points()
			};
			auto player = createHumanPlayer(device, playerData, std::move(tetrisBoard));
			player->setClearedRows(tpPlayer.cleared_rows());
			player->setGravity(calculateGravity(tpPlayer.level()));
			return player;
		}

	}

	bool hasSavedGame() {
		return cachedGame.last_played().seconds() != 0;
	}

	void clearSavedGame() {
		cachedGame.mutable_last_played()->set_seconds(0);
		std::filesystem::path filepath{SavedGameFile};
		if (std::filesystem::exists(filepath)) {
			std::filesystem::remove(filepath);
		}
	}

	void saveGame(const Player& player) {
		try {
			setTpPlayer(*cachedGame.mutable_player_board(), player);
			cachedGame.mutable_last_played()->CopyFrom(google::protobuf::util::TimeUtil::GetCurrentTime());
			saveToFile(cachedGame, SavedGameFile);
		} catch (const std::bad_variant_access& e) {
			spdlog::error("[Serilize] Fail to save, not DefaultPlayerData: {}", e.what());
		}
	}

	PlayerPtr loadGame(DevicePtr devicePtr) {
		if (!gameLoaded) {
			gameLoaded = true;
			
			if (!std::filesystem::exists(std::filesystem::path{SavedGameFile})) {
				spdlog::info("[Serialize] Game save file {} does not exist", SavedGameFile);
				return nullptr;
			}

			if (!loadFromFile(cachedGame, SavedGameFile)) {
				spdlog::warn("[Serialize] Game failed to be loaded: {}");
				return nullptr;
			}
		}
		if (cachedGame.has_player_board()) {
			return createPlayer(devicePtr, cachedGame.player_board());
		}
		return nullptr;
	}

}
