#include "serialize.h"

#include "localplayerboardbuilder.h"
#include "tetrisgame.h"

#include <message.pb.h>
#include <spdlog/spdlog.h>

#include <concepts>
#include <fstream>
#include <filesystem>

namespace tp = tetris_protocol;

namespace mwetris::game {

	namespace {

		static bool gameLoaded = false;
		static tp::Game cachedGame; // cachedGame.last_played_seconds() == 0 means that no game is currently saved

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
			const auto& blockTypes = playerBoard.getTetrisBoard().getBoardVector();
			tpPlayerBoard.clear_board();
			for (const auto type : blockTypes) {
				tpPlayerBoard.add_board(static_cast<tp::BlockType>(type));
			}
			tpPlayerBoard.set_ai(false);
			tpPlayerBoard.set_level(playerBoard.getLevel());
			tpPlayerBoard.set_points(playerBoard.getPoints());
			tpPlayerBoard.set_name(playerBoard.getName());
			tpPlayerBoard.set_next(static_cast<tp::BlockType>(playerBoard.getTetrisBoard().getNextBlockType()));
			tpPlayerBoard.set_cleared_rows(playerBoard.getClearedRows());
			tpPlayerBoard.set_width(playerBoard.getTetrisBoard().getColumns());
			tpPlayerBoard.set_height(playerBoard.getTetrisBoard().getRows());

			tpPlayerBoard.mutable_current()->set_lowest_start_row(playerBoard.getTetrisBoard().getBlock().getLowestStartRow());
			tpPlayerBoard.mutable_current()->set_start_column(playerBoard.getTetrisBoard().getBlock().getStartColumn());
			tpPlayerBoard.mutable_current()->set_rotations(playerBoard.getTetrisBoard().getBlock().getCurrentRotation());
			tpPlayerBoard.mutable_current()->set_type(static_cast<tp::BlockType>(playerBoard.getTetrisBoard().getBlock().getBlockType()));
		}

		LocalPlayerBoardPtr createPlayer(const tp::PlayerBoard& player) {
			return LocalPlayerBoardBuilder{}
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
		return cachedGame.last_played_seconds() != 0;
	}

	void clearSavedGame() {
		cachedGame.set_last_played_seconds(0);
		std::filesystem::path filepath{SavedGameFile};
		if (std::filesystem::exists(filepath)) {
			std::filesystem::remove(filepath);
		}
	}

	void saveGame(const PlayerBoard& playerBoard) {
		cachedGame.set_last_played_seconds(toTpSeconds(std::chrono::system_clock::now()));
		setTpPlayer(*cachedGame.mutable_player_board(), playerBoard);
		
		saveToFile(cachedGame, SavedGameFile);
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
		return createPlayer(cachedGame.player_board());
	}

	std::vector<HighScoreResult> loadHighScore() {
		tp::HighScore highScore;

		if (std::filesystem::exists(std::filesystem::path{SavedHighScoreFile})) {
			if (!loadFromFile(highScore, SavedHighScoreFile)) {
				spdlog::info("[Serialize] Nigscore  loaded: {}");
			}
		} else {
			spdlog::info("[Serialize] Highscore file {} does not exist", SavedHighScoreFile);
		}

		auto results = toHighScoreResults(highScore.results());
		results.resize(NbrHighScoreResults);
		return results;
	}

	bool isNewHighScore(const PlayerBoardPtr& playerBoard) {
		auto results = loadHighScore();
		return getIndexForNewResult(results, playerBoard->getPoints()) < NbrHighScoreResults;
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
