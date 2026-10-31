#ifndef APP_AUDIO_AUDIOENGINE_H
#define APP_AUDIO_AUDIOENGINE_H

#include "../configuration.h"

#include <memory>
#include <string_view>

namespace app::audio {

	enum class SoundEffect : int {
		UiClick, UiConfirm, UiError,
		Move, HardDrop, Lock,
		LineClear, LineClear4,
		LevelUp, Garbage, Countdown, Go, Pause, Resume, GameOver,
		Count
	};

	class AudioEngine {
	public:
		explicit AudioEngine(const Configuration::EditableSettings& settings);
		~AudioEngine();

		AudioEngine(const AudioEngine&) = delete;
		AudioEngine& operator=(const AudioEngine&) = delete;

		void applySettings(const Configuration::EditableSettings& previous,
			const Configuration::EditableSettings& current);
		void previewSettings(const Configuration::EditableSettings& settings);
		void play(SoundEffect effect);
		void playMusic();
		void stopMusic();
		bool isAvailable() const;
		std::string_view getStatus() const;

	private:
		struct Impl;
		std::unique_ptr<Impl> impl_;
	};

}

#endif
