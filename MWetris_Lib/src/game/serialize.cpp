#include "serialize.h"

#include "localplayerboardbuilder.h"
#include "tetrisgame.h"
#include "util/protofile.h"

#include <shared.pb.h>
#include <spdlog/spdlog.h>

#include <concepts>
#include <fstream>
#include <filesystem>
#include <google/protobuf/util/time_util.h>

namespace mwetris::game {

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

		void setTpPlayer(tp::PlayerBoard& tpPlayerBoard, const PlayerBoard& playerBoard) {
			auto playerData = std::get<DefaultPlayerData>(playerBoard.getPlayerData()); // Do first in case throwing bad_variant
			const auto& blockTypes = playerBoard.getBoardVector();
			tpPlayerBoard.clear_board();
			for (const auto type : blockTypes) {
				tpPlayerBoard.add_board(static_cast<tp::BlockType>(type));
			}
			tpPlayerBoard.set_ai(false);
			tpPlayerBoard.set_level(playerData.level);
			tpPlayerBoard.set_points(playerData.points);

			tpPlayerBoard.set_name(playerBoard.getName());
			tpPlayerBoard.set_next(static_cast<tp::BlockType>(playerBoard.getNextBlockType()));
			tpPlayerBoard.set_cleared_rows(playerBoard.getClearedRows());
			tpPlayerBoard.set_width(playerBoard.getColumns());
			tpPlayerBoard.set_height(playerBoard.getRows());

			tpPlayerBoard.mutable_current()->set_lowest_start_row(playerBoard.getBlock().getLowestStartRow());
			tpPlayerBoard.mutable_current()->set_start_column(playerBoard.getBlock().getStartColumn());
			tpPlayerBoard.mutable_current()->set_rotations(playerBoard.getBlock().getCurrentRotation());
			tpPlayerBoard.mutable_current()->set_type(static_cast<tp::BlockType>(playerBoard.getBlock().getBlockType()));
		}

		LocalPlayerBoardPtr createPlayer(const tp::PlayerBoard& player) {
			return LocalPlayerBoardBuilder{}
				.withBoard(toBoard(player))
				.withMovingBlock(toBlock(player.current()))
				.withHeight(player.height())
				.withWidth(player.width())
				.withPlayerData(DefaultPlayerData{
					.level = player.level(),
					.points = player.points()
				})
				.withName(player.name())
				.withNextBlockType(static_cast<tetris::BlockType>(player.next()))
				.withClearedRows(player.cleared_rows())
				.build();
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

	void saveGame(const PlayerBoard& playerBoard) {
		try {
			setTpPlayer(*cachedGame.mutable_player_board(), playerBoard);
			cachedGame.mutable_last_played()->CopyFrom(google::protobuf::util::TimeUtil::GetCurrentTime());
			saveToFile(cachedGame, SavedGameFile);
		} catch (const std::bad_variant_access& e) {
			spdlog::error("[Serilize] Fail to save, not DefaultPlayerData: {}", e.what());
		}
	}

	LocalPlayerBoardPtr loadGame() {
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
			return createPlayer(cachedGame.player_board());
		}
		return nullptr;
	}

}
