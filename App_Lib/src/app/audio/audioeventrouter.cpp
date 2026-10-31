#include "audioeventrouter.h"

namespace app::audio {

	std::vector<SoundEffect> AudioEventRouter::route(const game::Player& player, const game::PlayerBoardEvent& event) {
		std::vector<SoundEffect> result;
		if (!player.isHuman() || !player.isLocal()) return result;
		auto& state = playerStates_[&player];

		std::visit([&](const auto& value) {
			using Event = std::decay_t<decltype(value)>;
			if constexpr (std::is_same_v<Event, game::UpdateRestart>) {
				state = {};
			} else if constexpr (std::is_same_v<Event, game::UpdateMove>) {
				state.pendingMove = value.move;
			} else if constexpr (std::is_same_v<Event, game::TetrisBoardEvent>) {
				switch (value.event) {
					case tetris::BoardEvent::PlayerMovesBlock:
						if (state.pendingMove) {
							switch (*state.pendingMove) {
								case tetris::Move::Left:
								case tetris::Move::Right:
								case tetris::Move::RotateLeft:
								case tetris::Move::RotateRight: result.push_back(SoundEffect::Move); break;
								case tetris::Move::DownGround: result.push_back(SoundEffect::HardDrop); break;
								default: break;
							}
						}
						state.pendingMove.reset();
						break;
					case tetris::BoardEvent::GravityMovesBlock:
						state.pendingMove.reset();
						break;
					case tetris::BoardEvent::BlockCollision:
						state.pendingLock = true;
						break;
					case tetris::BoardEvent::RowsRemoved:
						state.pendingLock = false;
						result.push_back(value.value >= 4 ? SoundEffect::LineClear4 : SoundEffect::LineClear);
						break;
					case tetris::BoardEvent::CurrentBlockUpdated:
						if (state.pendingLock) result.push_back(SoundEffect::Lock);
						state.pendingLock = false;
						state.pendingMove.reset();
						break;
					default: break;
				}
			} else if constexpr (std::is_same_v<Event, game::UpdatePlayerData>) {
				if (const auto* data = std::get_if<game::DefaultPlayerData>(&value.playerData)) {
					if (state.level >= 0 && data->level > state.level) result.push_back(SoundEffect::LevelUp);
					state.level = data->level;
				}
			} else if constexpr (std::is_same_v<Event, game::ExternalRows>) {
				if (!value.blockTypes.empty()) result.push_back(SoundEffect::Garbage);
			}
		}, event);
		return result;
	}

	std::vector<SoundEffect> AudioEventRouter::route(const TetrisEvent& event) {
		std::vector<SoundEffect> result;
		std::visit([&](const auto& value) {
			using Event = std::decay_t<decltype(value)>;
			if constexpr (std::is_same_v<Event, game::GamePause>) {
				if (value.pause && value.countDown > 0) {
					result.push_back(SoundEffect::Countdown);
					countingDown_ = true;
					paused_ = true;
				} else if (value.pause) {
					result.push_back(SoundEffect::Pause);
					paused_ = true;
					countingDown_ = false;
				} else {
					result.push_back(!started_ || countingDown_ ? SoundEffect::Go : SoundEffect::Resume);
					started_ = true;
					paused_ = false;
					countingDown_ = false;
				}
			} else if constexpr (std::is_same_v<Event, game::GameOver>) {
				if (value.player && value.player->isHuman() && value.player->isLocal()) result.push_back(SoundEffect::GameOver);
			} else if constexpr (std::is_same_v<Event, CreateGameEvent>) {
				reset();
			}
		}, event);
		return result;
	}

	void AudioEventRouter::reset() {
		playerStates_.clear();
		started_ = false;
		paused_ = false;
		countingDown_ = false;
	}

}
