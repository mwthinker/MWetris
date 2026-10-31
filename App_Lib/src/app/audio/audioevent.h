#ifndef APP_AUDIO_AUDIOEVENT_H
#define APP_AUDIO_AUDIOEVENT_H

#include "audioengine.h"
#include "../tetriscontroller.h"

#include <optional>

namespace app::audio {

	std::optional<SoundEffect> soundEffectFor(const game::Player& player, const game::PlayerBoardEvent& event);
	std::optional<SoundEffect> soundEffectFor(const TetrisEvent& event);
	void playEventSound(AudioEngine& audioEngine, const game::Player& player, const game::PlayerBoardEvent& event);
	void playEventSound(AudioEngine& audioEngine, const TetrisEvent& event);

}

#endif
