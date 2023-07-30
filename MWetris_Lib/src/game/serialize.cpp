#include "serialize.h"

#include "localplayerbuilder.h"
#include "tetrisgame.h"

#include <message.pb.h>

#include <concepts>
#include <fstream>
#include <filesystem>

namespace tp = tetris_protocol;

namespace mwetris::game {

	namespace {

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

		LocalPlayerPtr createPlayer(const tp::Player& player) {
			return LocalPlayerBuilder{}
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

		int64_t toTpSeconds(std::chrono::year_month_day ymd) {
			std::chrono::system_clock::time_point tp = std::chrono::sys_days{ymd};
			auto seconds = std::chrono::duration_cast<std::chrono::seconds>(tp.time_since_epoch()).count();
			return static_cast<int64_t>(seconds);
		}

		std::chrono::time_point<std::chrono::system_clock> tpSecondsToDate(int64_t seconds) {
			std::chrono::time_point<std::chrono::system_clock> date{std::chrono::seconds{seconds}};
			return date;
		}

		template <typename Type>
		bool loadFromFile(Type& type, const std::string& file) {
			std::ifstream input{file};
			if (input.fail()) {
				return false;
			}
			return type.ParseFromIstream(&input);
		}

		template <typename Type>
		void saveToFile(const Type& type, const std::string& file) {
			std::ofstream output{file};
			type.SerializePartialToOstream(&output);
		}

		std::vector<HighScoreResult> toHighScoreResults(const google::protobuf::RepeatedPtrField<tp::HighScore_Result>& repeatedField) {
			std::vector<HighScoreResult> results;
			for (const auto& result : repeatedField) {
				auto time = tpSecondsToDate(result.last_played_seconds());
				auto ymd = std::chrono::year_month_day{std::chrono::floor<std::chrono::days>(time)};
				results.emplace_back(result.name(), result.points(), result.rows(), result.level(), ymd);
			}
			std::sort(results.begin(), results.end(), [](const HighScoreResult& a, const HighScoreResult& b) {
				return a.points > b.points || a.points == b.points && a.lastPlayed < b.lastPlayed;
			});
			return results;
		}

		void setTpHighScoreResults(tp::HighScore& highScore, const std::vector<HighScoreResult>& results) {
			highScore.clear_results();
			for (const auto& result : results) {
				auto tpResult = highScore.add_results();
				tpResult->set_last_played_seconds(toTpSeconds(result.lastPlayed));

			}
		}

		int getIndexForNewResult(const std::vector<HighScoreResult>& results, int points) {
			int index = 0;
			for (const auto& result : results) {
				if (points > result.points) {
					return index;
				}
				++index;
			}
			return index;
		}

	}

	int getHighScorePlacement(int points) {
		auto results = loadHighScore();
		return getIndexForNewResult(results, points) + 1;
	}

	bool hasSavedGame() {
		return std::filesystem::exists(SavedGameFile);
	}

	void clearSavedGame() {
		std::filesystem::path filepath{SavedGameFile};
		if (std::filesystem::exists(filepath)) {
			std::filesystem::remove(filepath);
		}
	}

	void saveGame(const std::vector<PlayerDevice>& players) {
		tp::Game game;
		game.set_last_played_seconds(toTpSeconds(std::chrono::system_clock::now()));

		for (const auto& playerDevice: players) {
			auto human = game.add_human();
			human->set_device_guid(playerDevice.device->getGuid());
			setTpPlayer(*human->mutable_player(), *playerDevice.player);
		}
		std::ofstream output{SavedGameFile};
		game.SerializePartialToOstream(&output);
	}

	std::vector<PlayerDevice> loadGame(const DeviceManager& deviceManager) {
		std::ifstream input{SavedGameFile};
		if (input.fail()) {
			return {};
		}

		tp::Game game;
		if (!loadFromFile(game, SavedGameFile)) {
			return {};
		}

		std::vector<PlayerDevice> players_;
		for (const auto& pbHuman : game.human()) {
			players_.push_back(PlayerDevice{
				.device = deviceManager.findDevice(pbHuman.device_guid()),
				.player = createPlayer(pbHuman.player())
			});
		}
		return players_;
	}

	std::vector<HighScoreResult> loadHighScore() {
		tp::HighScore highScore;
		loadFromFile(highScore, SavedHighScoreFile);

		auto results = toHighScoreResults(highScore.results());
		results.resize(NbrHighScoreResults);
		return results;
	}

	bool isNewHighScore(const PlayerPtr& player) {
		auto results = loadHighScore();
		return getIndexForNewResult(results, player->getPoints()) < NbrHighScoreResults;
	}

	void saveHighScore(const std::string& name, int points, int rows, int level) {
		tp::HighScore highScore;
		loadFromFile(highScore, SavedHighScoreFile);

		auto result = highScore.add_results();
		result->set_name(name);
		result->set_points(points);
		result->set_rows(rows);
		result->set_level(level);
		result->set_last_played_seconds(toTpSeconds(std::chrono::system_clock::now()));

		auto highScoreResults = toHighScoreResults(highScore.results());
		highScoreResults.resize(NbrHighScoreResults);
		saveToFile(highScore, SavedHighScoreFile);
	}

}
