#include "serialize.h"

#include "localplayerbuilder.h"

#include <message.pb.h>

#include <fstream>

namespace tp = tetris_protocol;

namespace mwetris::game {

	namespace {

		const std::string SavedGameFile{"savedGame.mw"};

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

		std::vector<tetris::BlockType> toBoard(const tp::Player& player) {
			std::vector<tetris::BlockType> board;
			for (auto type : player.board()) {
				board.push_back(static_cast<tetris::BlockType>(type));
			}
			return board;
		}

		tetris::TetrisBoard toTetrisBoard(const tp::Player& player) {
			std::vector<tetris::BlockType> board;
			for (auto type : player.board()) {
				board.push_back(static_cast<tetris::BlockType>(type));
			}

			auto next = static_cast<tetris::BlockType>(player.next());
			tetris::Block current = toBlock(player.current());

			return tetris::TetrisBoard{board, player.width(), player.height(), current, next};
		}

		void setTpPlayer(tp::Player& player, const LocalPlayer& localPlayer) {
			const auto& blockTypes = localPlayer.getTetrisBoard().getBoardVector();
			for (const auto type : blockTypes) {
				player.add_board(static_cast<tp::BlockType>(type));
			}
			player.set_ai(false);
			player.set_level(localPlayer.getLevel());
			player.set_points(localPlayer.getPoints());
			player.set_name(localPlayer.getName());
			player.set_next(static_cast<tp::BlockType>(localPlayer.getTetrisBoard().getNextBlockType()));
			player.set_cleared_rows(localPlayer.getClearedRows());
			player.set_width(localPlayer.getTetrisBoard().getColumns());
			player.set_height(localPlayer.getTetrisBoard().getRows());

			player.mutable_current()->set_lowest_start_row(localPlayer.getTetrisBoard().getBlock().getLowestStartRow());
			player.mutable_current()->set_start_column(localPlayer.getTetrisBoard().getBlock().getStartColumn());
			player.mutable_current()->set_rotations(localPlayer.getTetrisBoard().getBlock().getCurrentRotation());
			player.mutable_current()->set_type(static_cast<tp::BlockType>(localPlayer.getTetrisBoard().getBlock().getBlockType()));
		}

		LocalPlayerPtr createPlayer(const tp::Player& player, DevicePtr device) {
			return LocalPlayerBuilder{}
				.withDevice(device)
				.withBoard(toBoard(player))
				.withMovingBlock(toBlock(player.current()))
				.withHeight(player.height())
				.withWidth(player.width())
				.withLevel(player.level())
				.withPoints(player.points())
				.withName(player.name())
				.withNextBlockType(static_cast<tetris::BlockType>(player.next()))
				.withClearedRows(player.cleared_rows())
				.build();
		}

		int64_t toTpSeconds(std::chrono::time_point<std::chrono::system_clock> timePoint) {
			auto seconds = std::chrono::duration_cast<std::chrono::seconds>(timePoint.time_since_epoch()).count();
			return static_cast<int64_t>(seconds);
		}

		std::chrono::time_point<std::chrono::system_clock> tpSecondsToDate(int64_t seconds) {
			std::chrono::time_point<std::chrono::system_clock> date{std::chrono::seconds{seconds}};
			return date;
		}

	}

	void saveGame(const std::vector<LocalPlayerPtr>& players) {
		tp::Game game;
		game.set_last_played_seconds(toTpSeconds(std::chrono::system_clock::now()));

		for (const auto& localPlayer: players) {
			setTpPlayer(*game.add_player(), *localPlayer);
		}
		std::ofstream output{SavedGameFile};
		game.SerializePartialToOstream(&output);
	}

	std::vector<LocalPlayerPtr> loadGame(const std::vector<DevicePtr>& availableDevices) {
		std::ifstream input{SavedGameFile};
		if (input.fail() || availableDevices.empty()) {
			return {};
		}

		tp::Game game;
		game.ParseFromIstream(&input);

		std::vector<LocalPlayerPtr> players_;
		for (const auto& pbPlayer : game.player()) {
			players_.push_back(createPlayer(pbPlayer, availableDevices.front()));
		}
		return players_;
	}

}
