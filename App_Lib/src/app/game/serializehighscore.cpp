#include "serializehighscore.h"
#include "configuration.h"
#include "tetrisparameters.h"
#include "util/protofile.h"

#include <protocol/high_score.pb.h>
#include <fmt/std.h>
#include <spdlog/spdlog.h>

#include <algorithm>
#include <concepts>
#include <filesystem>
#include <chrono>
#include <google/protobuf/util/time_util.h>
#ifdef GetCurrentTime
#undef GetCurrentTime
#endif

namespace app::game {

	namespace {

		constexpr int NbrHighScoreResults = 10;
		const std::filesystem::path& savedHighScoreFile() {
			static const auto file = [] {
				auto path = Configuration::getInstance().getSavePath("highScore.mw");
				spdlog::info("[Serialize] High-score file: {}", path);
				return path;
			}();
			return file;
		}

		bool resultRanksBefore(const tp::HighScore_Result& a, const tp::HighScore_Result& b) {
			if (a.points() != b.points()) {
				return a.points() > b.points();
			}
			if (a.last_played().seconds() != b.last_played().seconds()) {
				return a.last_played().seconds() < b.last_played().seconds();
			}
			return a.last_played().nanos() < b.last_played().nanos();
		}

		std::vector<HighScoreResult> toHighScoreResults(const google::protobuf::RepeatedPtrField<tp::HighScore_Result>& repeatedField) {
			std::vector<HighScoreResult> results;
			for (const auto& result : repeatedField) {
				auto timePoint = std::chrono::system_clock::from_time_t(result.last_played().seconds());
				auto ymd = std::chrono::year_month_day{std::chrono::floor<std::chrono::days>(timePoint)};
				const int startLevel{clampStartLevel(result.start_level())};
				results.push_back(HighScoreResult{
					.name = result.name(),
					.points = result.points(),
					.rows = result.rows(),
					.level = levelForClearedRows(result.rows(), startLevel),
					.startLevel = startLevel,
					.lastPlayed = ymd
				});
			}
			std::sort(results.begin(), results.end(), [](const HighScoreResult& a, const HighScoreResult& b) {
				return a.points > b.points || (a.points == b.points && a.lastPlayed < b.lastPlayed);
			});
			return results;
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

	std::vector<HighScoreResult> loadHighScore() {
		tp::HighScore highScore;
		const auto file = savedHighScoreFile();

		if (std::filesystem::exists(file)) {
			if (!loadFromFile(highScore, file.string())) {
				spdlog::info("[Serialize] Nigscore  loaded: {}");
			}
		} else {
			spdlog::info("[Serialize] High-score file does not exist");
		}

		auto results = toHighScoreResults(highScore.results());
		results.resize(NbrHighScoreResults);
		return results;
	}

	bool isNewHighScore(int points) {
		auto results = loadHighScore();
		return getIndexForNewResult(results, points) < NbrHighScoreResults;
	}

	void saveHighScore(const std::string& name, int points, int rows, int level, int startLevel) {
		tp::HighScore highScore;
		const auto& file = savedHighScoreFile();
		loadFromFile(highScore, file.string());

		auto result = highScore.add_results();
		result->set_name(name);
		result->set_points(points);
		result->set_rows(rows);
		result->set_level(level);
		result->set_start_level(clampStartLevel(startLevel));
		result->mutable_last_played()->CopyFrom(google::protobuf::util::TimeUtil::GetCurrentTime());

		std::vector<tp::HighScore_Result> results{highScore.results().begin(), highScore.results().end()};
		std::sort(results.begin(), results.end(), resultRanksBefore);

		highScore.clear_results();
		const auto resultCount = std::min<int>(NbrHighScoreResults, static_cast<int>(results.size()));
		for (int index = 0; index < resultCount; ++index) {
			highScore.add_results()->CopyFrom(results[index]);
		}
		if (!saveToFile(highScore, file.string())) {
			spdlog::error("[Serialize] Failed to save high scores: {}", file);
		}
	}

}
