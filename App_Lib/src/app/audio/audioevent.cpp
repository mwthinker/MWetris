#include "audioevent.h"

#include <type_traits>

namespace app::audio {

	std::optional<SoundEffect> soundEffectFor(const game::Player& player, const game::PlayerBoardEvent& event) {
		if (!player.isHuman() || !player.isLocal()) return {};

		std::optional<SoundEffect> result;
		std::visit([&](const auto& value) {
			using Event = std::decay_t<decltype(value)>;
			if constexpr (std::is_same_v<Event, game::TetrisBoardEvent>) {
				switch (value.event) {
					case tetris::BoardEvent::PlayerMovedBlock:
					case tetris::BoardEvent::PlayerRotatedBlock:
						result = SoundEffect::Move;
						break;
					case tetris::BoardEvent::PlayerHardDroppedBlock:
						result = SoundEffect::HardDrop;
						break;
					case tetris::BoardEvent::BlockLocked:
						result = value.value == 0
							? SoundEffect::Lock
							: value.value >= 4 ? SoundEffect::LineClear4 : SoundEffect::LineClear;
						break;
					default: break;
				}
			} else if constexpr (std::is_same_v<Event, game::LevelUp>) {
				result = SoundEffect::LevelUp;
			} else if constexpr (std::is_same_v<Event, game::ExternalRows>) {
				if (!value.blockTypes.empty()) result = SoundEffect::Garbage;
			}
		}, event);
		return result;
	}

	std::optional<SoundEffect> soundEffectFor(const TetrisEvent& event) {
		std::optional<SoundEffect> result;
		std::visit([&](const auto& value) {
			using Event = std::decay_t<decltype(value)>;
			if constexpr (std::is_same_v<Event, game::GamePause>) {
				switch (value.action) {
					case game::GamePauseAction::Countdown: result = SoundEffect::Countdown; break;
					case game::GamePauseAction::CountdownFinished: result = SoundEffect::Go; break;
					case game::GamePauseAction::Started: result = SoundEffect::Go; break;
					case game::GamePauseAction::Paused: result = SoundEffect::Pause; break;
					case game::GamePauseAction::Resumed: result = SoundEffect::Resume; break;
				}
			} else if constexpr (std::is_same_v<Event, game::GameOver>) {
				if (value.player && value.player->isHuman() && value.player->isLocal()) result = SoundEffect::GameOver;
			}
		}, event);
		return result;
	}

	void playEventSound(AudioEngine& audioEngine, const game::Player& player, const game::PlayerBoardEvent& event) {
		if (auto effect = soundEffectFor(player, event)) audioEngine.play(*effect);
	}

	void playEventSound(AudioEngine& audioEngine, const TetrisEvent& event) {
		if (auto effect = soundEffectFor(event)) audioEngine.play(*effect);
	}

}
