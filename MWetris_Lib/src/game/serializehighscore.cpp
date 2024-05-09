#include "serializehighscore.h"
#include "util/protofile.h"

#include <high_score.pb.h>
#include <spdlog/spdlog.h>

#include <concepts>
#include <filesystem>
#include <chrono>
#include <google/protobuf/util/time_util.h>

namespace mwetris::game {

	namespace {

		constexpr int NbrHighScoreResults = 10;
		const std::string SavedGameFile{"savedGame.mw"};
		const std::string SavedHighScoreFile{"highScore.mw"};

		std::vector<HighScoreResult> toHighScoreResults(const google::protobuf::RepeatedPtrField<tp::HighScore_Result>& repeatedField) {
			std::vector<HighScoreResult> results;
			for (const auto& result : repeatedField) {
				auto timePoint = std::chrono::system_clock::from_time_t(result.last_played().seconds());
				auto ymd = std::chrono::year_month_day{std::chrono::floor<std::chrono::days>(timePoint)};
				results.push_back(HighScoreResult{
					.name = result.name(),
					.points = result.points(),
					.rows = result.rows(),
					.level = result.level(),
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

	bool isNewHighScore(int points) {
		auto results = loadHighScore();
		return getIndexForNewResult(results, points) < NbrHighScoreResults;
	}

	void saveHighScore(const std::string& name, int points, int rows, int level) {
		tp::HighScore highScore;
		loadFromFile(highScore, SavedHighScoreFile);

		auto result = highScore.add_results();
		result->set_name(name);
		result->set_points(points);
		result->set_rows(rows);
		result->set_level(level);
		result->mutable_last_played()->CopyFrom(google::protobuf::util::TimeUtil::GetCurrentTime());

		auto highScoreResults = toHighScoreResults(highScore.results());
		highScoreResults.resize(NbrHighScoreResults);
		saveToFile(highScore, SavedHighScoreFile);
	}

}
