#ifndef APP_AUDIO_AUDIOEVENTROUTER_H
#define APP_AUDIO_AUDIOEVENTROUTER_H

#include "audioengine.h"
#include "../tetriscontroller.h"

#include <optional>
#include <unordered_map>
#include <vector>

namespace app::audio {

	class AudioEventRouter {
	public:
		std::vector<SoundEffect> route(const game::Player& player, const game::PlayerBoardEvent& event);
		std::vector<SoundEffect> route(const TetrisEvent& event);
		void reset();

	private:
		struct PlayerState {
			std::optional<tetris::Move> pendingMove;
			bool pendingLock{};
			int level{-1};
		};

		std::unordered_map<const game::Player*, PlayerState> playerStates_;
		bool started_{};
		bool paused_{};
		bool countingDown_{};
	};

}

#endif
