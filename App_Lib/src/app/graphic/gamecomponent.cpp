#include "gamecomponent.h"

#include "game/classicscorecomparison.h"
#include "game/tetrisgameevent.h"

#include <optional>
#include <utility>
#include <algorithm>

namespace app::graphic {
	namespace {
		constexpr float PlayerCardSpacing = 12.f;
		constexpr double ScoreGapDisplayDuration = 5.0;

		struct ClassicScoreRanking {
			bool available = false;
			int leader = 0;
			int runnerUp = 0;
		};

		ClassicScoreRanking calculateClassicScoreRanking(const std::vector<ImGuiBoard>& boards) {
			const int boardCount{static_cast<int>(boards.size())};
			if (boardCount < 2) {
				return {};
			}

			int leader{0};
			int leaderPoints{0};
			for (int index = 0; index < boardCount; ++index) {
				const auto* playerData = std::get_if<game::DefaultPlayerData>(&boards[index].getPlayer().getPlayerData());
				if (playerData == nullptr) {
					return {};
				}
				if (index == 0 || playerData->points > leaderPoints) {
					leader = index;
					leaderPoints = playerData->points;
				}
			}

			int runnerUp{leader == 0 ? 1 : 0};
			int runnerUpPoints{std::get<game::DefaultPlayerData>(boards[runnerUp].getPlayer().getPlayerData()).points};
			for (int index = 0; index < boardCount; ++index) {
				if (index == leader) {
					continue;
				}
				const int points{std::get<game::DefaultPlayerData>(boards[index].getPlayer().getPlayerData()).points};
				if (points > runnerUpPoints) {
					runnerUp = index;
					runnerUpPoints = points;
				}
			}

			return ClassicScoreRanking{
				.available = true,
				.leader = leader,
				.runnerUp = runnerUp
			};
		}

		game::ClassicScoreEntry classicScoreEntry(const ImGuiBoard& board) {
			const auto& player = board.getPlayer();
			const auto& playerData = std::get<game::DefaultPlayerData>(player.getPlayerData());
			return {
				.points = playerData.points,
				.level = playerData.level,
				.clearedRows = player.getClearedRows(),
				.startLevel = playerData.startLevel
			};
		}
	}

	GameComponent::GameComponent(BoardSidebarRenderer boardSidebarRenderer)
		: boardSidebarRenderer_{std::move(boardSidebarRenderer)} {
	}

	GameComponent::~GameComponent() {}

	void GameComponent::draw(int windowWidth, int windowHeight, double deltaTime) {
		scoreGapDisplayTime_ += deltaTime;
		while (scoreGapDisplayTime_ >= ScoreGapDisplayDuration) {
			scoreGapDisplayTime_ -= ScoreGapDisplayDuration;
			scoreGapDisplay_ = scoreGapDisplay_ == ScoreGapDisplay::Points
				? ScoreGapDisplay::Tetrises
				: ScoreGapDisplay::Points;
		}
		const auto scoreRanking = calculateClassicScoreRanking(imguiBoards_);

		const auto cursorPos = ImGui::GetCursorPos();

		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, Vec2{0.f, 0.f});
		ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, Vec2{0.f, 0.f});

		const int boardCount{static_cast<int>(imguiBoards_.size())};
		float width = windowWidth;
		if (boardCount > 1) {
			width = windowWidth / boardCount;
		}
		const float cardWidth = std::max(1.f, width - 2.f * PlayerCardSpacing);
		const float cardHeight = std::max(1.f, windowHeight - 2.f * PlayerCardSpacing);

		const Vec2 pos = ImGui::GetCursorPos();
		ImGui::SetCursorPos(pos + Vec2{PlayerCardSpacing, PlayerCardSpacing});
		for (int i = 0; i < boardCount; ++i) {
			std::optional<game::ClassicScoreComparison> scoreComparison;
			if (scoreRanking.available) {
				const int opponent{i == scoreRanking.leader ? scoreRanking.runnerUp : scoreRanking.leader};
				scoreComparison = game::calculateClassicScoreComparison(
					classicScoreEntry(imguiBoards_[i]), 
					classicScoreEntry(imguiBoards_[opponent])
				);
			}
			imguiBoards_[i].draw(cardWidth, cardHeight, deltaTime, gamePause_, scoreComparison, scoreGapDisplay_);
			if (i + 1 < boardCount) {
				ImGui::SameLine(0.f, 2.f * PlayerCardSpacing);
			}
		}

		ImGui::PopStyleVar(2);

		ImGui::SetCursorPos(cursorPos);
		ImGui::Dummy({static_cast<float>(windowWidth), static_cast<float>(windowHeight)});
	}

	void GameComponent::initGame(const std::vector<game::PlayerPtr>& players) {
		imguiBoards_.clear();
		scoreGapDisplayTime_ = 0.0;
		scoreGapDisplay_ = ScoreGapDisplay::Points;
		int playerNumber = 1;
		for (auto& player : players) {
			imguiBoards_.emplace_back(player, playerNumber++, boardSidebarRenderer_);
		}
	}

	void GameComponent::gamePause(const game::GamePause& gamePause) {
		gamePause_ = gamePause;
	}

}
