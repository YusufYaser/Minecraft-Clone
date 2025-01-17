#pragma once
#include <miniaudio/miniaudio.h>
#include <stdint.h>
#include <map>
#include <vector>
#include <mutex>
#include <thread>
#include "../config.h"

enum class SOUND_TYPE : uint8_t {
	BUTTON = 0
};

struct Sound {
	SOUND_TYPE type;
	int variations;
	const char* fileNames[8];
};

class SoundEngine {
public:
	SoundEngine();
	~SoundEngine();

	const char* getSoundDeviceName();

	void loadSounds();
	void registerSound(Sound& sound);

	void playSound(SOUND_TYPE soundType);

	bool isLoaded() const { return m_loaded; };

private:
	struct ESound {
		Sound soundData;
		int loadedCount = 0;
		ma_sound* sounds[8][SAME_SOUND_COUNT];
	};

	std::map<SOUND_TYPE, ESound*> sounds;
	std::mutex queueMux;
	std::vector<SOUND_TYPE> queue;

	bool m_loaded = false;
	ma_engine* engine;

	std::atomic<bool> unloading;
	std::thread t;
	void soundTick();
};
