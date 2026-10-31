#include "audioengine.h"

#include <SDL3/SDL.h>
#include <SDL3_mixer/SDL_mixer.h>
#include <sdl/util.h>
#include <spdlog/spdlog.h>

#include <array>
#include <algorithm>
#include <utility>

namespace app::audio {

	namespace {
		
		constexpr int EffectCount = std::to_underlying(SoundEffect::Count);
		constexpr int EffectTrackCount = 8;

		using MixMixer = sdl::SdlUniquePtr<MIX_Mixer, MIX_DestroyMixer>;
		using MixTrack = sdl::SdlUniquePtr<MIX_Track, MIX_DestroyTrack>;
		using MixAudio = sdl::SdlUniquePtr<MIX_Audio, MIX_DestroyAudio>;

		std::array<std::string, EffectCount> effectPaths(const Configuration::AudioAssetSettings& assets) {
			return {
				assets.ui.click, assets.ui.confirm, assets.ui.error,
				assets.game.move, assets.game.hardDrop, assets.game.lock,
				assets.game.lineClear, assets.game.lineClear4, assets.game.levelUp,
				assets.game.garbage, assets.game.countdown, assets.game.go,
				assets.game.pause, assets.game.resume, assets.game.gameOver,
			};
		}
	}

	struct AudioSubsystemLifecycle {
		bool sdlAudioInitialized = false;
		bool mixerInitialized = false;

		~AudioSubsystemLifecycle() {
			if (mixerInitialized) MIX_Quit();
			if (sdlAudioInitialized) SDL_QuitSubSystem(SDL_INIT_AUDIO);
		}
	};

	struct AudioEngine::Impl : AudioSubsystemLifecycle {
		MixMixer mixer;
		MixTrack musicTrack;
		MixTrack moveTrack;
		std::array<MixTrack, EffectTrackCount> effectTracks;
		std::array<MixAudio, EffectCount> effects;
		std::array<bool, EffectCount> effectLoaded{};
		Configuration::EditableSettings settings;
		std::string status{"Audio unavailable"};
		int nextTrack{};
		bool musicLoaded{};

		~Impl() {
			clearAudio();
		}

		void setError(std::string_view context, std::string_view path = {}) {
			status = path.empty()
				? std::string{context} + ": " + SDL_GetError()
				: std::string{context} + " '" + std::string{path} + "': " + SDL_GetError();
			spdlog::error("{}", status);
		}

		void clearAudio() {
			if (musicTrack) {
				MIX_SetTrackIOStream(musicTrack.get(), nullptr, false);
			}
			musicLoaded = false;
			if (moveTrack) {
				MIX_SetTrackAudio(moveTrack.get(), nullptr);
			}
			for (auto& track : effectTracks) {
				if (track) MIX_SetTrackAudio(track.get(), nullptr);
			}
			for (auto& effect : effects) {
				effect.reset();
			}
			effectLoaded.fill(false);
		}

		void loadMusic() {
			MIX_StopTrack(musicTrack.get(), 0);
			MIX_SetTrackIOStream(musicTrack.get(), nullptr, false);
			musicLoaded = false;
			auto* musicStream = SDL_IOFromFile(settings.audioAssets.music.c_str(), "rb");
			if (!musicStream || !MIX_SetTrackIOStream(musicTrack.get(), musicStream, true)) {
				if (musicStream) SDL_CloseIO(musicStream);
				setError("Could not load music", settings.audioAssets.music);
			} else {
				musicLoaded = true;
			}
		}

		void loadEffect(int index, const std::string& path) {
			effects[index].reset(MIX_LoadAudio(mixer.get(), path.c_str(), true));
			effectLoaded[index] = effects[index] != nullptr;
			if (!effects[index]) setError("Could not load sound effect", path);
		}

		void updateAudioStatus() {
			if (musicLoaded && std::ranges::all_of(effectLoaded, [](bool loaded) { return loaded; })) status = "Audio ready";
		}

		void loadAudio() {
			loadMusic();
			auto paths = effectPaths(settings.audioAssets);
			for (int index = 0; index < EffectCount; ++index) {
				loadEffect(index, paths[index]);
			}
			updateAudioStatus();
		}

		void loadAudio(const Configuration::AudioAssetSettings& previous) {
			if (previous.music != settings.audioAssets.music) loadMusic();
			auto paths = effectPaths(settings.audioAssets);
			auto previousPaths = effectPaths(previous);
			for (int index = 0; index < EffectCount; ++index) {
				if (paths[index] != previousPaths[index]) loadEffect(index, paths[index]);
			}
			updateAudioStatus();
		}

		void applyGain() {
			if (!mixer) return;
			MIX_SetMixerGain(mixer.get(), settings.audio.masterEnabled ? settings.audio.masterVolume : 0.f);
			MIX_SetTrackGain(musicTrack.get(), settings.audio.musicEnabled ? settings.audio.musicVolume : 0.f);
			const float effectGain = settings.audio.effectsEnabled ? settings.audio.effectsVolume : 0.f;
			MIX_SetTrackGain(moveTrack.get(), effectGain);
			for (auto& track : effectTracks) MIX_SetTrackGain(track.get(), effectGain);
		}
	};

	AudioEngine::AudioEngine(const Configuration::EditableSettings& settings)
		: impl_{std::make_unique<Impl>()} {
		
		impl_->settings = settings;
		if (!SDL_InitSubSystem(SDL_INIT_AUDIO)) {
			impl_->setError("Could not initialize SDL audio");
			return;
		}
		impl_->sdlAudioInitialized = true;
		if (!MIX_Init()) {
			impl_->setError("Could not initialize SDL_mixer");
			return;
		}
		impl_->mixerInitialized = true;
		impl_->mixer.reset(MIX_CreateMixerDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, nullptr));
		if (!impl_->mixer) {
			impl_->setError("Could not open audio output");
			return;
		}
		impl_->musicTrack.reset(MIX_CreateTrack(impl_->mixer.get()));
		impl_->moveTrack.reset(MIX_CreateTrack(impl_->mixer.get()));
		for (auto& track : impl_->effectTracks) track.reset(MIX_CreateTrack(impl_->mixer.get()));
		if (!impl_->musicTrack || !impl_->moveTrack || std::ranges::any_of(impl_->effectTracks, [](const auto& track) { return !track; })) {
			impl_->setError("Could not create audio tracks");
			impl_->effectTracks = {};
			impl_->moveTrack.reset();
			impl_->musicTrack.reset();
			impl_->mixer.reset();
			return;
		}
		impl_->loadAudio();
		impl_->applyGain();
	}

	AudioEngine::~AudioEngine() = default;

	void AudioEngine::applySettings(const Configuration::EditableSettings& previous, const Configuration::EditableSettings& current) {
		impl_->settings = current;
		if (!impl_->mixer) return;
		if (previous.audioAssets != current.audioAssets) impl_->loadAudio(previous.audioAssets);
		impl_->applyGain();
		if (current.audio.musicEnabled) playMusic(); else stopMusic();
	}

	void AudioEngine::previewSettings(const Configuration::EditableSettings& settings) {
		auto previous = impl_->settings;
		applySettings(previous, settings);
	}

	void AudioEngine::play(SoundEffect effect) {
		if (!impl_->mixer || !impl_->settings.audio.effectsEnabled || effect == SoundEffect::Count) return;
		
		auto* audio = impl_->effects[std::to_underlying(effect)].get();
		if (!audio) return;
		
		MIX_Track* track{};
		if (effect == SoundEffect::Move) {
			track = impl_->moveTrack.get();
			MIX_StopTrack(track, 0);
		} else {
			for (int offset = 0; offset < EffectTrackCount; ++offset) {
				auto index = (impl_->nextTrack + offset) % EffectTrackCount;
				if (!MIX_TrackPlaying(impl_->effectTracks[index].get())) {
					impl_->nextTrack = (index + 1) % EffectTrackCount;
					track = impl_->effectTracks[index].get();
					break;
				}
			}
			if (!track) {
				track = impl_->effectTracks[impl_->nextTrack].get();
				impl_->nextTrack = (impl_->nextTrack + 1) % EffectTrackCount;
				MIX_StopTrack(track, 0);
			}
		}
		if (!MIX_SetTrackAudio(track, audio) || !MIX_PlayTrack(track, 0)) impl_->setError("Could not play sound effect");
	}

	void AudioEngine::playMusic() {
		if (!impl_->mixer || !impl_->musicLoaded || !impl_->settings.audio.musicEnabled || MIX_TrackPlaying(impl_->musicTrack.get())) return;
		MIX_SetTrackLoops(impl_->musicTrack.get(), -1);
		if (!MIX_PlayTrack(impl_->musicTrack.get(), 0)) impl_->setError("Could not play music");
	}

	void AudioEngine::stopMusic() {
		if (impl_->musicTrack) MIX_StopTrack(impl_->musicTrack.get(), 0);
	}

	bool AudioEngine::isAvailable() const {
		return impl_->mixer != nullptr;
	}

	std::string_view AudioEngine::getStatus() const {
		return impl_->status;
	}

}
